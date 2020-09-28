#pragma once

#include <SofaRHI/config.h>

#include <SofaRHI/RHIUtils.h>

namespace sofa::rhi
{

class SOFA_SOFARHI_API RHIVisualModel
{
public:
    virtual bool initRHIResources(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc) = 0;
    virtual void updateRHIResources(QRhiResourceUpdateBatch* batch) = 0;
    virtual void updateRHICommands(QRhiCommandBuffer* cb, const QRhiViewport& viewport) = 0;
};


} // namespace sofa::rhi
