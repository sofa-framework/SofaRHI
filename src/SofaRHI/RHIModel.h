#pragma once

#include <SofaRHI/config.h>

#include <SofaRHI/RHIVisualModel.h>
#include <SofaRHI/RHIUtils.h>
#include <SofaBaseVisual/VisualModelImpl.h>
#include <sofa/core/DataTracker.h>

#include <QtGui/private/qrhi_p.h>
#include <QFile>

class QImage;

namespace sofa::rhi
{

class RHIGroup
{
public:
    using FaceGroup = sofa::component::visualmodel::VisualModelImpl::FaceGroup;

    RHIGroup(const FaceGroup& g, const utils::BufferInfo& bufferInfo);

    //bool initRHI(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc) override;
    //void updateRHIResources(QRhiResourceUpdateBatch* batch) override;
    void addDrawCommand(QRhiCommandBuffer* cb, const QRhiCommandBuffer::VertexInput* vbindings);

//private:
    const FaceGroup m_group;
    const utils::BufferInfo m_bufferInfo;
};

class RHIRendering
{
public:
    using QRhiPtr = std::shared_ptr<QRhi>;
    using QRhiCommandBufferPtr = std::shared_ptr<QRhiCommandBuffer>;
    using QRhiResourceUpdateBatchPtr = std::shared_ptr<QRhiResourceUpdateBatch>;
    using QRhiRenderPassDescriptorPtr = std::shared_ptr<QRhiRenderPassDescriptor>;
    using LoaderMaterial = sofa::core::loader::Material;
protected:
    //Uniform buffers
    QRhiBuffer* m_materialBuffer = nullptr;
};

class RHIPhongGroup : public RHIRendering
{
public:
    RHIPhongGroup(const RHIGroup& group);

    bool initRHIResources(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc, std::vector<QRhiShaderResourceBinding> globalBindings, const LoaderMaterial& loaderMaterial);
    void updateRHIResources(QRhiResourceUpdateBatch* batch, const LoaderMaterial& loaderMaterial);
    void updateRHICommands(QRhiCommandBuffer* cb, const QRhiViewport& viewport, const QRhiCommandBuffer::VertexInput* vbindings);

    int getMaterialID() const { return m_rhigroup.m_group.materialId; }
private:
    RHIGroup m_rhigroup;
    QRhiGraphicsPipeline* m_pipeline = nullptr;
    QRhiShaderResourceBindings* m_srb = nullptr;

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
    void initVisual() override;
    void updateVisual() override;
    void handleTopologyChange() override; 
    
    // RHIVisualModel API
    bool initRHIResources(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc);
    void updateRHIResources(QRhiResourceUpdateBatch* batch);
    void updateRHICommands(QRhiCommandBuffer* cb, const QRhiViewport& viewport);

private:
    void internalDraw(const sofa::core::visual::VisualParams* vparams, bool transparent) override;

    void updateBuffers() override;

    void updateVertexBuffer(QRhiResourceUpdateBatch* batch, bool updateGroupInfo = false);
    void updateIndexBuffer(QRhiResourceUpdateBatch* batch);
    void updateCameraUniformBuffer(QRhiResourceUpdateBatch* batch);
    //void updateMaterialUniformBuffer(QRhiResourceUpdateBatch* batch);
    
    //Uniform buffers
    QRhiBuffer* m_cameraUniformBuffer = nullptr;
    //Dynamic buffers
    QRhiBuffer* m_vertexPositionBuffer = nullptr;
    QRhiBuffer* m_indexTriangleBuffer = nullptr;
    QRhiBuffer* m_indexEdgeBuffer = nullptr;

    QMatrix4x4 m_correctionMatrix;

    int m_triangleNumber = 0;
    int m_quadTriangleNumber = 0;
    quint32 m_positionsBufferSize = 0, m_normalsBufferSize = 0, m_textureCoordsBufferSize = 0;

    bool m_needUpdatePositions = true;
    bool m_needUpdateTopology = true;
    bool m_needUpdateMaterial = true;

    std::vector<std::shared_ptr<RHIPhongGroup> > m_phongGroups;
};

} // namespace sofa::rhi
