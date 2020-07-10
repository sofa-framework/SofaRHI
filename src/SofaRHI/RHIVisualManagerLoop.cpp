#include <SofaQt3D/Qt3DVisualManagerLoop.h>
#include <SofaQt3D/Qt3DVisualVisitor.h>

#include <sofa/core/ObjectFactory.h>
#include <sofa/core/visual/VisualParams.h>

#include <sofa/simulation/VisualVisitor.h>
#include <sofa/simulation/UpdateContextVisitor.h>
#include <sofa/simulation/UpdateMappingVisitor.h>
#include <sofa/simulation/UpdateMappingEndEvent.h>
#include <sofa/simulation/PropagateEventVisitor.h>


#include <sofa/helper/AdvancedTimer.h>

namespace sofa::qt3d
{

int Qt3DVisualManagerLoopClass = core::RegisterObject("Visual Loop Manager for Qt3D render")
        .add< Qt3DVisualManagerLoop >()
;

Qt3DVisualManagerLoop::Qt3DVisualManagerLoop(simulation::Node* _gnode)
    : Inherit()
    , gRoot(_gnode)
{
    //assert(gRoot);
}

Qt3DVisualManagerLoop::~Qt3DVisualManagerLoop()
{

}

void Qt3DVisualManagerLoop::init()
{
    if (!gRoot)
        gRoot = dynamic_cast<simulation::Node*>(this->getContext());
}


void Qt3DVisualManagerLoop::initStep(sofa::core::ExecParams* params)
{
    if ( !gRoot ) return;
    gRoot->execute<simulation::VisualInitVisitor>(params);
    // Do a visual update now as it is not done in load() anymore
    /// \todo Separate this into another method?
    gRoot->execute<simulation::VisualUpdateVisitor>(params);
}

void Qt3DVisualManagerLoop::updateStep(sofa::core::ExecParams* params)
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

void Qt3DVisualManagerLoop::updateContextStep(sofa::core::visual::VisualParams* vparams)
{
    simulation::UpdateVisualContextVisitor vis(vparams);
    vis.execute(gRoot);


}

void Qt3DVisualManagerLoop::drawStep(sofa::core::visual::VisualParams* vparams)
{
    if ( !gRoot ) return;

    if (gRoot->visualManager.empty())
    {
        vparams->pass() = sofa::core::visual::VisualParams::Std;
        Qt3DVisualDrawVisitor act ( vparams );
        act.setTags(this->getTags());
        gRoot->execute ( &act );
        vparams->pass() = sofa::core::visual::VisualParams::Transparent;
        Qt3DVisualDrawVisitor act2 ( vparams );
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

            Qt3DVisualDrawVisitor act ( vparams );
            act.setTags(this->getTags());
            gRoot->execute ( &act );
            vparams->pass() = sofa::core::visual::VisualParams::Transparent;
            Qt3DVisualDrawVisitor act2 ( vparams );
            act2.setTags(this->getTags());
            gRoot->execute ( &act2 );
        }
        simulation::Node::Sequence<core::visual::VisualManager>::reverse_iterator rbegin = gRoot->visualManager.rbegin(), rend = gRoot->visualManager.rend(), rit;
        for (rit = rbegin; rit != rend; ++rit)
            (*rit)->postDrawScene(vparams);
    }
}

void Qt3DVisualManagerLoop::computeBBoxStep(sofa::core::visual::VisualParams* vparams, SReal* minBBox, SReal* maxBBox, bool init)
{
    simulation::VisualComputeBBoxVisitor act(vparams);
    if ( gRoot )
        gRoot->execute ( act );
//    cerr<<"Qt3DVisualManagerLoop::computeBBoxStep, xm= " << act.minBBox[0] <<", xM= " << act.maxBBox[0] << endl;
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

} // namespace sofa::qt3d
