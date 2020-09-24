#pragma once

#include <SofaRHI/config.h>

#include <sofa/core/visual/VisualParams.h>
#include <sofa/simulation/Visitor.h>
#include <SofaRHI/DrawToolRHI.h>
#include <SofaRHI/RHIVisualModel.h>

namespace sofa::component::visualmodel
{
    class VisualStyle;
}

namespace sofa::rhi
{

class SOFA_SOFARHI_API RHIVisualDrawVisitor : public sofa::simulation::Visitor
{
public:
    using QRhiPtr = std::shared_ptr<QRhi>;
    using QRhiCommandBufferPtr = std::shared_ptr<QRhiCommandBuffer>;
    using QRhiResourceUpdateBatchPtr = std::shared_ptr<QRhiResourceUpdateBatch>;
    using QRhiRenderPassDescriptorPtr = std::shared_ptr<QRhiRenderPassDescriptor>;

    RHIVisualDrawVisitor(core::visual::VisualParams* params)
        : sofa::simulation::Visitor(params)
        , m_rhiDrawTool(nullptr)
    {
        m_vparams = params;
        m_rhiDrawTool = dynamic_cast<sofa::rhi::DrawToolRHI*>(params->drawTool());
    }

    virtual Result processNodeTopDown(simulation::Node* node) = 0;
    virtual void processObject(simulation::Node* /*node*/, core::objectmodel::BaseObject* o) = 0;

    const char* getCategoryName() const override { return "visual"; }
    const char* getClassName() const override { return "RHIVisualVisitor"; }

    /// qt3d visual visitor must be executed as a tree, such as forward and backward orders are coherent
    bool treeTraversal(TreeTraversalRepetition& repeat) override { repeat=NO_REPETITION; return true; }
protected:
    sofa::rhi::DrawToolRHI* m_rhiDrawTool;
    core::visual::VisualParams* m_vparams;

};

class SOFA_SOFARHI_API RHIVisualDrawInitResourcesVisitor : public RHIVisualDrawVisitor
{
public:
    RHIVisualDrawInitResourcesVisitor(core::visual::VisualParams* params)
        : RHIVisualDrawVisitor(params) {}

    Result processNodeTopDown(simulation::Node* node) override;
    void processObject(simulation::Node* /*node*/, core::objectmodel::BaseObject* o) override;

    const char* getClassName() const override { return "RHIVisualDrawInitResourcesVisitor"; }

};

class SOFA_SOFARHI_API RHIVisualDrawUpdateResourcesVisitor : public RHIVisualDrawVisitor
{
public:
    using VisualStyle = component::visualmodel::VisualStyle;

    RHIVisualDrawUpdateResourcesVisitor(core::visual::VisualParams* params)
        : RHIVisualDrawVisitor(params) {}

    Result processNodeTopDown(simulation::Node* node) override;
    void processNodeBottomUp(simulation::Node* node) override;
    void processObject(simulation::Node* /*node*/, core::objectmodel::BaseObject* o) override;
    void processFwdVisualStyle(simulation::Node* /*node*/, VisualStyle* vs);
    void processBwdVisualStyle(simulation::Node* /*node*/, VisualStyle* vs);

    const char* getClassName() const override { return "RHIVisualDrawUpdateResourcesVisitor"; }
};

class SOFA_SOFARHI_API RHIVisualDrawUpdateCommandsVisitor : public RHIVisualDrawVisitor
{
public:
    RHIVisualDrawUpdateCommandsVisitor(core::visual::VisualParams* params)
        : RHIVisualDrawVisitor(params) {}

    Result processNodeTopDown(simulation::Node* node) override;
    void processObject(simulation::Node* /*node*/, core::objectmodel::BaseObject* o) override;

    const char* getClassName() const override { return "RHIVisualDrawUpdateCommandsVisitor"; }
};

} // namespace sofa::rhi


