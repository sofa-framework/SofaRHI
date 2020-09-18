#pragma once

#include <SofaRHI/config.h>

#include <QtGui/private/qrhi_p.h>
#include <array>

namespace sofa::rhi::utils
{
    using Vector3 = std::array<float, 3>;
    using Vec3i = std::array<int, 3>;
    using Vec2i = std::array<int, 2>;
    using Vec2f = std::array<float, 2>;

    enum class PrimitiveType : int
    {
        POINT = 0,
        LINE = 1,
        TRIANGLE = 2,
    };

    struct BufferInfo
    {
        QRhiBuffer* buffer;
        int offset;
        int size;
    };

    //as defined in the shader files
    struct PhongMaterial
    {
        std::array<float, 4> ambient;
        std::array<float, 4> diffuse;
        std::array<float, 4> specular;
        std::array<float, 4> shininess;// 3 padding of float;
    };

    struct GroupInfo
    {
        uint8_t materialID;
    };

    //Definitions
    constexpr std::size_t MATRIX4_SIZE{ 64 };
    constexpr std::size_t VEC3_SIZE{ 12 };
    constexpr std::size_t PHONG_MATERIAL_SIZE = sizeof(PhongMaterial);
    constexpr std::size_t MAXIMUM_MATERIAL_NUMBER{ 9 }; //
    constexpr std::size_t GROUPINFO_SIZE = sizeof(GroupInfo);

    //Helper functions
    QShader loadShader(const std::string& name);

    /// http://www.songho.ca/opengl/gl_sphere.html
    template<typename TVector3, typename TVec2f, typename TVec3i>
    void buildVerticesSmooth(float radius, int sectors, int stacks,
        std::vector<TVector3>& vertices, std::vector<TVector3>& normals, std::vector<TVec2f>& texcoords, std::vector<TVec3i>& indices)
    {
        if (radius < 0.0f)
            return;
        if (sectors < 3)
            return;
        if (sectors < 2)
            return;

        const float PI = acos(-1);

        vertices.clear();
        normals.clear();
        texcoords.clear();
        indices.clear();

        float x, y, z, xy;                              // vertex position
        float nx, ny, nz, lengthInv = 1.0f / radius;    // normal
        float s, t;                                     // texCoord

        float sectorStep = 2 * PI / sectors;
        float stackStep = PI / stacks;
        float sectorAngle, stackAngle;

        for (int i = 0; i <= stacks; ++i)
        {
            stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
            xy = radius * cosf(stackAngle);             // r * cos(u)
            z = radius * sinf(stackAngle);              // r * sin(u)

            // add (sectors+1) vertices per stack
            // the first and last vertices have same position and normal, but different tex coords
            for (int j = 0; j <= sectors; ++j)
            {
                sectorAngle = j * sectorStep;           // starting from 0 to 2pi

                // vertex position
                x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
                y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
                vertices.push_back({ x, y, z });

                // normalized vertex normal
                nx = x * lengthInv;
                ny = y * lengthInv;
                nz = z * lengthInv;
                normals.push_back({ nx, ny, nz });

                // vertex tex coord between [0, 1]
                s = (float)j / sectors;
                t = (float)i / stacks;
                texcoords.push_back({ s, t });
            }
        }

        // indices
        //  k1--k1+1
        //  |  / |
        //  | /  |
        //  k2--k2+1
        int k1, k2;
        for (int i = 0; i < stacks; ++i)
        {
            k1 = i * (sectors + 1);     // beginning of current stack
            k2 = k1 + sectors + 1;      // beginning of next stack

            for (int j = 0; j < sectors; ++j, ++k1, ++k2)
            {
                // 2 triangles per sector excluding 1st and last stacks
                if (i != 0)
                {
                    indices.push_back({ k1, k2, k1 + 1 }); // k1---k2---k1+1
                }

                if (i != (stacks - 1))
                {
                    indices.push_back({ k1 + 1, k2, k2 + 1 }); // k1+1---k2---k2+1
                }
            }
        }
    }

    

    template<typename TVector3, typename TVec2f, typename TVec3i>
    void buildVerticesFlat(float radius, int sectors, int stacks,
        std::vector<Vector3>& vertices, std::vector<Vector3>& normals, std::vector<Vec2f>& texcoords, std::vector<Vec3i>& indices)
    {
        if (radius < 0.0f)
            return;
        if (sectors < 3)
            return;
        if (sectors < 2)
            return;

        vertices.clear();
        normals.clear();
        texcoords.clear();
        indices.clear();

        const float PI = acos(-1);

        // tmp vertex definition (x,y,z,s,t)
        struct TempVertex
        {
            float x, y, z, s, t;
        };
        std::vector<TempVertex> tmpVertices;

        float sectorStep = 2 * PI / sectors;
        float stackStep = PI / stacks;
        float sectorAngle, stackAngle;

        // compute all vertices first, each vertex contains (x,y,z,s,t) except normal
        for (int i = 0; i <= stacks; ++i)
        {
            stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
            float xy = radius * cosf(stackAngle);       // r * cos(u)
            float z = radius * sinf(stackAngle);        // r * sin(u)

            // add (sectors+1) vertices per stack
            // the first and last vertices have same position and normal, but different tex coords
            for (int j = 0; j <= sectors; ++j)
            {
                sectorAngle = j * sectorStep;           // starting from 0 to 2pi

                TempVertex vertex;
                vertex.x = xy * cosf(sectorAngle);      // x = r * cos(u) * cos(v)
                vertex.y = xy * sinf(sectorAngle);      // y = r * cos(u) * sin(v)
                vertex.z = z;                           // z = r * sin(u)
                vertex.s = (float)j / sectors;        // s
                vertex.t = (float)i / stacks;         // t
                tmpVertices.push_back(vertex);
            }
        }

        TempVertex v1, v2, v3, v4;                          // 4 vertex positions and tex coords
        Vector3 n;                           // 1 face normal

        int i, j, k, vi1, vi2;
        int index = 0;                                  // index for vertex
        for (i = 0; i < stacks; ++i)
        {
            vi1 = i * (sectors + 1);                // index of tmpVertices
            vi2 = (i + 1) * (sectors + 1);

            for (j = 0; j < sectors; ++j, ++vi1, ++vi2)
            {
                // get 4 vertices per sector
                //  v1--v3
                //  |    |
                //  v2--v4
                v1 = tmpVertices[vi1];
                v2 = tmpVertices[vi2];
                v3 = tmpVertices[vi1 + 1];
                v4 = tmpVertices[vi2 + 1];

                // if 1st stack and last stack, store only 1 triangle per sector
                // otherwise, store 2 triangles (quad) per sector
                if (i == 0) // a triangle for first stack ==========================
                {
                    // put a triangle
                    vertices.push_back({ v1.x, v1.y, v1.z });
                    vertices.push_back({ v2.x, v2.y, v2.z });
                    vertices.push_back({ v4.x, v4.y, v4.z });

                    // put tex coords of triangle
                    texcoords.push_back({ v1.s, v1.t });
                    texcoords.push_back({ v2.s, v2.t });
                    texcoords.push_back({ v4.s, v4.t });

                    // put normal
                    n = computeNormal<Vector3>({ v1.x, v1.y, v1.z }, { v2.x, v2.y, v2.z }, { v4.x, v4.y, v4.z });
                    for (k = 0; k < 3; ++k)  // same normals for 3 vertices
                    {
                        normals.push_back({ n[0], n[1], n[2] });
                    }

                    // put indices of 1 triangle
                    indices.push_back({ index, index + 1, index + 2 });

                    index += 3;     // for next
                }
                else if (i == (stacks - 1)) // a triangle for last stack =========
                {
                    // put a triangle
                    vertices.push_back({ v1.x, v1.y, v1.z });
                    vertices.push_back({ v2.x, v2.y, v2.z });
                    vertices.push_back({ v3.x, v3.y, v3.z });

                    // put tex coords of triangle
                    texcoords.push_back({ v1.s, v1.t });
                    texcoords.push_back({ v2.s, v2.t });
                    texcoords.push_back({ v3.s, v3.t });

                    // put normal
                    n = computeNormal<Vector3>({ v1.x, v1.y, v1.z }, { v2.x, v2.y, v2.z }, { v3.x, v3.y, v3.z });
                    for (k = 0; k < 3; ++k)  // same normals for 3 vertices
                    {
                        normals.push_back({ n[0], n[1], n[2] });
                    }

                    // put indices of 1 triangle
                    indices.push_back({ index, index + 1, index + 2 });


                    index += 3;     // for next
                }
                else // 2 triangles for others ====================================
                {
                    // put quad vertices: v1-v2-v3-v4
                    vertices.push_back({ v1.x, v1.y, v1.z });
                    vertices.push_back({ v2.x, v2.y, v2.z });
                    vertices.push_back({ v3.x, v3.y, v3.z });
                    vertices.push_back({ v4.x, v4.y, v4.z });

                    // put tex coords of quad
                    texcoords.push_back({ v1.s, v1.t });
                    texcoords.push_back({ v2.s, v2.t });
                    texcoords.push_back({ v3.s, v3.t });
                    texcoords.push_back({ v4.s, v4.t });

                    // put normal
                    n = computeNormal<Vector3>({ v1.x, v1.y, v1.z }, { v2.x, v2.y, v2.z }, { v3.x, v3.y, v3.z });
                    for (k = 0; k < 4; ++k)  // same normals for 4 vertices
                    {
                        normals.push_back({ n[0], n[1], n[2] });
                    }

                    // put indices of quad (2 triangles)
                    indices.push_back({ index, index + 1, index + 2 });
                    indices.push_back({ index + 2, index + 1, index + 3 });

                    index += 4;     // for next
                }
            }
        }
    }
    template<typename TVector3>
    TVector3 computeNormal(const TVector3& v0, const TVector3& v1, const TVector3& v2)
    {
        using Real = double; // or TVector3::value_type?
        const double EPSILON = 0.000001f;

        TVector3 normal{ 0.0f, 0.0f, 0.0f };     // default return value (0,0,0)
        Real nx, ny, nz;

        // find 2 edge vectors: v1-v2, v1-v3
        Real ex1 = v1[0] - v0[0];
        Real ey1 = v1[1] - v0[1];
        Real ez1 = v1[2] - v0[2];
        Real ex2 = v2[0] - v0[0];
        Real ey2 = v2[1] - v0[1];
        Real ez2 = v2[2] - v0[2];

        // cross product: e1 x e2
        nx = ey1 * ez2 - ez1 * ey2;
        ny = ez1 * ex2 - ex1 * ez2;
        nz = ex1 * ey2 - ey1 * ex2;

        // normalize only if the length is > 0
        Real length = sqrtf(nx * nx + ny * ny + nz * nz);
        if (length > EPSILON)
        {
            // normalize
            Real lengthInv = 1.0f / length;
            normal[0] = nx * lengthInv;
            normal[1] = ny * lengthInv;
            normal[2] = nz * lengthInv;
        }

        return normal;
    }


} // namespace sofa::rhi::utils

