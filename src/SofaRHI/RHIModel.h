#pragma once

#include <SofaRHI/config.h>

#include <SofaBaseVisual/VisualModelImpl.h>
#include <sofa/core/DataTracker.h>

namespace sofa::rhi
{

class SOFA_SOFARHI_API RHIModel : public sofa::component::visualmodel::VisualModelImpl
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

private:

    bool ready() { return m_isReady; }
    void setReady(bool ready) { m_isReady = ready; }
    bool m_isReady;
    bool m_bTopologyHasChanged;

    /// name is not really relevant
    void internalDraw(const sofa::core::visual::VisualParams* vparams, bool transparent) override;

    void updateBuffers() override;
    void createVertexBuffer();
    void initVertexBuffer();
    void createEdgesIndicesBuffer();
    void createTrianglesIndicesBuffer();
    void createQuadsIndicesBuffer();
    void updateVertexBuffer();
    void updateEdgesIndicesBuffer();
    void updateTrianglesIndicesBuffer();
    void updateQuadsIndicesBuffer();
    
    void* m_vertexPositionBuffer{ nullptr };
    void* m_indexTriangleBuffer{ nullptr };
    void* m_indexQuadBuffer{ nullptr };
    void* m_indexEdgeBuffer{ nullptr };
    void* m_indexWireframeBuffer{ nullptr };
    std::size_t m_oldPositionSize, m_oldNormalSize, m_oldEdgeSize, m_oldTriangleSize, m_oldQuadSize;

};

} // namespace sofa::rhi
