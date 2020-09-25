#pragma once

#include <SofaRHI/config.h>

#include <sofa/core/objectmodel/BaseObject.h>

namespace sofa::rhi
{

/// Does not do anything
/// Main purpose is to be a stub for removed opengl components
class SOFA_SOFARHI_API DisabledObject : public sofa::core::objectmodel::BaseObject
{
public:
    SOFA_CLASS(DisabledObject, sofa::core::objectmodel::BaseObject);


};

} // namespace sofa::rhi


