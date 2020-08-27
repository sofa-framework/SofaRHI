#pragma once

#include <SofaRHI/config.h>

#include <sofa/core/visual/VisualParams.h>
#include <sofa/simulation/Visitor.h>
#include <SofaRHI/DrawToolRHI.h>

namespace sofa::rhi
{

class SOFA_SOFARHI_API RHIVisualDrawVisitor : public sofa::simulation::Visitor
{
public:
    RHIVisualDrawVisitor(core::visual::VisualParams* params)
        : sofa::simulation::Visitor(params)
        , m_rhiDrawTool(nullptr)
    {
        vparams = params;
        m_rhiDrawTool = dynamic_cast<sofa::rhi::DrawToolRHI*>(params->drawTool());
    }

    Result processNodeTopDown(simulation::Node* node) override;
    void processNodeBottomUp(simulation::Node* node) override;

    virtual void processVisualModel(simulation::Node* node, core::visual::VisualModel* vm);
    virtual void processOtherObject(simulation::Node* /*node*/, core::objectmodel::BaseObject* o);

    virtual void fwdVisualModel(simulation::Node* node, core::visual::VisualModel* vm);
    virtual void bwdVisualModel(simulation::Node* node, core::visual::VisualModel* vm);

    const char* getCategoryName() const override { return "visual"; }
    const char* getClassName() const override { return "RHIVisualVisitor"; }

    /// qt3d visual visitor must be executed as a tree, such as forward and backward orders are coherent
    bool treeTraversal(TreeTraversalRepetition& repeat) override { repeat=NO_REPETITION; return true; }
private:
    sofa::rhi::DrawToolRHI* m_rhiDrawTool;
    core::visual::VisualParams* vparams;

};

} // namespace sofa::rhi


