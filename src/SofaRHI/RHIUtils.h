#pragma once

#include <SofaRHI/config.h>

#include <QtGui/private/qrhi_p.h>
#include <array>

namespace sofa::rhi::utils
{
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

} // namespace sofa::rhi::utils

