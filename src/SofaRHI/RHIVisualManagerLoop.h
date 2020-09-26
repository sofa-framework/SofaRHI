#pragma once

#include <SofaRHI/config.h>

#include <sofa/core/visual/VisualLoop.h>
#include <sofa/simulation/Node.h>
#include <SofaRHI/RHIVisualModel.h>

namespace sofa::rhi
{

class SOFA_SOFARHI_API RHIVisualManagerLoop : public sofa::core::visual::VisualLoop
{
public:
    using Inherit = sofa::core::visual::VisualLoop;

    SOFA_CLASS(RHIVisualManagerLoop, Inherit);

protected:
    RHIVisualManagerLoop(simulation::Node* gnode = nullptr);

    ~RHIVisualManagerLoop() override;
public:
    void init() override;

    /// init step (aka initialize buffers, pipeline, etc)
    void initStep(sofa::core::ExecParams* params) override;

    /// Update the Visual Models: triggers the Mappings
    void updateStep(sofa::core::ExecParams* params) override;

    /// Update contexts. Required before drawing the scene if root flags are modified.
    /// (aka update resources)
    void updateContextStep(sofa::core::visual::VisualParams* vparams) override;

    /// Render the scene (aka update command buffers)
    void drawStep(sofa::core::visual::VisualParams* vparams) override;

    /// Compute the bounding box of the scene. If init is set to "true", then minBBox and maxBBox will be initialised to a default value
    void computeBBoxStep(sofa::core::visual::VisualParams* vparams, SReal* minBBox, SReal* maxBBox, bool init) override;

    // Own RHI update
    // (can not be inserted alongside updateStep(), as updateStep is called by Simulation() at a time there is no "RHI Context"
    void updateRHIResourcesStep(sofa::core::visual::VisualParams* vparams);

    // Compute Calls
    // Initialize RHI Compute resources for RHIComputeModels
    void initComputeCommandsStep(sofa::core::visual::VisualParams* vparams);
    // Update RHI Compute resources for RHIComputeModels
    void updateComputeResourcesStep(sofa::core::visual::VisualParams* vparams);
    // Update RHI Compute commands for RHIComputeModels
    void updateComputeCommandsStep(sofa::core::visual::VisualParams* vparams);

protected:

    simulation::Node* gRoot;
};

} // namespace sofa::qt3d
