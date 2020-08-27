#pragma once

#include <SofaRHI/config.h>

#include <sofa/core/visual/VisualParams.h>

namespace sofa::rhi
{

/// It would be nice that VisualParams be entirely OpenGL-free
class SOFA_SOFARHI_API RHIVisualParams : public sofa::core::visual::VisualParams
{
public:
    RHIVisualParams()
    {
    }



};

} // namespace sofa::rhi


