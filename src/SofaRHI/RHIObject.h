#pragma once

#include <SofaQt3D/config.h>

#include <Qt3DCore/QEntity>

namespace sofa::qt3d
{


 /* This class does not inherit BaseObject because I dont want to mix up things
  *
  */

class SOFA_SOFAQT3D_API Qt3DObject
{
public:
    Qt3DObject();
    virtual ~Qt3DObject();

    Qt3DCore::QEntity* getRootEntity();
    virtual void initQt() =0;
    virtual void updateQt() =0;

private:
    Qt3DCore::QEntity* m_rootEntity;
};

} // namespace sofa::qt3d
