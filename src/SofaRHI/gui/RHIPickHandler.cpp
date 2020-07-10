#include <SofaQt3D/gui/Qt3DPickHandler.h>

namespace sofa::qt3d
{

Qt3DPickHandler::BodyPicked Qt3DPickHandler::findCollision()
{
    BodyPicked result;
    QPoint qtMousePosition {mousePosition.x, mousePosition.y};

    m_screenRayCaster->trigger(qtMousePosition);

    Qt3DRender::QAbstractRayCaster::Hits hits = m_screenRayCaster->hits();

    qDebug() << hits.size();
    for(auto h : hits)
        qDebug() << h.entity()->objectName() << " " << h.worldIntersection();

    return Qt3DPickHandler::BodyPicked();
}


} // namespace sofa::qt3d
