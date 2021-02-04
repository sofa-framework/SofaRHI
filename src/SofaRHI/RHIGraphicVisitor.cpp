#include <SofaRHI/RHIGraphicVisitor.h>

#include <SofaBaseVisual/VisualStyle.h>
#include <sofa/simulation/Node.h>

namespace sofa::rhi
{


sofa::simulation::Visitor::Result RHIGraphicInitResourcesVisitor::processNodeTopDown(simulation::Node* node)
{
    for_each(this, node, node->object, &RHIGraphicInitResourcesVisitor::processObject);

    return RESULT_CONTINUE;
}

void RHIGraphicInitResourcesVisitor::processObject(simulation::Node* /*node*/, core::objectmodel::BaseObject* o)
{
    //not very efficient but node does not store RHIGraphicModel, contrary to VisualModel
    RHIGraphicModel* rgm = dynamic_cast<RHIGraphicModel*>(o);

    if (rgm) // RHIGraphicModel
    {
        QRhiPtr rhi = m_rhiDrawTool->getRHI();
        QRhiRenderPassDescriptorPtr rpDesc = m_rhiDrawTool->getRenderPassDescriptor();

        rgm->initGraphicResources(rhi, rpDesc);
    }
    else // other than RHIGraphicModel
    {
        // not init resources for the other types of objects whatsoever
    }
}


sofa::simulation::Visitor::Result RHIGraphicUpdateResourcesVisitor::processNodeTopDown(simulation::Node* node)
{
    //Preliminary: call fwdDraw of all VisualStyle

    std::vector<VisualStyle*> vstyles;
    node->get<VisualStyle, std::vector<VisualStyle*> >(&vstyles, sofa::core::objectmodel::BaseContext::Local);

    //for_each(this, node, vstyles, &RHIGraphicUpdateResourcesVisitor::processFwdVisualStyle); //lol
    for (auto& vstyle : vstyles)
    {
        vstyle->fwdDraw(m_vparams);
    }

    for_each(this, node, node->object, &RHIGraphicUpdateResourcesVisitor::processObject);

    return RESULT_CONTINUE;
}


void RHIGraphicUpdateResourcesVisitor::processNodeBottomUp(simulation::Node* node)
{
    //call bwdDraw of all VisualStyle
    std::vector<VisualStyle*> vstyles;
    node->get<VisualStyle, std::vector<VisualStyle*> >(&vstyles, sofa::core::objectmodel::BaseContext::Local);

    //for_each(this, node, vstyles, &RHIGraphicUpdateResourcesVisitor::processBwdVisualStyle); // lol
    for (auto& vstyle : vstyles)
    {
        vstyle->bwdDraw(m_vparams);
    }
}

void RHIGraphicUpdateResourcesVisitor::processFwdVisualStyle(simulation::Node* /*node*/, VisualStyle* vs)
{
    vs->fwdDraw(m_vparams);
}
void RHIGraphicUpdateResourcesVisitor::processBwdVisualStyle(simulation::Node* /*node*/, VisualStyle* vs)
{
    vs->bwdDraw(m_vparams);
}

void RHIGraphicUpdateResourcesVisitor::processObject(simulation::Node* /*node*/, core::objectmodel::BaseObject* o)
{
    RHIGraphicModel* rgm = dynamic_cast<RHIGraphicModel*>(o);

    if (rgm) // RHIGraphicModel
    {
        QRhiResourceUpdateBatch* batch = m_rhiDrawTool->getResourceUpdateBatch();

        rgm->updateGraphicResources(batch);
    }
    else // other than RHIGraphicModel
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

sofa::simulation::Visitor::Result RHIGraphicUpdateCommandsVisitor::processNodeTopDown(simulation::Node* node)
{
    for_each(this, node, node->object, &RHIGraphicUpdateCommandsVisitor::processObject);

    return RESULT_CONTINUE;
}

void RHIGraphicUpdateCommandsVisitor::processObject(simulation::Node* /*node*/, core::objectmodel::BaseObject* o)
{
    RHIGraphicModel* rvm = dynamic_cast<RHIGraphicModel*>(o);

    if (rvm) // RHIGraphicModel
    {
        QRhiCommandBuffer* cb = m_rhiDrawTool->getCommandBuffer();
        const QRhiViewport& viewport = m_rhiDrawTool->getViewport();

        rvm->updateGraphicCommands(cb, viewport);
    }
    else // other than RHIGraphicModel
    {
        // commands are done by the drawtool itself
    }

}


} // namespace sofa::rhi
