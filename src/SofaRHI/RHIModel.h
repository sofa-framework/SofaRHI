#pragma once

#include <SofaRHI/config.h>

#include <SofaBaseVisual/VisualModelImpl.h>
#include <sofa/core/DataTracker.h>

#include <QtGui/private/qrhi_p.h>
#include <QFile>

namespace sofa::rhi
{

class RHIVisualModel
{
public:
    using QRhiPtr = std::shared_ptr<QRhi>;
    using QRhiCommandBufferPtr = std::shared_ptr<QRhiCommandBuffer>;
    using QRhiResourceUpdateBatchPtr = std::shared_ptr<QRhiResourceUpdateBatch>;
    using QRhiRenderPassDescriptorPtr = std::shared_ptr<QRhiRenderPassDescriptor>;

    virtual void initRHI(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc) = 0;
    virtual void addResourceUpdate(QRhiResourceUpdateBatch* batch) = 0;
    virtual void updateRHI(QRhiCommandBuffer* cb, const QRhiViewport& viewport) = 0;

    static QShader loadShader(const char* name)
    {
        QFile f(QString::fromUtf8(name));
        if (f.open(QIODevice::ReadOnly)) {
            const QByteArray contents = f.readAll();
            return QShader::fromSerialized(contents);
        }
        return QShader();
    }
};


class SOFA_SOFARHI_API RHIModel : public sofa::component::visualmodel::VisualModelImpl, public RHIVisualModel
{
public:
    SOFA_CLASS(RHIModel,  sofa::component::visualmodel::VisualModelImpl);

    using InheritedVisual = sofa::component::visualmodel::VisualModelImpl;

    RHIModel();
    virtual ~RHIModel() override {}

    // VisualModel API
    void init() override;
    void initVisual() override { InheritedVisual::initVisual(); }
    void updateVisual() override;
    void handleTopologyChange() override; 
    
    // RHIVisualModel API
    void initRHI(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc);
    void addResourceUpdate(QRhiResourceUpdateBatch* batch);
    void updateRHI(QRhiCommandBuffer* cb, const QRhiViewport& viewport);

private:

    bool ready() { return m_isReady; }
    void setReady(bool ready) { m_isReady = ready; }
    bool m_isReady;
    bool m_bTopologyHasChanged;

    /// name is not really relevant
    void internalDraw(const sofa::core::visual::VisualParams* vparams, bool transparent) override;

    void updateBuffers() override;

    void updateVertexBuffer(QRhiResourceUpdateBatch* batch);
    void updateIndexBuffer(QRhiResourceUpdateBatch* batch);
    void updateUniformBuffer(QRhiResourceUpdateBatch* batch);

    QRhiGraphicsPipeline* m_pipeline;
    QRhiShaderResourceBindings* m_srb;
    QRhiBuffer* m_uniformBuffer;
    QRhiBuffer* m_vertexPositionBuffer;
    QRhiBuffer* m_indexTriangleBuffer;
    QMatrix4x4 m_correctionMatrix;

    std::size_t m_triangleNumber;

};

} // namespace sofa::rhi
