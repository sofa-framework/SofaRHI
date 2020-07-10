#include <SofaQt3D/Qt3DVisualVisitor.h>

namespace sofa::qt3d
{

//sofa::simulation::Visitor::Result Qt3DVisualDrawVisitor::processNodeTopDown(simulation::Node* node)
//{
//    for_each(this, node, node->visualModel, &Qt3DVisualDrawVisitor::fwdVisualModel);
//    this->VisualVisitor::processNodeTopDown(node);

//    return RESULT_CONTINUE;
//}

void Qt3DVisualDrawVisitor::processNodeBottomUp(simulation::Node* node)
{
    for_each(this, node, node->visualModel, &Qt3DVisualDrawVisitor::bwdVisualModel);
}

void Qt3DVisualDrawVisitor::fwdVisualModel(simulation::Node* node, core::visual::VisualModel* vm)
{
    vm->fwdDraw(vparams);
}

void Qt3DVisualDrawVisitor::bwdVisualModel(simulation::Node* node, core::visual::VisualModel* vm)
{
    vm->bwdDraw(vparams);
}

sofa::simulation::Visitor::Result Qt3DVisualDrawVisitor::processNodeTopDown(simulation::Node* node)
{
    for_each(this, node, node->visualModel, &Qt3DVisualDrawVisitor::fwdVisualModel);

    for_each(this, node, node->collisionModel, &Qt3DVisualDrawVisitor::processCollisionModel);
    for_each(this, node, node->object, &Qt3DVisualDrawVisitor::processOtherObject);
    for_each(this, node, node->visualModel, &Qt3DVisualDrawVisitor::processVisualModel);

    return RESULT_CONTINUE;
}

void Qt3DVisualDrawVisitor::processVisualModel(simulation::Node* node, core::visual::VisualModel* vm)
{
    switch(vparams->pass())
    {
    case core::visual::VisualParams::Std:
    {
        vm->drawVisual(vparams);
        break;
    }
    case core::visual::VisualParams::Transparent:
    {
        vm->drawTransparent(vparams);
        break;
    }
    case core::visual::VisualParams::Shadow:
        vm->drawShadow(vparams);
     break;
    }
}

void Qt3DVisualDrawVisitor::processOtherObject(simulation::Node* /*node*/, core::objectmodel::BaseObject* o)
{
    if(dynamic_cast<core::CollisionModel*>(o) == nullptr)
    {
        if (vparams->pass() == core::visual::VisualParams::Transparent || vparams->pass() == core::visual::VisualParams::Shadow)
        {
            if(m_qt3dDrawTool)
                m_qt3dDrawTool->startDrawingObject(o->getName());

            o->draw(vparams);

            if(m_qt3dDrawTool)
                m_qt3dDrawTool->stopDrawingObject();
        }
    }
}

void Qt3DVisualDrawVisitor::processCollisionModel(simulation::Node* /*node*/, core::CollisionModel* cm)
{

    if (vparams->pass() == core::visual::VisualParams::Transparent || vparams->pass() == core::visual::VisualParams::Shadow)
    {
        if(m_qt3dDrawTool)
            m_qt3dDrawTool->startDrawingObject(cm->getName(), "CollisionModel");

        cm->draw(vparams);

        if(m_qt3dDrawTool)
            m_qt3dDrawTool->stopDrawingObject();
    }
}

} // namespace sofa::qt3d
