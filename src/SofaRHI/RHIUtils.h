#pragma once

#include <SofaRHI/config.h>

#include <QtGui/private/qrhi_p.h>

namespace sofa::rhi::utils
{
    //Definitions
    constexpr std::size_t MATRIX4_SIZE{ 64 };
    constexpr std::size_t VEC3_SIZE{ 12 };

    //Helper functions
    QShader loadShader(const std::string& name);

} // namespace sofa::rhi::utils

