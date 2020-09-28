#include <SofaRHI/RHIVisualManagerLoop.h>
#include <SofaRHI/RHIVisualVisitor.h>
#include <SofaRHI/RHIComputeVisitor.h>

#include <sofa/core/ObjectFactory.h>
#include <sofa/core/visual/VisualParams.h>

#include <sofa/simulation/VisualVisitor.h>
#include <sofa/simulation/UpdateContextVisitor.h>
#include <sofa/simulation/UpdateMappingVisitor.h>
#include <sofa/simulation/UpdateMappingEndEvent.h>
#include <sofa/simulation/PropagateEventVisitor.h>

#include <sofa/helper/AdvancedTimer.h>

namespace sofa::rhi
{

int RHIVisualManagerLoopClass = core::RegisterObject("Visual Loop Manager for the RHI render")
        .add< RHIVisualManagerLoop >()
;

RHIVisualManagerLoop::RHIVisualManagerLoop(simulation::Node* _gnode)
    : Inherit()
    , gRoot(_gnode)
{
    //assert(gRoot);
}

RHIVisualManagerLoop::~RHIVisualManagerLoop()
{

}

void RHIVisualManagerLoop::init()
{
    if (!gRoot)
        gRoot = dynamic_cast<simulation::Node*>(this->getContext());
}



void RHIVisualManagerLoop::initStep(sofa::core::ExecParams* params)
{
    if (!gRoot) return;

    gRoot->execute<sofa::simulation::VisualInitVisitor>(params);

    //I DONT UNDERSTAND WHY THERE IS NO VISUALPARAMS IN A VISUAL INITIALIZATION !111!!!
    auto vparams = sofa::core::visual::VisualParams::defaultInstance();

    RHIGraphicInitResourcesVisitor initVisitor(vparams);
    gRoot->execute(&initVisitor);

    // Do a visual update now as it is not done in load() anymore
    /// \todo Separate this into another method?
    gRoot->execute<sofa::simulation::VisualUpdateVisitor>(params);
}

void RHIVisualManagerLoop::updateStep(sofa::core::ExecParams* params)
{
    if (!gRoot) return;

#ifdef SOFA_DUMP_VISITOR_INFO
    simulation::Visitor::printNode("UpdateVisual");
#endif
    gRoot->execute<sofa::simulation::VisualUpdateVisitor>(params);

#ifdef SOFA_DUMP_VISITOR_INFO
    simulation::Visitor::printCloseNode("UpdateVisual");
#endif
}

void RHIVisualManagerLoop::updateRHIResourcesStep(sofa::core::visual::VisualParams* vparams)
{
    if (!gRoot) return;

#ifdef SOFA_DUMP_VISITOR_INFO
    simulation::Visitor::printNode("UpdateRHIResources");
#endif

    RHIGraphicUpdateResourcesVisitor updateVisitor(vparams);
    gRoot->execute(&updateVisitor);

#ifdef SOFA_DUMP_VISITOR_INFO
    simulation::Visitor::printCloseNode("UpdateRHIResources");
#endif
}

void RHIVisualManagerLoop::updateContextStep(sofa::core::visual::VisualParams* vparams)
{
    if (!gRoot) return;
    //dont really understand what does that do ??
    //(from the original DefaultVisualManagerLoop
    simulation::UpdateVisualContextVisitor vis(vparams);
    vis.execute(gRoot);
}

void RHIVisualManagerLoop::drawStep(sofa::core::visual::VisualParams* vparams)
{
    if ( !gRoot ) return;
    
    vparams->pass() = sofa::core::visual::VisualParams::Std;

    // RHI
    RHIGraphicUpdateCommandsVisitor act ( vparams );
    act.setTags(this->getTags());
    gRoot->execute ( &act );
}

void RHIVisualManagerLoop::computeBBoxStep(sofa::core::visual::VisualParams* vparams, SReal* minBBox, SReal* maxBBox, bool init)
{
    simulation::VisualComputeBBoxVisitor act(vparams);
    if ( gRoot )
        gRoot->execute ( act );

    if (init)
    {
        minBBox[0] = (SReal)(act.minBBox[0]);
        minBBox[1] = (SReal)(act.minBBox[1]);
        minBBox[2] = (SReal)(act.minBBox[2]);
        maxBBox[0] = (SReal)(act.maxBBox[0]);
        maxBBox[1] = (SReal)(act.maxBBox[1]);
        maxBBox[2] = (SReal)(act.maxBBox[2]);
    }
    else
    {
        if ((SReal)(act.minBBox[0]) < minBBox[0] ) minBBox[0] = (SReal)(act.minBBox[0]);
        if ((SReal)(act.minBBox[1]) < minBBox[1] ) minBBox[1] = (SReal)(act.minBBox[1]);
        if ((SReal)(act.minBBox[2]) < minBBox[2] ) minBBox[2] = (SReal)(act.minBBox[2]);
        if ((SReal)(act.maxBBox[0]) > maxBBox[0] ) maxBBox[0] = (SReal)(act.maxBBox[0]);
        if ((SReal)(act.maxBBox[1]) > maxBBox[1] ) maxBBox[1] = (SReal)(act.maxBBox[1]);
        if ((SReal)(act.maxBBox[2]) > maxBBox[2] ) maxBBox[2] = (SReal)(act.maxBBox[2]);
    }
}

void RHIVisualManagerLoop::initComputeCommandsStep(sofa::core::visual::VisualParams* vparams)
{
    if (!gRoot) return;

    RHIComputeInitResourcesVisitor compInitVisitor(vparams);
    gRoot->execute(&compInitVisitor);
}

void RHIVisualManagerLoop::updateComputeResourcesStep(sofa::core::visual::VisualParams* vparams)
{
    if (!gRoot) return;

    RHIComputeUpdateResourcesVisitor compUpdateResVisitor(vparams);
    gRoot->execute(&compUpdateResVisitor);
}
void RHIVisualManagerLoop::updateComputeCommandsStep(sofa::core::visual::VisualParams* vparams)
{
    if (!gRoot) return;

    RHIComputeUpdateCommandsVisitor compUpdateComVisitor(vparams);
    gRoot->execute(&compUpdateComVisitor);
}

} // namespace sofa::rhi
