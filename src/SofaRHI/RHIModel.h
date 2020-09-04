#pragma once

#include <SofaRHI/config.h>

#include <SofaRHI/RHIVisualModel.h>
#include <SofaRHI/RHIUtils.h>
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

    void updateVertexBuffer(QRhiResourceUpdateBatch* batch, bool updateGroupInfo = false);
    void updateIndexBuffer(QRhiResourceUpdateBatch* batch);
    void updateCameraUniformBuffer(QRhiResourceUpdateBatch* batch);
    void updateMaterialUniformBuffer(QRhiResourceUpdateBatch* batch);
    
    QRhiGraphicsPipeline* m_pipeline = nullptr;
    QRhiShaderResourceBindings* m_srb = nullptr;
    //Uniform buffers
    QRhiBuffer* m_cameraUniformBuffer = nullptr;
    QRhiBuffer* m_materialsUniformBuffer = nullptr;
    //Dynamic buffers
    QRhiBuffer* m_vertexPositionBuffer = nullptr;
    QRhiBuffer* m_indexTriangleBuffer = nullptr;
    QRhiBuffer* m_indexEdgeBuffer = nullptr;

    QMatrix4x4 m_correctionMatrix;

    int m_triangleNumber = 0;
    int m_quadTriangleNumber = 0;
    quint32 m_positionsBufferSize = 0, m_normalsBufferSize = 0, m_textureCoordsBufferSize = 0, m_materialIDBufferSize = 0;

    bool m_needUpdatePositions = true;
    bool m_needUpdateTopology = true;
    bool m_needUpdateMaterial = true;
};

} // namespace sofa::rhi
