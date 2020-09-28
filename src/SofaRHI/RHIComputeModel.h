#pragma once

#include <SofaRHI/config.h>

#include <SofaRHI/RHIUtils.h>

namespace sofa::rhi
{

class SOFA_SOFARHI_API RHIComputeModel
{
public:
    virtual bool initComputeResources(QRhiPtr rhi) = 0;
    virtual void updateComputeResources(QRhiResourceUpdateBatch* batch) = 0;
    virtual void updateComputeCommands(QRhiCommandBuffer* cb) = 0;
};


} // namespace sofa::rhi
