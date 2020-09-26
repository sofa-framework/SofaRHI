#pragma once

#include <SofaRHI/config.h>

#include <sofa/simulation/Visitor.h>
#include <SofaRHI/RHIComputeModel.h>

#include <sofa/core/visual/VisualParams.h>
#include <SofaRHI/DrawToolRHI.h>

namespace sofa::rhi
{

class SOFA_SOFARHI_API RHIComputeVisitor : public sofa::simulation::Visitor
{
public:
    using QRhiPtr = std::shared_ptr<QRhi>;
    using QRhiCommandBufferPtr = std::shared_ptr<QRhiCommandBuffer>;
    using QRhiResourceUpdateBatchPtr = std::shared_ptr<QRhiResourceUpdateBatch>;
    using QRhiRenderPassDescriptorPtr = std::shared_ptr<QRhiRenderPassDescriptor>;

    RHIComputeVisitor(core::visual::VisualParams* params)
        : sofa::simulation::Visitor(params)
        , m_rhiDrawTool(nullptr)
    {
        m_vparams = params;
        m_rhiDrawTool = dynamic_cast<sofa::rhi::DrawToolRHI*>(m_vparams->drawTool());
    }

    virtual Result processNodeTopDown(simulation::Node* node) = 0;
    virtual void processObject(simulation::Node* /*node*/, core::objectmodel::BaseObject* o) = 0;

    const char* getCategoryName() const override { return "visualcompute"; }
    const char* getClassName() const override { return "RHIComputeVisitor"; }

    /// qt3d visual visitor must be executed as a tree, such as forward and backward orders are coherent
    bool treeTraversal(TreeTraversalRepetition& repeat) override { repeat=NO_REPETITION; return true; }
protected:
    sofa::rhi::DrawToolRHI* m_rhiDrawTool;
    core::visual::VisualParams* m_vparams;
};

class SOFA_SOFARHI_API RHIComputeInitResourcesVisitor : public RHIComputeVisitor
{
public:
    RHIComputeInitResourcesVisitor(core::visual::VisualParams* params)
        : RHIComputeVisitor(params) {}

    Result processNodeTopDown(simulation::Node* node) override;
    void processObject(simulation::Node* /*node*/, core::objectmodel::BaseObject* o) override;

    const char* getClassName() const override { return "RHIComputeInitResourcesVisitor"; }

};

class SOFA_SOFARHI_API RHIComputeUpdateResourcesVisitor : public RHIComputeVisitor
{
public:
    RHIComputeUpdateResourcesVisitor(core::visual::VisualParams* params)
        : RHIComputeVisitor(params) {}

    Result processNodeTopDown(simulation::Node* node) override;
    void processObject(simulation::Node* /*node*/, core::objectmodel::BaseObject* o) override;

    const char* getClassName() const override { return "RHIComputeUpdateResourcesVisitor"; }
};

class SOFA_SOFARHI_API RHIComputeUpdateCommandsVisitor : public RHIComputeVisitor
{
public:
    RHIComputeUpdateCommandsVisitor(core::visual::VisualParams* params)
        : RHIComputeVisitor(params) {}

    Result processNodeTopDown(simulation::Node* node) override;
    void processObject(simulation::Node* /*node*/, core::objectmodel::BaseObject* o) override;

    const char* getClassName() const override { return "RHIComputeUpdateCommandsVisitor"; }
};

} // namespace sofa::rhi


