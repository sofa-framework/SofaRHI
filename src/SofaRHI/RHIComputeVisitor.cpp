#include <SofaRHI/RHIComputeVisitor.h>

namespace sofa::rhi
{


sofa::simulation::Visitor::Result RHIComputeInitResourcesVisitor::processNodeTopDown(simulation::Node* node)
{
    for_each(this, node, node->object, &RHIComputeInitResourcesVisitor::processObject);

    return RESULT_CONTINUE;
}

void RHIComputeInitResourcesVisitor::processObject(simulation::Node* /*node*/, core::objectmodel::BaseObject* o)
{
    //not very efficient but node does not store RHIComputeModel, contrary to VisualModel
    RHIComputeModel* rvm = dynamic_cast<RHIComputeModel*>(o);

    if (rvm) // RHIComputeModel
    {
        QRhiPtr rhi = m_rhiDrawTool->getRHI();
        QRhiRenderPassDescriptorPtr rpDesc = m_rhiDrawTool->getRenderPassDescriptor();

        rvm->initComputeResources(rhi);
    }
    else // other than RHIComputeModel
    {
        // not init resources for the other types of objects whatsoever
    }
}


sofa::simulation::Visitor::Result RHIComputeUpdateResourcesVisitor::processNodeTopDown(simulation::Node* node)
{
    for_each(this, node, node->object, &RHIComputeUpdateResourcesVisitor::processObject);

    return RESULT_CONTINUE;
}

void RHIComputeUpdateResourcesVisitor::processObject(simulation::Node* /*node*/, core::objectmodel::BaseObject* o)
{
    RHIComputeModel* rvm = dynamic_cast<RHIComputeModel*>(o);

    if (rvm) // RHIComputeModel
    {
        QRhiResourceUpdateBatch* batch = m_rhiDrawTool->getResourceUpdateBatch();

        rvm->updateComputeResources(batch);
    }
    else // other than RHIComputeModel
    {
        // Nope
    }

}

sofa::simulation::Visitor::Result RHIComputeUpdateCommandsVisitor::processNodeTopDown(simulation::Node* node)
{
    for_each(this, node, node->object, &RHIComputeUpdateCommandsVisitor::processObject);

    return RESULT_CONTINUE;
}

void RHIComputeUpdateCommandsVisitor::processObject(simulation::Node* /*node*/, core::objectmodel::BaseObject* o)
{
    RHIComputeModel* rvm = dynamic_cast<RHIComputeModel*>(o);

    if (rvm) // RHIComputeModel
    {
        QRhiCommandBuffer* cb = m_rhiDrawTool->getCommandBuffer();
        const QRhiViewport& viewport = m_rhiDrawTool->getViewport();

        rvm->updateComputeCommands(cb);
    }
    else // other than RHIComputeModel
    {
        // commands are done by the drawtool itself
    }

}


} // namespace sofa::rhi
