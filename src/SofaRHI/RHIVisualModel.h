#pragma once

#include <SofaRHI/config.h>

#include <QtGui/private/qrhi_p.h>

namespace sofa::rhi
{

class SOFA_SOFARHI_API RHIVisualModel
{
public:
    using QRhiPtr = std::shared_ptr<QRhi>;
    using QRhiCommandBufferPtr = std::shared_ptr<QRhiCommandBuffer>;
    using QRhiResourceUpdateBatchPtr = std::shared_ptr<QRhiResourceUpdateBatch>;
    using QRhiRenderPassDescriptorPtr = std::shared_ptr<QRhiRenderPassDescriptor>;

    virtual bool initRHIResources(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc) = 0;
    virtual void updateRHIResources(QRhiResourceUpdateBatch* batch) = 0;
    virtual void updateRHICommands(QRhiCommandBuffer* cb, const QRhiViewport& viewport) = 0;
};


} // namespace sofa::rhi
