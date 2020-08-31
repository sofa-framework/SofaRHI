#pragma once

#include <SofaRHI/config.h>

#include <SofaRHI/RHIVisualModel.h>
#include <SofaBaseVisual/VisualModelImpl.h>
#include <sofa/core/DataTracker.h>

#include <QtGui/private/qrhi_p.h>
#include <QFile>

namespace sofa::rhi
{

class SOFA_SOFARHI_API RHIModel : public sofa::component::visualmodel::VisualModelImpl, public RHIVisualModel
{
public:
    SOFA_CLASS(RHIModel,  sofa::component::visualmodel::VisualModelImpl);

    using InheritedVisual = sofa::component::visualmodel::VisualModelImpl;

    RHIModel();
    virtual ~RHIModel() override {}

    // VisualModel API
    void init() override;
    void initVisual() override;
    void updateVisual() override;
    void handleTopologyChange() override; 
    
    // RHIVisualModel API
    bool initRHI(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc);
    void updateRHIResources(QRhiResourceUpdateBatch* batch);
    void updateRHICommands(QRhiCommandBuffer* cb, const QRhiViewport& viewport);

private:
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

    int m_triangleNumber;
    quint32 m_positionsBufferSize, m_normalsBufferSize, m_textureCoordsBufferSize;

    bool m_needUpdatePositions = true;
    bool m_needUpdateTopology = true;
};

} // namespace sofa::rhi
