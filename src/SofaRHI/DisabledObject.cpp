#include <SofaRHI/DisabledObject.h>

#include <sofa/core/ObjectFactory.h>

namespace sofa::rhi
{

SOFA_DECL_CLASS(DisabledObject)

int DisabledObjectClass = core::RegisterObject("DisabledObject")
.add< DisabledObject>()
;

} // namespace sofa::rhi
