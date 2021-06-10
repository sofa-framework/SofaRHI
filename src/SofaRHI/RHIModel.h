#pragma once

#include <SofaRHI/config.h>

#include <SofaRHI/RHIGraphicModel.h>
#include <SofaRHI/RHIComputeModel.h>
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

    RHIGroup(const utils::BufferInfo& bufferInfo, int materialID);

    //bool initRHI(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc) override;
    //void updateRHIResources(QRhiResourceUpdateBatch* batch) override;
    void addDrawCommand(QRhiCommandBuffer* cb, const QRhiCommandBuffer::VertexInput* vbindings);

    int getMaterialID() const { return m_materialID; }
private:
    int m_materialID;
    const utils::BufferInfo m_bufferInfo;
};

class RHIRendering
{
public:
    using QRhiPtr = std::shared_ptr<QRhi>;
    using QRhiCommandBufferPtr = std::shared_ptr<QRhiCommandBuffer>;
    using QRhiResourceUpdateBatchPtr = std::shared_ptr<QRhiResourceUpdateBatch>;
    using QRhiRenderPassDescriptorPtr = std::shared_ptr<QRhiRenderPassDescriptor>;
    using LoaderMaterial = sofa::type::Material;

    RHIRendering(const RHIGroup& group)
        : m_rhigroup(group)
    {}

    virtual bool initRHIResources(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc, std::vector<QRhiShaderResourceBinding> globalBindings, const LoaderMaterial& loaderMaterial) = 0;
    virtual void updateRHIResources(QRhiResourceUpdateBatch* batch, const LoaderMaterial& loaderMaterial) = 0;
    void updateRHICommands(QRhiCommandBuffer* cb, const QRhiViewport& viewport, const QRhiCommandBuffer::VertexInput* vbindings)
    {
        //Create commands
        cb->setGraphicsPipeline(m_pipeline);
        cb->setShaderResources();
        cb->setViewport(viewport);

        m_rhigroup.addDrawCommand(cb, vbindings);
    }

    int getMaterialID() const { return m_rhigroup.getMaterialID(); }
protected:
    RHIGroup m_rhigroup;
    QRhiGraphicsPipeline* m_pipeline = nullptr;
    QRhiShaderResourceBindings* m_srb = nullptr;
    //Uniform buffers
    QRhiBuffer* m_materialBuffer = nullptr;
};

class RHIPhongRendering : public RHIRendering
{
public:
    RHIPhongRendering(const RHIGroup& group)
        : RHIRendering(group)
    {}

    bool initRHIResources(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc, std::vector<QRhiShaderResourceBinding> globalBindings, const LoaderMaterial& loaderMaterial) override;
    void updateRHIResources(QRhiResourceUpdateBatch* batch, const LoaderMaterial& loaderMaterial) override;
};

class RHIDiffuseTexturedPhongRendering : public RHIRendering
{
public:
    RHIDiffuseTexturedPhongRendering(const RHIGroup& group)
        : RHIRendering(group)
    {}

    bool initRHIResources(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc, std::vector<QRhiShaderResourceBinding> globalBindings, const LoaderMaterial& loaderMaterial) override;
    void updateRHIResources(QRhiResourceUpdateBatch* batch, const LoaderMaterial& loaderMaterial) override;

private:
    //TODO: parameter or anything
    bool m_bMipMap = false;
    bool m_bAutoGenMipMap = false;

    QImage m_diffuseImage;
    QRhiTexture* m_diffuseTexture = nullptr;
    QRhiSampler* m_diffuseSampler = nullptr;
};

class RHIWireframeRendering : public RHIRendering
{
public:
    RHIWireframeRendering(const RHIGroup& group)
        : RHIRendering(group)
    {}

    bool initRHIResources(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc, std::vector<QRhiShaderResourceBinding> globalBindings, const LoaderMaterial& loaderMaterial) override;
    void updateRHIResources(QRhiResourceUpdateBatch* batch, const LoaderMaterial& loaderMaterial) override;

private:
};

class SOFA_SOFARHI_API RHIModel : public sofa::component::visualmodel::VisualModelImpl, public RHIGraphicModel, public RHIComputeModel
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
    bool initGraphicResources(QRhiPtr rhi, QRhiRenderPassDescriptorPtr rpDesc) override;
    void updateGraphicResources(QRhiResourceUpdateBatch* batch) override;
    void updateGraphicCommands(QRhiCommandBuffer* cb, const QRhiViewport& viewport) override;

    // RHIComputeModel API
    bool initComputeResources(QRhiPtr rhi) override;
    void updateComputeResources(QRhiResourceUpdateBatch* batch) override;
    void updateComputeCommands(QRhiCommandBuffer* cb) override;

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

    std::vector<std::shared_ptr<RHIRendering> > m_renderGroups;
    std::vector<std::shared_ptr<RHIWireframeRendering> > m_wireframeGroups;

    //Compute
    QRhiBuffer* m_storageBuffer = nullptr;
    QRhiBuffer* m_computeNormalBuffer = nullptr;
    QRhiBuffer* m_computeUniformBuffer = nullptr;
    QRhiShaderResourceBindings* m_computeBindings = nullptr;
    QRhiComputePipeline* m_computePipeline = nullptr;

    void updateStorageBuffer(QRhiResourceUpdateBatch* batch);
};

} // namespace sofa::rhi
