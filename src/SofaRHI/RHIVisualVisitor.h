#pragma once

#include <SofaQt3D/config.h>

#include <sofa/core/visual/VisualParams.h>
#include <sofa/simulation/Visitor.h>
#include <SofaQt3D/DrawToolQt3D.h>

namespace sofa::qt3d
{

class SOFA_SOFAQT3D_API Qt3DVisualDrawVisitor : public sofa::simulation::Visitor
{
public:
    Qt3DVisualDrawVisitor(core::visual::VisualParams* params)
        : sofa::simulation::Visitor(params)
        , m_qt3dDrawTool(nullptr)
    {
        vparams = params;
        m_qt3dDrawTool = dynamic_cast<sofa::core::visual::DrawToolQt3D*>(params->drawTool());
    }

    Result processNodeTopDown(simulation::Node* node) override;
    void processNodeBottomUp(simulation::Node* node) override;

    virtual void processVisualModel(simulation::Node* node, core::visual::VisualModel* vm);
    virtual void processOtherObject(simulation::Node* /*node*/, core::objectmodel::BaseObject* o);
    virtual void processCollisionModel(simulation::Node* /*node*/, core::CollisionModel* cm);

    virtual void fwdVisualModel(simulation::Node* node, core::visual::VisualModel* vm);
    virtual void bwdVisualModel(simulation::Node* node, core::visual::VisualModel* vm);

    const char* getCategoryName() const override { return "visual"; }
    const char* getClassName() const override { return "Qt3DVisualVisitor"; }

    /// qt3d visual visitor must be executed as a tree, such as forward and backward orders are coherent
    bool treeTraversal(TreeTraversalRepetition& repeat) override { repeat=NO_REPETITION; return true; }
private:
    sofa::core::visual::DrawToolQt3D* m_qt3dDrawTool;
    core::visual::VisualParams* vparams;

};

} // namespace sofa::qt3d


