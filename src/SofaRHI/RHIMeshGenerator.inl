#pragma once

#include <SofaRHI/RHIMeshGenerator.h>

namespace sofa::rhi
{

template<typename TVertexType, typename TNormalType, typename TTexcoordType, typename TTriangleType>
void MeshGenerator<TVertexType, TNormalType, TTexcoordType, TTriangleType>::SmoothSphere(std::vector<TVertexType>& vertices, std::vector<TNormalType>& normals,
    std::vector<TTexcoordType>& texcoords, std::vector<TTriangleType>& indices,
        float radius, int sectors, int stacks)
{
    if (radius < 0.0f)
        return;
    if (sectors < 4)
        return;
    if (stacks < 3)
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

template<typename TVertexType, typename TNormalType, typename TTexcoordType, typename TTriangleType>
void MeshGenerator<TVertexType, TNormalType, TTexcoordType, TTriangleType>::RoughSphere(std::vector<TVertexType>& vertices, std::vector<TNormalType>& normals,
    std::vector<TTexcoordType>& texcoords, std::vector<TTriangleType>& indices,
    float radius, int sectors, int stacks)
{
    if (radius < 0.0f)
        return;
    if (sectors < 4)
        return;
    if (stacks < 3)
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
    TNormalType n;                           // 1 face normal

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
                n = utils::computeNormal<TNormalType>({ v1.x, v1.y, v1.z }, { v2.x, v2.y, v2.z }, { v4.x, v4.y, v4.z });
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
                n = utils::computeNormal<TNormalType>({ v1.x, v1.y, v1.z }, { v2.x, v2.y, v2.z }, { v3.x, v3.y, v3.z });
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
                n = utils::computeNormal<TNormalType>({ v1.x, v1.y, v1.z }, { v2.x, v2.y, v2.z }, { v3.x, v3.y, v3.z });
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

template<typename TVertexType, typename TNormalType, typename TTexcoordType, typename TTriangleType>
void MeshGenerator<TVertexType, TNormalType, TTexcoordType, TTriangleType>::RoughCylinder(std::vector<TVertexType>& vertices, std::vector<TNormalType>& normals, std::vector<TTexcoordType>& texcoords, std::vector<TTriangleType>& indices,
    float baseRadius, float topRadius, float height, int sectors, int stacks)
{
    if (baseRadius < 0.0f)
        return;
    if (topRadius < 0.0f)
        return;
    if (sectors < 4)
        return;
    if (stacks < 2)
        return;

    vertices.clear();
    normals.clear();
    texcoords.clear();
    indices.clear();

    float x, y, z;                                  // vertex position
    //float s, t;                                     // texCoord
    float radius;                                   // radius for each stack

    // get normals for cylinder sides
    std::vector<float> sideNormals;
    const float PI = acos(-1);
    float sectorStep = 2 * PI / sectors;
    float sectorAngle;  // radian
    //build unit circle
    std::vector<float> unitCircleVertices;
    std::vector<float>().swap(unitCircleVertices);
    for (int i = 0; i <= sectors; ++i)
    {
        sectorAngle = i * sectorStep;
        unitCircleVertices.push_back(cos(sectorAngle)); // x
        unitCircleVertices.push_back(sin(sectorAngle)); // y
        unitCircleVertices.push_back(0);                // z
    }

    // compute the normal vector at 0 degree first
    // tanA = (baseRadius-topRadius) / height
    float zAngle = atan2(baseRadius - topRadius, height);
    float x0 = cos(zAngle);     // nx
    float y0 = 0;               // ny
    float z0 = sin(zAngle);     // nz

    // rotate (x0,y0,z0) per sector angle
    for (int i = 0; i <= sectors; ++i)
    {
        sectorAngle = i * sectorStep;
        sideNormals.push_back(cos(sectorAngle) * x0 - sin(sectorAngle) * y0);   // nx
        sideNormals.push_back(sin(sectorAngle) * x0 + cos(sectorAngle) * y0);   // ny
        sideNormals.push_back(z0);  // nz
    }

    // put vertices of side cylinder to array by scaling unit circle
    for (int i = 0; i <= stacks; ++i)
    {
        z = -(height * 0.5f) + (float)i / stacks * height;      // vertex position z
        radius = baseRadius + (float)i / stacks * (topRadius - baseRadius);     // lerp
        float t = 1.0f - (float)i / stacks;   // top-to-bottom

        for (int j = 0, k = 0; j <= sectors; ++j, k += 3)
        {
            x = unitCircleVertices[k];
            y = unitCircleVertices[k + 1];

            vertices.push_back({ x * radius, y * radius, z });
            normals.push_back({ sideNormals[k], sideNormals[k + 1], sideNormals[k + 2] });
            texcoords.push_back({ (float)j / sectors, t });
        }
    }

    // remember where the base.top vertices start
    int baseVertexIndex = (int)vertices.size() / 3;

    // put vertices of base of cylinder
    z = -height * 0.5f;
    vertices.push_back({ 0, 0, z });
    normals.push_back({ 0, 0, -1 });
    texcoords.push_back({ 0.5f, 0.5f });

    for (int i = 0, j = 0; i < sectors; ++i, j += 3)
    {
        x = unitCircleVertices[j];
        y = unitCircleVertices[j + 1];

        vertices.push_back({ x * baseRadius, y * baseRadius, z });
        normals.push_back({ 0, 0, -1 });
        texcoords.push_back({ -x * 0.5f + 0.5f, -y * 0.5f + 0.5f });// flip horizontal
    }

    // remember where the base vertices start
    int topVertexIndex = (int)vertices.size() / 3;

    // put vertices of top of cylinder
    z = height * 0.5f;
    vertices.push_back({ 0, 0, z });
    normals.push_back({ 0, 0, 1 });
    texcoords.push_back({ 0.5f, 0.5f });
    for (int i = 0, j = 0; i < sectors; ++i, j += 3)
    {
        x = unitCircleVertices[j];
        y = unitCircleVertices[j + 1];
        vertices.push_back({ x * topRadius, y * topRadius, z });
        normals.push_back({ 0, 0, 1 });
        texcoords.push_back({ x * 0.5f + 0.5f, -y * 0.5f + 0.5f });
    }

    // put indices for sides
    int k1, k2;
    for (int i = 0; i < stacks; ++i)
    {
        k1 = i * (sectors + 1);     // beginning of current stack
        k2 = k1 + sectors + 1;      // beginning of next stack

        for (int j = 0; j < sectors; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector
            indices.push_back({ k1, k1 + 1, k2 });
            indices.push_back({ k2, k1 + 1, k2 + 1 });

        }
    }

    // put indices for base
    for (int i = 0, k = baseVertexIndex + 1; i < sectors; ++i, ++k)
    {
        if (i < (sectors - 1))
            indices.push_back({ baseVertexIndex, k + 1, k });
        else    // last triangle
            indices.push_back({ baseVertexIndex, baseVertexIndex + 1, k });
    }

    
    for (int i = 0, k = topVertexIndex + 1; i < sectors; ++i, ++k)
    {
        if (i < (sectors - 1))
            indices.push_back({ topVertexIndex, k, k + 1 });
        else
            indices.push_back({ topVertexIndex, k, topVertexIndex + 1 });
    }
}

template<typename TVertexType, typename TNormalType, typename TTexcoordType, typename TTriangleType>
void MeshGenerator<TVertexType, TNormalType, TTexcoordType, TTriangleType>::SmoothCylinder(std::vector<TVertexType>& vertices, std::vector<TNormalType>& normals, std::vector<TTexcoordType>& texcoords, std::vector<TTriangleType>& indices,
    float baseRadius, float topRadius, float height, int sectors, int stacks)
{
    if (baseRadius < 0.0f)
        return;
    if (topRadius < 0.0f)
        return;
    if (sectors < 4)
        return;
    if (stacks < 2)
        return;

    // tmp vertex definition (x,y,z,s,t)
    struct TempVertex
    {
        float x, y, z, s, t;
    };
    std::vector<TempVertex> tmpVertices;

    //build unit circle
    std::vector<float> unitCircleVertices;
    const float PI = acos(-1);
    float sectorStep = 2 * PI / sectors;
    float sectorAngle;  // radian

    std::vector<float>().swap(unitCircleVertices);
    for (int i = 0; i <= sectors; ++i)
    {
        sectorAngle = i * sectorStep;
        unitCircleVertices.push_back(cos(sectorAngle)); // x
        unitCircleVertices.push_back(sin(sectorAngle)); // y
        unitCircleVertices.push_back(0);                // z
    }

    int i, j, k;    // indices
    float x, y, z, s, t, radius;

    // put tmp vertices of cylinder side to array by scaling unit circle
    //NOTE: start and end vertex positions are same, but texcoords are different
    //      so, add additional vertex at the end point
    for (i = 0; i <= stacks; ++i)
    {
        z = -(height * 0.5f) + (float)i / stacks * height;      // vertex position z
        radius = baseRadius + (float)i / stacks * (topRadius - baseRadius);     // lerp
        t = 1.0f - (float)i / stacks;   // top-to-bottom

        for (j = 0, k = 0; j <= sectors; ++j, k += 3)
        {
            x = unitCircleVertices[k];
            y = unitCircleVertices[k + 1];
            s = (float)j / sectors;

            TempVertex vertex;
            vertex.x = x * radius;
            vertex.y = y * radius;
            vertex.z = z;
            vertex.s = s;
            vertex.t = t;
            tmpVertices.push_back(vertex);
        }
    }

    vertices.clear();
    normals.clear();
    texcoords.clear();
    indices.clear();

    TempVertex v1, v2, v3, v4;      // 4 vertex positions v1, v2, v3, v4
    TNormalType n;       // 1 face normal
    int vi1, vi2;               // indices
    int index = 0;

    // v2-v4 <== stack at i+1
    // | \ |
    // v1-v3 <== stack at i
    for (i = 0; i < stacks; ++i)
    {
        vi1 = i * (sectors + 1);            // index of tmpVertices
        vi2 = (i + 1) * (sectors + 1);

        for (j = 0; j < sectors; ++j, ++vi1, ++vi2)
        {
            v1 = tmpVertices[vi1];
            v2 = tmpVertices[vi2];
            v3 = tmpVertices[vi1 + 1];
            v4 = tmpVertices[vi2 + 1];

            // compute a face normal of v1-v3-v2
            n = utils::computeNormal<TNormalType>({ v1.x, v1.y, v1.z }, { v3.x, v3.y, v3.z }, { v2.x, v2.y, v2.z });
            
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
            for (k = 0; k < 4; ++k)  // same normals for all 4 vertices
            {
                normals.push_back({ n[0], n[1], n[2] });
            }

            // put indices of a quad
            indices.push_back({ index, index + 2, index + 1 }); // v1-v3-v2
            indices.push_back({ index + 1, index + 2, index + 3 }); // v2-v3-v4

            index += 4;     // for next
        }
    }

    int baseVertexIndex = (int)vertices.size() / 3;

    // put vertices of base of cylinder
    z = -height * 0.5f;
    vertices.push_back({ 0, 0, z });
    normals.push_back({ 0, 0, -1 });
    texcoords.push_back({ 0.5f, 0.5f });
    for (i = 0, j = 0; i < sectors; ++i, j += 3)
    {
        x = unitCircleVertices[j];
        y = unitCircleVertices[j + 1];    
        
        vertices.push_back({ x * baseRadius, y * baseRadius, z });
        normals.push_back({ 0, 0, -1 });
        texcoords.push_back({ -x * 0.5f + 0.5f, -y * 0.5f + 0.5f });// flip horizontal
    }

    // put indices for base
    for (i = 0, k = baseVertexIndex + 1; i < sectors; ++i, ++k)
    {
        if (i < sectors - 1)
            indices.push_back({ baseVertexIndex, k + 1, k});
        else
            indices.push_back({ baseVertexIndex, baseVertexIndex + 1, k });
    }

    int topVertexIndex = (int)vertices.size() / 3;

    // put vertices of top of cylinder
    z = height * 0.5f;
    vertices.push_back({ 0, 0, z });
    normals.push_back({ 0, 0, 1 });
    texcoords.push_back({ 0.5f, 0.5f });

    for (i = 0, j = 0; i < sectors; ++i, j += 3)
    {
        x = unitCircleVertices[j];
        y = unitCircleVertices[j + 1];

        vertices.push_back({ x * topRadius, y * topRadius, z });
        normals.push_back({ 0, 0, 1 });
        texcoords.push_back({ x * 0.5f + 0.5f, -y * 0.5f + 0.5f });
    }

    for (i = 0, k = topVertexIndex + 1; i < sectors; ++i, ++k)
    {
        if (i < sectors - 1)
            indices.push_back({ topVertexIndex, k, k + 1 });
        else
            indices.push_back({ topVertexIndex, k, topVertexIndex + 1 });
    }
}

} // namespace sofa::rhi

