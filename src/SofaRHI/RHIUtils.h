#pragma once

#include <SofaRHI/config.h>

#include <QtGui/private/qrhi_p.h>
#include <array>

namespace sofa::rhi
{
    using QRhiPtr = std::shared_ptr<QRhi>;
    using QRhiCommandBufferPtr = std::shared_ptr<QRhiCommandBuffer>;
    using QRhiResourceUpdateBatchPtr = std::shared_ptr<QRhiResourceUpdateBatch>;
    using QRhiRenderPassDescriptorPtr = std::shared_ptr<QRhiRenderPassDescriptor>;
}

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

