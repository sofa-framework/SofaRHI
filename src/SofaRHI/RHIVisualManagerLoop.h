#pragma once

#include <SofaQt3D/config.h>

#include <sofa/core/visual/VisualLoop.h>
#include <sofa/simulation/Node.h>

namespace sofa::qt3d
{

class SOFA_SOFAQT3D_API Qt3DVisualManagerLoop : public sofa::core::visual::VisualLoop
{
public:
    using Inherit = sofa::core::visual::VisualLoop;

    SOFA_CLASS(Qt3DVisualManagerLoop, Inherit);

protected:
    Qt3DVisualManagerLoop(simulation::Node* gnode = nullptr);

    ~Qt3DVisualManagerLoop() override;
public:
    void init() override;

    /// Initialize the textures
    void initStep(sofa::core::ExecParams* params) override;

    /// Update the Visual Models: triggers the Mappings
    void updateStep(sofa::core::ExecParams* params) override;

    /// Update contexts. Required before drawing the scene if root flags are modified.
    void updateContextStep(sofa::core::visual::VisualParams* vparams) override;

    /// Render the scene
    void drawStep(sofa::core::visual::VisualParams* vparams) override;

    /// Compute the bounding box of the scene. If init is set to "true", then minBBox and maxBBox will be initialised to a default value
    void computeBBoxStep(sofa::core::visual::VisualParams* vparams, SReal* minBBox, SReal* maxBBox, bool init) override;

protected:

    simulation::Node* gRoot;
};

} // namespace sofa::qt3d
