#include <SofaQt3D/Qt3DObject.h>

namespace sofa::qt3d
{


Qt3DObject::Qt3DObject()
    : m_rootEntity(new Qt3DCore::QEntity())
{

}

Qt3DObject::~Qt3DObject()
{

}

Qt3DCore::QEntity* Qt3DObject::getRootEntity()
{
    return m_rootEntity;
}

} // namespace sofa::qt3d
