#include <SofaRHI/RHIVisualVisitor.h>

#include <SofaBaseVisual/VisualStyle.h>

namespace sofa::rhi
{


sofa::simulation::Visitor::Result RHIVisualDrawInitResourcesVisitor::processNodeTopDown(simulation::Node* node)
{
    for_each(this, node, node->object, &RHIVisualDrawInitResourcesVisitor::processObject);

    return RESULT_CONTINUE;
}

void RHIVisualDrawInitResourcesVisitor::processObject(simulation::Node* /*node*/, core::objectmodel::BaseObject* o)
{
    //not very efficient but node does not store RHIVisualModel, contrary to VisualModel
    RHIVisualModel* rvm = dynamic_cast<RHIVisualModel*>(o);

    if (rvm) // RHIVisualModel
    {
        QRhiPtr rhi = m_rhiDrawTool->getRHI();
        QRhiRenderPassDescriptorPtr rpDesc = m_rhiDrawTool->getRenderPassDescriptor();

        rvm->initRHIResources(rhi, rpDesc);
    }
    else // other than RHIVisualModel
    {
        // not init resources for the other types of objects whatsoever
    }
}


sofa::simulation::Visitor::Result RHIVisualDrawUpdateResourcesVisitor::processNodeTopDown(simulation::Node* node)
{
    //Preliminary: call fwdDraw of all VisualStyle

    std::vector<VisualStyle*> vstyles;
    node->get<VisualStyle, std::vector<VisualStyle*> >(&vstyles, sofa::core::objectmodel::BaseContext::Local);

    //for_each(this, node, vstyles, &RHIVisualDrawUpdateResourcesVisitor::processFwdVisualStyle); //lol
    for (auto& vstyle : vstyles)
    {
        vstyle->fwdDraw(m_vparams);
    }

    for_each(this, node, node->object, &RHIVisualDrawUpdateResourcesVisitor::processObject);

    return RESULT_CONTINUE;
}


void RHIVisualDrawUpdateResourcesVisitor::processNodeBottomUp(simulation::Node* node)
{
    //call bwdDraw of all VisualStyle
    std::vector<VisualStyle*> vstyles;
    node->get<VisualStyle, std::vector<VisualStyle*> >(&vstyles, sofa::core::objectmodel::BaseContext::Local);

    //for_each(this, node, vstyles, &RHIVisualDrawUpdateResourcesVisitor::processBwdVisualStyle); // lol
    for (auto& vstyle : vstyles)
    {
        vstyle->bwdDraw(m_vparams);
    }
}

void RHIVisualDrawUpdateResourcesVisitor::processFwdVisualStyle(simulation::Node* /*node*/, VisualStyle* vs)
{
    vs->fwdDraw(m_vparams);
}
void RHIVisualDrawUpdateResourcesVisitor::processBwdVisualStyle(simulation::Node* /*node*/, VisualStyle* vs)
{
    vs->bwdDraw(m_vparams);
}

void RHIVisualDrawUpdateResourcesVisitor::processObject(simulation::Node* /*node*/, core::objectmodel::BaseObject* o)
{

    RHIVisualModel* rvm = dynamic_cast<RHIVisualModel*>(o);

    if (rvm) // RHIVisualModel
    {
        QRhiResourceUpdateBatch* batch = m_rhiDrawTool->getResourceUpdateBatch();

        rvm->updateRHIResources(batch);
    }
    else // other than RHIVisualModel
    {
        // EXCEPTION: VisualStyle: we need its fwdDraw and bwdDraw
        VisualStyle* vstyle = dynamic_cast<VisualStyle*>(o);
        if (vstyle)
        {
            vstyle->fwdDraw(m_vparams);
        }
        // RHI resources are effectively set while doing the draw() function
        o->draw(m_vparams);
    }

}

sofa::simulation::Visitor::Result RHIVisualDrawUpdateCommandsVisitor::processNodeTopDown(simulation::Node* node)
{
    for_each(this, node, node->object, &RHIVisualDrawUpdateCommandsVisitor::processObject);

    return RESULT_CONTINUE;
}

void RHIVisualDrawUpdateCommandsVisitor::processObject(simulation::Node* /*node*/, core::objectmodel::BaseObject* o)
{
    RHIVisualModel* rvm = dynamic_cast<RHIVisualModel*>(o);

    if (rvm) // RHIVisualModel
    {
        QRhiCommandBuffer* cb = m_rhiDrawTool->getCommandBuffer();
        const QRhiViewport& viewport = m_rhiDrawTool->getViewport();

        rvm->updateRHICommands(cb, viewport);
    }
    else // other than RHIVisualModel
    {
        // commands are done by the drawtool itself
    }

}


} // namespace sofa::rhi
