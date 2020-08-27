#include <SofaRHI/RHIVisualManagerLoop.h>
#include <SofaRHI/RHIVisualVisitor.h>

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
    if ( !gRoot ) return;
    gRoot->execute<simulation::VisualInitVisitor>(params);
    // Do a visual update now as it is not done in load() anymore
    /// \todo Separate this into another method?
    gRoot->execute<simulation::VisualUpdateVisitor>(params);
}

void RHIVisualManagerLoop::updateStep(sofa::core::ExecParams* params)
{
    if ( !gRoot ) return;
#ifdef SOFA_DUMP_VISITOR_INFO
    simulation::Visitor::printNode("UpdateVisual");
#endif
    sofa::helper::AdvancedTimer::begin("UpdateVisual");

    gRoot->execute<simulation::VisualUpdateVisitor>(params);
    sofa::helper::AdvancedTimer::end("UpdateVisual");
#ifdef SOFA_DUMP_VISITOR_INFO
    simulation::Visitor::printCloseNode("UpdateVisual");
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

    if (gRoot->visualManager.empty())
    {
        vparams->pass() = sofa::core::visual::VisualParams::Std;
        RHIVisualDrawVisitor act ( vparams );
        act.setTags(this->getTags());
        gRoot->execute ( &act );
        vparams->pass() = sofa::core::visual::VisualParams::Transparent;
        RHIVisualDrawVisitor act2 ( vparams );
        act2.setTags(this->getTags());
        gRoot->execute ( &act2 );
    }
    else
    {
        simulation::Node::Sequence<core::visual::VisualManager>::iterator begin = gRoot->visualManager.begin(), end = gRoot->visualManager.end(), it;
        for (it = begin; it != end; ++it)
            (*it)->preDrawScene(vparams);
        bool rendered = false; // true if a manager did the rendering
        for (it = begin; it != end; ++it)
            if ((*it)->drawScene(vparams))
            {
                rendered = true;
                break;
            }
        if (!rendered) // do the rendering
        {
            vparams->pass() = sofa::core::visual::VisualParams::Std;

            RHIVisualDrawVisitor act ( vparams );
            act.setTags(this->getTags());
            gRoot->execute ( &act );
            vparams->pass() = sofa::core::visual::VisualParams::Transparent;
            RHIVisualDrawVisitor act2 ( vparams );
            act2.setTags(this->getTags());
            gRoot->execute ( &act2 );
        }
        simulation::Node::Sequence<core::visual::VisualManager>::reverse_iterator rbegin = gRoot->visualManager.rbegin(), rend = gRoot->visualManager.rend(), rit;
        for (rit = rbegin; rit != rend; ++rit)
            (*rit)->postDrawScene(vparams);
    }
}

void RHIVisualManagerLoop::computeBBoxStep(sofa::core::visual::VisualParams* vparams, SReal* minBBox, SReal* maxBBox, bool init)
{
    simulation::VisualComputeBBoxVisitor act(vparams);
    if ( gRoot )
        gRoot->execute ( act );
//    cerr<<"RHIVisualManagerLoop::computeBBoxStep, xm= " << act.minBBox[0] <<", xM= " << act.maxBBox[0] << endl;
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

} // namespace sofa::rhi
