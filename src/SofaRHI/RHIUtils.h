#pragma once

#include <SofaRHI/config.h>

#include <QtGui/private/qrhi_p.h>
#include <array>

namespace sofa::rhi::utils
{
    //as defined in the shader files
    struct Material
    {
        std::array<float, 3> ambient;
        std::array<float, 3> diffuse;
        std::array<float, 3> specular;
        float shininess;
    };

    struct GroupInfo
    {
        uint8_t materialID;
    };

    //Definitions
    constexpr std::size_t MATRIX4_SIZE{ 64 };
    constexpr std::size_t VEC3_SIZE{ 12 };
    constexpr std::size_t MATERIAL_SIZE = sizeof(Material);
    constexpr std::size_t MAXIMUM_MATERIAL_NUMBER{ 9 }; //
    constexpr std::size_t GROUPINFO_SIZE = sizeof(GroupInfo);

    //Helper functions
    QShader loadShader(const std::string& name);

} // namespace sofa::rhi::utils

