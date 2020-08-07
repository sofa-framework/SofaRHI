#include <SofaRHI/gui/RHIViewer.h>

#include <sofa/helper/system/FileRepository.h>
#include <sofa/core/objectmodel/KeypressedEvent.h>
#include <sofa/core/objectmodel/KeyreleasedEvent.h>
#include <sofa/core/ObjectFactory.h>
#include <sofa/simulation/Simulation.h>
#include <sofa/simulation/DefaultVisualManagerLoop.h>

#include <sofa/gui/GUIManager.h>
#include <sofa/gui/qt/RealGUI.h>

#include <sofa/defaulttype/RigidTypes.h>
#include <sofa/gui/ColourPickingVisitor.h>

#include <sofa/helper/system/FileSystem.h>
#include <fstream>

#include <QtGui/private/qshader_p.h>
#if QT_CONFIG(opengl)
# include <QOpenGLContext>
# include <QtGui/private/qrhigles2_p.h>
# define TST_GL
#endif

#if QT_CONFIG(vulkan)
# include <QVulkanInstance>
# include <QtGui/private/qrhivulkan_p.h>
# define TST_VK
#endif

#ifdef Q_OS_WIN
#include <QtGui/private/qrhid3d11_p.h>
# define TST_D3D11
#endif

#ifdef Q_OS_DARWIN
# include <QtGui/private/qrhimetal_p.h>
# define TST_MTL
#endif

Q_DECLARE_METATYPE(QRhi::Implementation)
Q_DECLARE_METATYPE(QRhiInitParams*)

namespace sofa::rhi
{

using sofa::simulation::getSimulation;
using sofa::defaulttype::Vector3;
using sofa::defaulttype::Quat;

/// Takes the same view file format as qglviewer
const std::string RHIViewer::VIEW_FILE_EXTENSION = "rhiviewer.view";

RHIViewer::RHIViewer(QWidget* parent, const char* name, const unsigned int nbMSAASamples, bool replaceOgl)
    : QWidget(parent)
{
    this->setObjectName(name);

    //// RHI Setup
    QRhi::Implementation currentImpl = QRhi::OpenGLES2;
    //QRhi::Implementation currentImpl = QRhi::D3D11;
    QRhiGles2InitParams gl;
    gl.fallbackSurface = QRhiGles2InitParams::newFallbackSurface();

    m_rhi.reset(QRhi::create(currentImpl, &gl, QRhi::Flags(), nullptr));
    m_drawTool = new sofa::core::visual::DrawToolRHI(m_rhi);
    //m_rhi->addCleanupCallback(cleanupRHI);

    m_window = new QWindow();

    switch (currentImpl)
    {
    case QRhi::OpenGLES2:
#if QT_CONFIG(opengl)
        m_window->setFormat(QRhiGles2InitParams::adjustedFormat());
        msg_info("RHIViewer") << "Will use OpenGLES2";
#endif
        //Q_FALLTHROUGH();
        break;
    case QRhi::D3D11:
        m_window->setSurfaceType(QSurface::OpenGLSurface);
        msg_info("RHIViewer") << "Will use D3D11";
        break;
    case QRhi::Metal:
        m_window->setSurfaceType(QSurface::MetalSurface);
        msg_info("RHIViewer") << "Will use Metal";
        break;
    case QRhi::Vulkan:
        m_window->setSurfaceType(QSurface::VulkanSurface);
        msg_info("RHIViewer") << "Will use Vulkan";
#if QT_CONFIG(vulkan)
        window->setVulkanInstance(&vulkanInstance);
#endif
        break;
    default:
        break;
    }

    m_container = createWindowContainer(m_window, this);
    
    auto ds = m_rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil,
        QSize(), // no need to set the size yet
        1,
        QRhiRenderBuffer::UsedWithSwapChainOnly);

    m_swapChain = m_rhi->newSwapChain();
    m_swapChain->setWindow(m_window);
    m_swapChain->setDepthStencil(ds);

    m_swapChain->setFlags(QRhiSwapChain::UsedAsTransferSource);
    m_rpDesc.reset(m_swapChain->newCompatibleRenderPassDescriptor());
    m_swapChain->setRenderPassDescriptor(m_rpDesc.get());
    m_swapChain->buildOrResize();

    /////
    m_backend.reset(new RHIBackend(this));

    m_vparams = core::visual::VisualParams::defaultInstance();
    m_vparams->drawTool() = m_drawTool;


    ///
    sofa::core::ObjectFactory::ClassEntry::SPtr replaceOglModel;
    sofa::core::ObjectFactory::AddAlias("VisualModel", "RHIModel", true,
            &replaceOglModel);
    if(replaceOgl)
    {
        msg_warning("RHIViewer") << "All occcurences of OglModel will be replaced by RHIModel";
        sofa::core::ObjectFactory::AddAlias("OglModel", "RHIModel", true,
                &replaceOglModel);
    }
    groot = nullptr;
    initTexturesDone = false;

    timerAnimate = new QTimer(this);

    _video = false;
    m_bShowAxis = false;
    _background = 0;
    _waitForRender = false;

    ////////////////
    // Interactor //
    ////////////////
    m_isControlPressed = false;

    connect( &captureTimer, SIGNAL(timeout()), this, SLOT(captureEvent()) );

    //the view grabs events and does not give it back so we need to install an eventfilter
    InteractionEventManager* eventManager = new  InteractionEventManager(this);
    m_window->installEventFilter(eventManager);

    m_bFirst = true;
	
    setBackgroundImage();
	
    this->setupDefaultLight();

}

// ---------------------------------------------------------
// --- Destructor
// ---------------------------------------------------------
RHIViewer::~RHIViewer()
{
}

void RHIViewer::setupMeshes()
{
    // custom get() function to get special BaseObject
    // without BaseObject inheritance
    //m_qt3dObjects.clear();
    //using BaseObject = sofa::core::objectmodel::BaseObject;
    groot->get<RHIModel,
            helper::vector<RHIModel::SPtr> >
            (&m_rhiModels, sofa::core::objectmodel::BaseContext::SearchRoot);

    for(auto rhiModel : m_rhiModels)
    {
        rhiModel->initRHI(m_rhi, m_rpDesc);
    }

}

void RHIViewer::setupDefaultLight()
{
    // Light is located at the same place as the camera
//    auto defaultLightEntity = new Qt3DCore::QEntity(m_rootEntity);
////    auto defaultLight = new Qt3DRender::QDirectionalLight();
//    auto defaultLight = new Qt3DRender::QPointLight();
//    auto defaultLightTransform = new Qt3DCore::QTransform();
//    defaultLight->setIntensity(1.2f);
//
////    defaultLightEntity->addComponent(m_defaultCamera->transform());//not shareable
//    defaultLightEntity->addComponent(defaultLightTransform);
//    defaultLightEntity->addComponent(defaultLight);
//
//    //howto link two transforms ?
//    //QuickFix: use signal and stuff
//    connect(m_defaultCamera, &QCamera::positionChanged,
//           [this, defaultLightTransform]()
//    {
//        defaultLightTransform->setTranslation(m_defaultCamera->position());
//    });

}

void RHIViewer::setupBoundingBox()
{
    const sofa::defaulttype::BoundingBox& bbox = groot->f_bbox.getValue();
    const Vector3& minBBox = bbox.minBBox();
    const Vector3& maxBBox = bbox.maxBBox();

    //if(m_bboxTransform == nullptr)
    //{
    //    double r = (maxBBox- minBBox).norm() / 500.0f;

    //    //Bounding box node
    //    m_bboxEntity = new Qt3DCore::QEntity(m_rootEntity);
    //    m_bboxTransform = new Qt3DCore::QTransform();
    //    m_bboxTransform->setScale(1.0f);
    //    m_bboxEntity->addComponent(m_bboxTransform);

    //    auto bboxMaterial = new Qt3DExtras::QPhongMaterial();
    //    bboxMaterial->setDiffuse(QColor(QRgb(0x005FFF)));

    //    //Bounding box is composed of 12 lines (cylinders)
    //    for(int i=0 ; i<12 ;i++)
    //    {
    //        auto cylinderEntity = new Qt3DCore::QEntity(m_bboxEntity);
    //        auto cylinderMesh = new Qt3DExtras::QCylinderMesh();
    //        cylinderMesh->setRadius(r);
    //        cylinderMesh->setLength(1.0f);
    //        cylinderMesh->setRings(100);
    //        cylinderMesh->setSlices(20);
    //        m_bboxCylinderTransform[i] = new Qt3DCore::QTransform();
    //        cylinderEntity->addComponent(cylinderMesh);
    //        cylinderEntity->addComponent(m_bboxCylinderTransform[i]);
    //        cylinderEntity->addComponent(bboxMaterial);
    //    }

    //    //front face XY
    //    m_bboxCylinderTransform[0]->setTranslation(QVector3D(-0.5f, 0.0f, 0.5f));
    //    m_bboxCylinderTransform[1]->setRotationZ(90.0f);
    //    m_bboxCylinderTransform[1]->setTranslation(QVector3D(0.0f, -0.5f, 0.5f));
    //    m_bboxCylinderTransform[2]->setTranslation(QVector3D(0.5f, 0.0f, 0.5f));
    //    m_bboxCylinderTransform[3]->setRotationZ(90.0f);
    //    m_bboxCylinderTransform[3]->setTranslation(QVector3D(0.0f, 0.5f, 0.5f));

    //    //back face XY
    //    m_bboxCylinderTransform[4]->setTranslation(QVector3D(-0.5f, 0.0f, -0.5f));
    //    m_bboxCylinderTransform[5]->setRotationZ(90.0f);
    //    m_bboxCylinderTransform[5]->setTranslation(QVector3D(0.0f, -0.5f, -0.5f));
    //    m_bboxCylinderTransform[6]->setTranslation(QVector3D(0.5f, 0.0f, -0.5f));
    //    m_bboxCylinderTransform[7]->setRotationZ(90.0f);
    //    m_bboxCylinderTransform[7]->setTranslation(QVector3D(0.0f, 0.5f, -0.5f));

    //    //sides
    //    m_bboxCylinderTransform[8]->setRotationX(90.0f);
    //    m_bboxCylinderTransform[8]->setTranslation(QVector3D(-0.5f, 0.5f, 0.0f));
    //    m_bboxCylinderTransform[9]->setRotationX(90.0f);
    //    m_bboxCylinderTransform[9]->setTranslation(QVector3D(0.5f, 0.5f, 0.0f));
    //    m_bboxCylinderTransform[10]->setRotationX(90.0f);
    //    m_bboxCylinderTransform[10]->setTranslation(QVector3D(0.5f, -0.5f, 0.0f));
    //    m_bboxCylinderTransform[11]->setRotationX(90.0f);
    //    m_bboxCylinderTransform[11]->setTranslation(QVector3D(-0.5f, -0.5f, 0.0f));
    //}

    //if(m_bShowAxis)
    //{
    //    m_bboxEntity->setEnabled(true);

    //    const Vector3 sizeBBox = maxBBox - minBBox;
    //    const Vector3 halfSizeBBox = sizeBBox*0.5;
    //    QVector3D center {
    //        float(halfSizeBBox[0] + minBBox[0]),
    //        float(halfSizeBBox[1] + minBBox[1]),
    //        float(halfSizeBBox[2] + minBBox[2])
    //    };
    //    QVector3D scaleHeight {
    //        1.0f,
    //        float(sizeBBox[1]),
    //        1.0f
    //    };
    //    QVector3D scaleWidth {
    //        1.0f,
    //        float(sizeBBox[0]),
    //        1.0f
    //    };
    //    QVector3D scaleDepth {
    //        1.0f,
    //        float(sizeBBox[2]),
    //        1.0f
    //    };

    ////    // set to scene center
    //    m_bboxTransform->setTranslation(center);

    //    // move cylinders to the new dims (no scale3d possible otherwise it will deform the cylinders)
    //    m_bboxCylinderTransform[0]->setTranslation(QVector3D(-halfSizeBBox[0], 0.0f, halfSizeBBox[2]));
    //    m_bboxCylinderTransform[0]->setScale3D(scaleHeight);
    //    m_bboxCylinderTransform[1]->setTranslation(QVector3D(0.0f, -halfSizeBBox[1], halfSizeBBox[2]));
    //    m_bboxCylinderTransform[1]->setScale3D(scaleWidth);
    //    m_bboxCylinderTransform[2]->setTranslation(QVector3D(halfSizeBBox[0], 0.0f, halfSizeBBox[2]));
    //    m_bboxCylinderTransform[2]->setScale3D(scaleHeight);
    //    m_bboxCylinderTransform[3]->setTranslation(QVector3D(0.0f, halfSizeBBox[1], halfSizeBBox[2]));
    //    m_bboxCylinderTransform[3]->setScale3D(scaleWidth);

    //    m_bboxCylinderTransform[4]->setTranslation(QVector3D(-halfSizeBBox[0], 0.0f, -halfSizeBBox[2]));
    //    m_bboxCylinderTransform[4]->setScale3D(scaleHeight);
    //    m_bboxCylinderTransform[5]->setTranslation(QVector3D(0.0f, -halfSizeBBox[1], -halfSizeBBox[2]));
    //    m_bboxCylinderTransform[5]->setScale3D(scaleWidth);
    //    m_bboxCylinderTransform[6]->setTranslation(QVector3D(halfSizeBBox[0], 0.0f, -halfSizeBBox[2]));
    //    m_bboxCylinderTransform[6]->setScale3D(scaleHeight);
    //    m_bboxCylinderTransform[7]->setTranslation(QVector3D(0.0f, halfSizeBBox[1], -halfSizeBBox[2]));
    //    m_bboxCylinderTransform[7]->setScale3D(scaleWidth);

    //    m_bboxCylinderTransform[8]->setTranslation(QVector3D(-halfSizeBBox[0], halfSizeBBox[1], 0.0f));
    //    m_bboxCylinderTransform[8]->setScale3D(scaleDepth);
    //    m_bboxCylinderTransform[9]->setTranslation(QVector3D(halfSizeBBox[0], halfSizeBBox[1], 0.0f));
    //    m_bboxCylinderTransform[9]->setScale3D(scaleDepth);
    //    m_bboxCylinderTransform[10]->setTranslation(QVector3D(halfSizeBBox[0], -halfSizeBBox[1], 0.0f));
    //    m_bboxCylinderTransform[10]->setScale3D(scaleDepth);
    //    m_bboxCylinderTransform[11]->setTranslation(QVector3D(-halfSizeBBox[0], -halfSizeBBox[1], 0.0f));
    //    m_bboxCylinderTransform[11]->setScale3D(scaleDepth);
    //}
    //else
    //{
    //    m_bboxEntity->setEnabled(false);
    //}


}


void RHIViewer::setupFrameAxis()
{
    const sofa::defaulttype::BoundingBox& bbox = groot->f_bbox.getValue();
    const Vector3& minBBox = bbox.minBBox();
    const Vector3& maxBBox = bbox.maxBBox();

    //if(m_frameAxisEntity == nullptr)
    //{
    //    //Bounding box node
    //    m_frameAxisEntity = new Qt3DCore::QEntity(m_rootEntity);
    //    m_frameAxisTransform = new Qt3DCore::QTransform();
    //    m_frameAxisTransform->setScale(1.0f);
    //    m_frameAxisEntity->addComponent(m_frameAxisTransform);


    //    double r = (maxBBox- minBBox).norm() / 250.0f;

    //    // X Axis
    //    auto xAxisEntity = new Qt3DCore::QEntity(m_frameAxisEntity);
    //    auto xAxisMesh = new Qt3DExtras::QCylinderMesh();
    //    xAxisMesh->setRadius(r);
    //    xAxisMesh->setLength(1.0f);
    //    xAxisMesh->setRings(100);
    //    xAxisMesh->setSlices(20);

    //    auto xAxisMaterial = new Qt3DExtras::QDiffuseSpecularMaterial();
    //    xAxisMaterial->setDiffuse(QColor(QRgb(0xFF0000)));

    //    m_xAxisTransform = new Qt3DCore::QTransform();
    //    m_xAxisTransform->setTranslation(QVector3D(0.0f, 0.5f, 0.0f));
    //    m_xAxisTransform->setRotationZ(90.0f);
    //    xAxisEntity->addComponent(xAxisMesh);
    //    xAxisEntity->addComponent(m_xAxisTransform);
    //    xAxisEntity->addComponent(xAxisMaterial);

    //    // Y Axis
    //    auto yAxisEntity = new Qt3DCore::QEntity(m_frameAxisEntity);
    //    auto yAxisMesh = new Qt3DExtras::QCylinderMesh();
    //    yAxisMesh->setRadius(r);
    //    yAxisMesh->setLength(1.0f);
    //    yAxisMesh->setRings(100);
    //    yAxisMesh->setSlices(20);

    //    auto yAxisMaterial = new Qt3DExtras::QDiffuseSpecularMaterial();
    //    yAxisMaterial->setDiffuse(QColor(QRgb(0x00FF00)));

    //    m_yAxisTransform = new Qt3DCore::QTransform();
    //    m_yAxisTransform->setTranslation(QVector3D(0.0f, 0.5f, 0.0f));
    //    yAxisEntity->addComponent(yAxisMesh);
    //    yAxisEntity->addComponent(m_yAxisTransform);
    //    yAxisEntity->addComponent(yAxisMaterial);

    //    // Z Axis
    //    auto zAxisEntity = new Qt3DCore::QEntity(m_frameAxisEntity);
    //    auto zAxisMesh = new Qt3DExtras::QCylinderMesh();
    //    zAxisMesh->setRadius(r);
    //    zAxisMesh->setLength(1.0f);
    //    zAxisMesh->setRings(100);
    //    zAxisMesh->setSlices(20);

    //    auto zAxisMaterial = new Qt3DExtras::QDiffuseSpecularMaterial();
    //    zAxisMaterial->setDiffuse(QColor(QRgb(0x0000FF)));

    //    m_zAxisTransform = new Qt3DCore::QTransform();
    //    m_zAxisTransform->setTranslation(QVector3D(0.0f, 0.5f, 0.0f));
    //    m_zAxisTransform->setRotationX(90.0f);
    //    zAxisEntity->addComponent(zAxisMesh);
    //    zAxisEntity->addComponent(m_zAxisTransform);
    //    zAxisEntity->addComponent(zAxisMaterial);

    //    // always in front
    //    //auto depthTest = new Qt3DRender::QDepthTest();
    //    //depthTest->setDepthFunction(depthTest->Always);
    //    //xAxisMaterial->effect()->techniques()[0]->renderPasses()[0]->addRenderState(depthTest);
    //    //yAxisMaterial->effect()->techniques()[0]->renderPasses()[0]->addRenderState(depthTest);
    //    //zAxisMaterial->effect()->techniques()[0]->renderPasses()[0]->addRenderState(depthTest);

    //}

    //if(m_bShowAxis)
    //{
    //    m_frameAxisEntity->setEnabled(true);

    //    const Vector3 sizeBBox = maxBBox - minBBox;
    //    float maxSize = std::max(sizeBBox[0],std::max(sizeBBox[1],sizeBBox[2]));
    //    QVector3D scaleAxis {
    //        1.0f,
    //        float(maxSize * 0.5f), //50% of the biggest size of the bbox
    //        1.0f
    //    };

    //    m_xAxisTransform->setTranslation(QVector3D(maxSize*0.25f, 0.0f, 0.0f));
    //    m_xAxisTransform->setScale3D(scaleAxis);
    //    m_yAxisTransform->setTranslation(QVector3D(0.0f,maxSize*0.25f, 0.0f));
    //    m_yAxisTransform->setScale3D(scaleAxis);
    //    m_zAxisTransform->setTranslation(QVector3D(0.0f, 0.0f, maxSize*0.25f));
    //    m_zAxisTransform->setScale3D(scaleAxis);
    //}
    //else
    //{
    //    m_frameAxisEntity->setEnabled(false);
    //}

}

void RHIViewer::setupLogo()
{
//    if(!m_logoMaterial || !m_texture2D)
//        return;
//
//    if(m_logoEntity == nullptr)
//    {
//        float maxSide = std::max(m_logoSize.width(), m_logoSize.height());
//        float w = m_logoSize.width() / maxSide;
//        float h = m_logoSize.height() / maxSide;
//
//        m_logoEntity = new Qt3DCore::QEntity(m_rootEntity);
//        auto logoMesh = new Qt3DExtras::QPlaneMesh();
//        logoMesh->setWidth(w * 1.5f);
//        logoMesh->setHeight(h * 1.5f);
//        m_logoTransform = new Qt3DCore::QTransform();
//        m_logoTransform->setRotationX(90.0f);
////        m_logoTransform->setRotation(QQuaternion::fromEulerAngles(-90.0f, 0.0f, 0.0f)
////                                     * QQuaternion::fromEulerAngles(0.0f, 0.0f, 180.0f));
//
//        // material is set in setBackgroundImage()
//
//        m_logoEntity->addComponent(logoMesh);
//        m_logoEntity->addComponent(m_logoTransform);
//        m_logoEntity->addComponent(m_logoMaterial);
//        m_logoEntity->addComponent(m_backgroundLayer);
//    }
//
//    if(_background == 0)
//    {
//        m_clearBuffers->setClearColor(QColor(QRgb(0x000000)));
//        m_logoEntity->setEnabled(true);
//    }
//    else
//    {
//        m_logoEntity->setEnabled(false);
//
//        if(_background == 1)
//        {
//            m_clearBuffers->setClearColor(QColor(QRgb(0x000000)));
//        }
//        else
//        {
//            m_clearBuffers->setClearColor(QColor(backgroundColour[0]*255, backgroundColour[1]*255, backgroundColour[2]*255));
//        }
//    }
}

void RHIViewer::drawColourPicking(sofa::gui::ColourPickingVisitor::ColourCode code)
{
//    // Define background color
//    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//    glMatrixMode(GL_PROJECTION);
//    glPushMatrix();
//    glLoadIdentity();
//    glMultMatrixd(lastProjectionMatrix);
//    glMatrixMode(GL_MODELVIEW);


//    ColourPickingVisitor cpv(sofa::core::visual::VisualParams::defaultInstance(), code);
//    cpv.execute( groot.get() );

//    glMatrixMode(GL_PROJECTION);
//    glPopMatrix();
//    glMatrixMode(GL_MODELVIEW);
//    glPopMatrix();
    //TODO
}

// ----------------------------------------
// --- Handle events (mouse, keyboard, ...)
// ----------------------------------------


void RHIViewer::keyPressEvent(QKeyEvent * e)
{
    if (isControlPressed()) // pass event to the scene data structure
    {
        if (groot)
        {
            sofa::core::objectmodel::KeypressedEvent keyEvent(e->key());
            groot->propagateEvent(core::ExecParams::defaultInstance(), &keyEvent);
        }
    }
    else
    {
        SofaViewer::keyPressEvent(e);
    }
}

void RHIViewer::keyReleaseEvent(QKeyEvent * e)
{
//    std::cerr<<"RHIViewer::keyReleaseEvent, key = "<<e->key()<<" "<<std::endl;
    SofaViewer::keyReleaseEvent(e);
}

void RHIViewer::wheelEvent(QWheelEvent* e)
{
    //std::cerr<<"RHIViewer::wheelEvent"<<std::endl;
    SofaViewer::wheelEvent(e);
}

void RHIViewer::mousePressEvent(QMouseEvent * e)
{
//    std::cerr<<"RHIViewer::mousePressEvent"<<std::endl;
    mouseEvent(e);

    SofaViewer::mousePressEvent(e);
}

void RHIViewer::mouseReleaseEvent(QMouseEvent * e)
{
//    std::cerr<<"RHIViewer::mouseReleaseEvent"<<std::endl;
    mouseEvent(e);

    SofaViewer::mouseReleaseEvent(e);

}

void RHIViewer::mouseMoveEvent(QMouseEvent * e)
{
    //if the mouse move is not "interactive", give the event to the camera
    if(!mouseEvent(e))
    {
        SofaViewer::mouseMoveEvent(e);
    }
    else
    {
    }
}

// ---------------------- Here are the mouse controls for the scene  ----------------------
bool RHIViewer::mouseEvent(QMouseEvent * e)
{
    bool isInteractive = false;
    int eventX = e->x();
    int eventY = e->y();

    if (e->modifiers() & Qt::ShiftModifier)
    {
        isInteractive = true;
        SofaViewer::mouseEvent(e);
    }


    return isInteractive;
}

void RHIViewer::moveRayPickInteractor(int eventX, int eventY)
{
    //const QRect viewport(QPoint(0,0), m_container->size());
    //float invertedEventY =  m_container->size().height() - eventY;

    ////Warning: 0 is bottom for opengl wheras is top for qt
    //QVector3D nearPos = QVector3D(eventX, invertedEventY, 0.0f);
    //nearPos = nearPos.unproject(m_defaultCamera->viewMatrix(), m_defaultCamera->projectionMatrix(), viewport);
    //QVector3D farPos = QVector3D(eventX, invertedEventY, 1.0f);
    //farPos = farPos.unproject(m_defaultCamera->viewMatrix(), m_defaultCamera->projectionMatrix(), viewport);
    //QVector3D qDir = (farPos - nearPos).normalized();

    //sofa::defaulttype::Vec3d position{nearPos.x(), nearPos.y(), nearPos.z()};
    //sofa::defaulttype::Vec3d direction{qDir.x(), qDir.y(), qDir.z()};

    //pick->updateRay(position, direction);

}

// -------------------------------------------------------------------
// ---
// -------------------------------------------------------------------
void RHIViewer::resetView()
{
    bool fileRead = false;

    if (!sceneFileName.empty())
    {
        std::string viewFileName = sceneFileName + "." + VIEW_FILE_EXTENSION;
        fileRead = currentCamera->importParametersFromFile(viewFileName);
    }

    //if there is no .view file , look at the center of the scene bounding box
    // and with a Up vector in the same axis as the gravity
    if (!fileRead)
    {
        newView();
    }

}

void RHIViewer::newView()
{
    SofaViewer::newView();
}

void RHIViewer::getView(Vector3& pos, Quat& ori) const
{
    SofaViewer::getView(pos, ori);
}

void RHIViewer::setView(const Vector3& pos, const Quat &ori)
{
    SofaViewer::setView(pos, ori);
}

void RHIViewer::saveView()
{
    if (!sceneFileName.empty())
    {
        std::string viewFileName = sceneFileName + "." + VIEW_FILE_EXTENSION;
        if (currentCamera->exportParametersInFile(viewFileName))
            std::cout << "View parameters saved in " << viewFileName << std::endl;
        else
            std::cout << "Error while saving view parameters in " << viewFileName << std::endl;
    }
}

void RHIViewer::setSizeW(int size)
{
//    resizeGL(size, _H);
//    update();
}

void RHIViewer::setSizeH(int size)
{
//    resizeGL(_W, size);
//    update();
}


int RHIViewer::getWidth()
{
    return this->width();
}

int RHIViewer::getHeight()
{
    return this->height();
}

QString RHIViewer::helpString() const
{
    static QString
    text(
        "<H1>RHIViewer</H1><hr>\
<ul>\
<li><b>Mouse</b>: TO NAVIGATE<br></li>\
<li><b>Shift & Left Button</b>: TO PICK OBJECTS<br></li>\
<li><b>B</b>: TO CHANGE THE BACKGROUND<br></li>\
<li><b>C</b>: TO SWITCH INTERACTION MODE: press the KEY C.<br>\
Allow or not the navigation with the mouse.<br></li>\
<li><b>O</b>: TO EXPORT TO .OBJ<br>\
The generated files scene-time.obj and scene-time.mtl are saved in the running project directory<br></li>\
<li><b>P</b>: TO SAVE A SEQUENCE OF OBJ<br>\
Each time the frame is updated an obj is exported<br></li>\
<li><b>R</b>: TO DRAW THE SCENE AXIS<br></li>\
<li><b>S</b>: TO SAVE A SCREENSHOT<br>\
The captured images are saved in the running project directory under the name format capturexxxx.bmp<br></li>\
<li><b>T</b>: TO CHANGE BETWEEN A PERSPECTIVE OR AN ORTHOGRAPHIC CAMERA<br></li>\
<li><b>V</b>: TO SAVE A VIDEO<br>\
Each time the frame is updated a screenshot is saved<br></li>\
<li><b>Esc</b>: TO QUIT ::sofa:: <br></li></ul>");
    return text;
}


void RHIViewer::screenshot(const std::string& filename, int compression_level)
{
    m_backend->screenshot(filename, compression_level);
}

void RHIViewer::wait()
{
    // called each timestep
    _waitForRender = true;

}

void RHIViewer::resizeView(QSize size)
{
    m_container->resize(size);
    //m_defaultCamera->lens()->setPerspectiveProjection(45.f, float(size.width()) / float(size.height()), 0.01f, 1000.f);

}

void RHIViewer::resizeEvent ( QResizeEvent * /*event*/ )
{
    resizeView(this->size());
}

void RHIViewer::paintEvent ( QPaintEvent * /*event*/ )
{
	if (!groot)
		return;

    //update VisualParams for the following draw() called by SofaObjects
    updateVisualParameters();


    //todo: do a real init of the scene
    if(m_bFirst)
    {
        std::cout << "first update()" << std::endl;

        setupMeshes();
        //drawScene();

        resetView();
        m_bFirst = false;
    }
    else
    {
        drawScene();
    }

}

void RHIViewer::updateVisualParameters()
{
	if (!groot)
		return;

    //TODO: compute znear zfar
    m_vparams->zNear() = currentCamera->getZNear();
    m_vparams->zFar() = currentCamera->getZFar();

    sofa::core::visual::VisualParams::Viewport vp{0, 0, m_container->width(), m_container->height() };
    m_vparams->viewport() = vp;
    double projectionMatrix[16];
    double modelviewMatrix[16];

    currentCamera->getProjectionMatrix(projectionMatrix);
    currentCamera->getModelViewMatrix(modelviewMatrix);

    m_vparams->setProjectionMatrix(projectionMatrix);
    m_vparams->setModelViewMatrix(modelviewMatrix);

    m_vparams->sceneBBox() = groot->f_bbox.getValue();
}

void RHIViewer::drawScene(void)
{
    if (!groot) return;
    m_swapChain->buildOrResize();
    m_rhi->beginFrame(m_swapChain); // == QRhi::FrameOpSuccess;
    QRhiCommandBuffer* cb = m_swapChain->currentFrameCommandBuffer();
    QRhiRenderTarget* rt = m_swapChain->currentFrameRenderTarget();
    QSize outputSize = m_swapChain->currentPixelSize();
    QRhiResourceUpdateBatch* updates;
    updates = (m_rhi->nextResourceUpdateBatch());

    QRhiViewport viewport(0, 0, float(outputSize.width()), float(outputSize.height()));// , currentCamera->getZNear(), currentCamera->getZFar());

    for (auto rhiModel : m_rhiModels)
    {
        rhiModel->addResourceUpdate(updates);
    }

    cb->beginPass(rt, Qt::gray, { 1.0f, 0 }, updates);

    for (auto rhiModel : m_rhiModels)
    {
        rhiModel->updateRHI(cb, viewport);
    }

    cb->endPass();

    m_rhi->endFrame(m_swapChain);

    _waitForRender = false;
    if (!captureTimer.isActive())
        SofaViewer::captureEvent();
}

bool RHIViewer::load()
{

    bool res = SofaViewer::load();

    //if (res)
    //{
    //    sofa::core::visual::VisualLoop::SPtr vloop;
    //    groot->get(vloop);
    //    //wont happen because Simulation is setting the default one before :(
    //    if(!vloop)
    //    {
    //        sofa::qt3d::Qt3DVisualManagerLoop::SPtr qt3dloop = sofa::core::objectmodel::New<Qt3DVisualManagerLoop>();
    //        groot->addObject(qt3dloop);
    //    }
    //    else
    //    {
    //        if(dynamic_cast<sofa::qt3d::Qt3DVisualManagerLoop*>(vloop.get()) == nullptr)
    //        {
    //            msg_warning("Qt3dViewer") << "This viewer needs a Qt3DVisualManagerLoop.";
    //            msg_warning("Qt3dViewer") << "Fallback: the viewer will replace the existing one.";

    //            groot->removeObject(vloop);
    //            sofa::qt3d::Qt3DVisualManagerLoop::SPtr qt3dloop = sofa::core::objectmodel::New<Qt3DVisualManagerLoop>();
    //            groot->addObject(qt3dloop);
    //            qt3dloop->init();
    //        }
    //    }
    //}

    return res;
}

bool RHIViewer::unload()
{
    return SofaViewer::unload();
}

void RHIViewer::update()
{
}

void RHIViewer::setBackgroundImage(std::string imageFileName)
{
    _background = 0;
    //if( sofa::helper::system::DataRepository.findFile(imageFileName) )
    //{
    //    backgroundImageFile = sofa::helper::system::DataRepository.getFile(imageFileName);

    //    m_logoMaterial = new Qt3DExtras::QTextureMaterial();
    //    m_textureImage = new Qt3DRender::QTextureImage();
    //    m_textureImage->setSource(QUrl::fromLocalFile(backgroundImageFile.c_str()));
    //    m_textureImage->setMirrored(false);
    //    m_texture2D = new Qt3DRender::QTexture2D();
    //    m_texture2D->addTextureImage(m_textureImage);
    //    m_logoMaterial->setTexture(m_texture2D);

    //    // store image size from a QImage, previous operation lose picture info
    //    QImage* dummyImage = new QImage(backgroundImageFile.c_str());
    //    m_logoSize = dummyImage->size();

    //}

}


bool InteractionEventManager::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        //        qDebug("Ate key press %d", keyEvent->key());
        m_viewer->keyPressEvent(keyEvent);
        return true;
    }
    else if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        m_viewer->keyReleaseEvent(keyEvent);
        return true;
    }
    else if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* keyEvent = static_cast<QMouseEvent*>(event);
        //        qDebug("Ate mouse press %d");
        m_viewer->mousePressEvent(keyEvent);
        return true;
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
        QMouseEvent* keyEvent = static_cast<QMouseEvent*>(event);
        m_viewer->mouseReleaseEvent(keyEvent);
        return true;
    }
    else if (event->type() == QEvent::Wheel)
    {
        QWheelEvent* keyEvent = static_cast<QWheelEvent*>(event);
        m_viewer->wheelEvent(keyEvent);
        return true;
    }
    else if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent* keyEvent = static_cast<QMouseEvent*>(event);
        m_viewer->mouseMoveEvent(keyEvent);
        return true;
    }
    else
    {
        // standard event processing
        return QObject::eventFilter(obj, event);
    }
}

helper::SofaViewerCreator< RHIViewer> RHIViewer_class("rhi",false);

int RHIGUIClass = sofa::gui::GUIManager::RegisterGUI ( "rhi", &sofa::gui::qt::RealGUI::CreateGUI, NULL, 3 );


} // namespace sofa::rhi
