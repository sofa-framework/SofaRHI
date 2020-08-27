#pragma once

#include <SofaRHI/config.h>

#include <sofa/core/visual/VisualLoop.h>
#include <sofa/simulation/Node.h>
#include <SofaRHI/RHIVisualModel.h>

namespace sofa::rhi
{
//
//class SOFA_SOFARHI_API ResourceUpdateVisitor : public sofa::simulation::VisualVisitor
//{
//public:
//    ResourceUpdateVisitor(core::visual::VisualParams* params)
//        : VisualVisitor(params)
//    {}
//
//    void processVisualModel(simulation::Node*, core::visual::VisualModel* vm) override
//    {
//        RHIVisualModel* rhivm = dynamic_cast<RHIVisualModel*>(vm);
//        rhi::DrawToolRHI* rhiDrawTool = dynamic_cast<rhi::DrawToolRHI*>(this->vparams->drawTool());
//        QRhiCommandBuffer* cb = rhiDrawTool->getCommandBuffer();
//        const QRhiViewport& viewport = rhiDrawTool->getViewport();
//        auto batch = rhiDrawTool->getResourceUpdateBatch();
//
//        if(rhivm != nullptr)
//            rhivm->updateRHIResources(batch);
//    }
//    const char* getClassName() const override { return "ResourceUpdateVisitor"; }
//};

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

protected:

    simulation::Node* gRoot;
};

} // namespace sofa::qt3d
