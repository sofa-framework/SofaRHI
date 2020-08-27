#include <SofaRHI/RHIVisualVisitor.h>

namespace sofa::rhi
{

void RHIVisualDrawVisitor::processNodeBottomUp(simulation::Node* node)
{
    for_each(this, node, node->visualModel, &RHIVisualDrawVisitor::bwdVisualModel);
}

void RHIVisualDrawVisitor::fwdVisualModel(simulation::Node* node, core::visual::VisualModel* vm)
{
    vm->fwdDraw(vparams);
}

void RHIVisualDrawVisitor::bwdVisualModel(simulation::Node* node, core::visual::VisualModel* vm)
{
    vm->bwdDraw(vparams);
}

sofa::simulation::Visitor::Result RHIVisualDrawVisitor::processNodeTopDown(simulation::Node* node)
{
    for_each(this, node, node->visualModel, &RHIVisualDrawVisitor::fwdVisualModel);

    for_each(this, node, node->object, &RHIVisualDrawVisitor::processOtherObject);
    for_each(this, node, node->visualModel, &RHIVisualDrawVisitor::processVisualModel);

    return RESULT_CONTINUE;
}

void RHIVisualDrawVisitor::processVisualModel(simulation::Node* node, core::visual::VisualModel* vm)
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

void RHIVisualDrawVisitor::processOtherObject(simulation::Node* /*node*/, core::objectmodel::BaseObject* o)
{
    if (vparams->pass() == core::visual::VisualParams::Transparent || vparams->pass() == core::visual::VisualParams::Shadow)
    {
        if(m_rhiDrawTool)
            m_rhiDrawTool->startDrawingObject(o->getName());

        o->draw(vparams);

        if(m_rhiDrawTool)
            m_rhiDrawTool->stopDrawingObject();
    }
}


} // namespace sofa::rhi
