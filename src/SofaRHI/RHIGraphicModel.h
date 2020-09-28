#pragma once

#include <SofaRHI/config.h>

#include <SofaRHI/RHIUtils.h>

namespace sofa::rhi
{

class SOFA_SOFARHI_API RHIGraphicModel
{
public:
    virtual bool initGraphicResources(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc) = 0;
    virtual void updateGraphicResources(QRhiResourceUpdateBatch* batch) = 0;
    virtual void updateGraphicCommands(QRhiCommandBuffer* cb, const QRhiViewport& viewport) = 0;
};


} // namespace sofa::rhi
