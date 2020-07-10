#pragma once

#include <SofaQt3D/config.h>

#include <SofaQt3D/DrawToolQt3D.h>
#include <sofa/gui/PickHandler.h>
#include <SofaUserInteraction/MouseInteractor.h>

#include <Qt3DRender/QScreenRayCaster>

namespace sofa::qt3d
{

class SOFA_SOFAQT3D_API Qt3DPickHandler : public sofa::gui::PickHandler
{
public:
    using Inherit = sofa::gui::PickHandler;
    using BodyPicked = sofa::component::collision::BodyPicked;

    Qt3DPickHandler(Qt3DCore::QEntity* rootEntity)
        :Inherit()
        , m_rootEntity(rootEntity)
    {
        m_screenRayCaster = new Qt3DRender::QScreenRayCaster(rootEntity);
        m_screenRayCaster->setRunMode(Qt3DRender::QAbstractRayCaster::SingleShot);
        rootEntity->addComponent(m_screenRayCaster);
    }
    virtual ~Qt3DPickHandler() override {}

    BodyPicked findCollision() override;


private:
    Qt3DCore::QEntity* m_rootEntity;
    Qt3DRender::QScreenRayCaster* m_screenRayCaster;

};

} // namespace sofa::qt3d
