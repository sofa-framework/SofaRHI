#include <SofaRHI/gui/RHIOffscreenViewer.h>

#include <SofaRHI/RHIVisualManagerLoop.h>

#include <sofa/helper/system/FileRepository.h>
#include <sofa/helper/system/FileSystem.h>
#include <sofa/helper/system/PluginManager.h>
#include <sofa/helper/AdvancedTimer.h>
#include <sofa/core/ObjectFactory.h>
#include <sofa/simulation/Simulation.h>
#include <sofa/simulation/UpdateContextVisitor.h>

#include <sofa/gui/GUIManager.h>

#include <QApplication>
#include <QFileInfo>
#include <QtGui/private/qshader_p.h>
#if QT_CONFIG(opengl)
# include <QOpenGLContext>
# include <QtGui/private/qrhigles2_p.h>
#endif

#if QT_CONFIG(vulkan) && Vulkan_FOUND
# define VIEWER_USE_VULKAN 1
# include <QVulkanInstance>
# include <QtGui/private/qrhivulkan_p.h>
#endif

#ifdef Q_OS_WIN
#include <QtGui/private/qrhid3d11_p.h>
#endif

#ifdef Q_OS_DARWIN
# include <QtGui/private/qrhimetal_p.h>
#endif

Q_DECLARE_METATYPE(QRhi::Implementation)
Q_DECLARE_METATYPE(QRhiInitParams*)



namespace sofa::rhi
{

const int RHIOffscreenViewer::DEFAULT_NUMBER_OF_ITERATIONS = 1000;
int RHIOffscreenViewer::s_nbIterations = RHIOffscreenViewer::DEFAULT_NUMBER_OF_ITERATIONS;

enum class GraphicsAPI
{
    OpenGL,
    Vulkan,
    D3D11,
    Metal
};

static std::map<std::string, std::pair<QRhi::Implementation, std::string> > s_mapGraphicsAPI
{
    { "ogl", { QRhi::OpenGLES2, "OpenGL Core" } },
    { "vlk", { QRhi::Vulkan, "Vulkan"} },
    { "d3d", { QRhi::D3D11, "Direct3D 11"} },
    { "mtl", { QRhi::Metal, "Metal"} }
};

std::string RHIOffscreenViewer::s_keyGgraphicsAPI = {"ogl"};

/// Takes the same view file format as qglviewer
const std::string RHIOffscreenViewer::VIEW_FILE_EXTENSION = "rhiviewer.view";


int RHIOffscreenViewer::RegisterGUIParameters(sofa::helper::ArgumentParser* argumentParser)
{
    static std::vector<std::string> supportedAPIs;
    
#ifdef Q_OS_WIN
    static std::string defaultStr = "ogl";

    supportedAPIs.emplace_back(defaultStr);
    supportedAPIs.emplace_back("d3d");
#endif // Q_OS_WIN
#ifdef Q_OS_DARWIN //ios or mac
    static std::string defaultStr = "ogl";

    supportedAPIs.emplace_back(defaultStr);
    supportedAPIs.emplace_back("mtl");
#endif // Q_OS_DARWIN
#if defined(Q_OS_LINUX) || defined(Q_OS_ANDROID)
    static std::string defaultStr = "ogl";

    supportedAPIs.emplace_back(defaultStr);
#endif // Q_OS_LINUX || Q_OS_ANDROID
#if VIEWER_USE_VULKAN
    supportedAPIs.emplace_back("vlk");
#endif // VIEWER_USE_VULKAN

    std::ostringstream displayChoice;
    displayChoice << "select graphics API between: " ;
    for (const auto& supportedAPI : supportedAPIs)
    {
        displayChoice << supportedAPI << " | ";
    }
    argumentParser->addArgument(
        boost::program_options::value<std::string>(&s_keyGgraphicsAPI)
        ->default_value(defaultStr)
        ->notifier([](const std::string value) {
            if (std::find(supportedAPIs.begin(), supportedAPIs.end(), value) != supportedAPIs.end())
            {
                msg_error("RHIViewer") << "Unsupported graphics API " << value << ", falling back to " << defaultStr << " .";
            }
        }),
        "api", displayChoice.str()
    );

    argumentParser->addArgument(
        boost::program_options::value<std::string>()
        ->notifier(setNumIterations),
        "nbIter,n",
        "(only batch) Number of iterations of the simulation"
    );

    return 0;
}

RHIOffscreenViewer::RHIOffscreenViewer()
{    
    //s_keyGgraphicsAPI = "ogl";
    s_keyGgraphicsAPI = "d3d";

    const QRhi::Implementation graphicsAPI = s_mapGraphicsAPI[s_keyGgraphicsAPI].first;

    //// RHI Setup
    QRhiInitParams* initParams = nullptr;

    if(graphicsAPI == QRhi::OpenGLES2)
    {
        QScopedPointer<QOffscreenSurface> offscreenSurface;
        offscreenSurface.reset(QRhiGles2InitParams::newFallbackSurface());
        QRhiGles2InitParams oglInitParams;
        oglInitParams.fallbackSurface = offscreenSurface.data();
        m_rhi.reset(QRhi::create(graphicsAPI, &oglInitParams));
        msg_info("RHIViewer") << "Will use OpenGLES2";
    }
#ifdef Q_OS_WIN
    if (graphicsAPI == QRhi::D3D11)
    {
        QRhiD3D11InitParams d3dInitParams;
        //d3dInitParams.enableDebugLayer = true;
        m_rhi.reset(QRhi::create(graphicsAPI, &d3dInitParams));
        msg_info("RHIViewer") << "Will use D3D11";
    }
#endif // Q_OS_WIN
#ifdef Q_OS_DARWIN
    if (graphicsAPI == QRhi::Metal)
    {
        m_window->setSurfaceType(QSurface::MetalSurface);
        QRhiMetalInitParams mtlInitParams;
        m_rhi.reset(QRhi::create(graphicsAPI, &mtlInitParams));
        msg_info("RHIViewer") << "Will use Metal";
    }
#endif // Q_OS_DARWIN
#if VIEWER_USE_VULKAN
    if (graphicsAPI == QRhi::Vulkan)
    {
        //// Vulkan setup.
        //QVulkanInstance inst;
        //inst.create();
        //m_window->setSurfaceType(QSurface::VulkanSurface);
        //QRhiVulkanInitParams vulkanInitParams;
        //vulkanInitParams.inst = m_window->vulkanInstance();
        //vulkanInitParams.window = m_window;
        //m_window->setVulkanInstance(&inst);
        //msg_info("RHIViewer") << "Will use Vulkan";
    }
#endif // VIEWER_USE_VULKAN

    if (!m_rhi)
    {
        msg_fatal("RHIViewer") << "Not implemented yet";
        qFatal("Failed to create RHI backend, quitting.");
        //exit
    }

    m_offscreenTexture = m_rhi->newTexture(QRhiTexture::RGBA8, QSize(1280, 720), 1, QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
    m_offscreenTexture->build();
    m_offscreenTextureRenderTarget = m_rhi->newTextureRenderTarget({ m_offscreenTexture });
    m_rpDesc.reset(m_offscreenTextureRenderTarget->newCompatibleRenderPassDescriptor());
    m_offscreenTextureRenderTarget->setRenderPassDescriptor(m_rpDesc.get());
    m_offscreenTextureRenderTarget->build();

    // as a parameter
    m_offscreenViewport.setViewport(0, 0, 1280, 720);

    m_vparams = core::visual::VisualParams::defaultInstance();
    m_drawTool = new DrawToolRHI(m_rhi, m_rpDesc);
    //m_rhi->addCleanupCallback(cleanupRHI);
    m_vparams->drawTool() = m_drawTool;


    sofa::core::ObjectFactory::ClassEntry::SPtr replaceOglModel;
    sofa::core::ObjectFactory::AddAlias("VisualModel", "RHIModel", true,
            &replaceOglModel);

    const std::string openglPlugin = "SofaOpenglVisual";
    auto& pluginManager = sofa::helper::system::PluginManager::getInstance();
    if (pluginManager.pluginIsLoaded(openglPlugin))
    {
        msg_warning("RHIViewer") << "SofaOpenGLVisual plugin has been loaded and is incompatible with SofaRHI";
        msg_warning("RHIViewer") << "SofaRHI will disable all its components and replace OglModel with RHIModel to prevent crashing.";

        std::vector<sofa::core::ObjectFactory::ClassEntry::SPtr> listComponents;
        sofa::core::ObjectFactory::getInstance()->getEntriesFromTarget(listComponents, openglPlugin);
        for (auto component : listComponents)
        {
            sofa::core::ObjectFactory::ClassEntry::SPtr entry;
            if(component->className != "OglModel")
            { 
                sofa::core::ObjectFactory::AddAlias(component->className, "DisabledObject", true,
                    &entry);
            }
            else
            {
                msg_warning("RHIViewer") << "All occurences of OglModel will be replaced by RHIModel";
                sofa::core::ObjectFactory::AddAlias("OglModel", "RHIModel", true,
                    &entry);
            }
        }
    }


    m_groot = nullptr;
}

// -------------------------------------------------------------------
// ---
// -------------------------------------------------------------------
void RHIOffscreenViewer::resetView()
{
    bool fileRead = false;

    if (!m_filename.empty())
    {
        std::string viewFileName = m_filename + "." + VIEW_FILE_EXTENSION;
        fileRead = m_currentCamera->importParametersFromFile(viewFileName);
    }

    //if there is no .view file , look at the center of the scene bounding box
    // and with a Up vector in the same axis as the gravity
    if (!fileRead)
    {
        m_currentCamera->setDefaultView(m_groot->getGravity());
    }

}


void RHIOffscreenViewer::updateVisualParameters()
{
	if (!m_groot)
		return;

    //TODO: compute znear zfar
    m_vparams->zNear() = m_currentCamera->getZNear();
    m_vparams->zFar() = m_currentCamera->getZFar();

    sofa::core::visual::VisualParams::Viewport vp{0, 0, int(m_offscreenViewport.viewport()[2]),  int(m_offscreenViewport.viewport()[3]) };
    m_vparams->viewport() = vp;
    double projectionMatrix[16];
    double modelviewMatrix[16];

    m_currentCamera->getProjectionMatrix(projectionMatrix);
    m_currentCamera->getProjectionMatrix(projectionMatrix);
    m_currentCamera->getModelViewMatrix(modelviewMatrix);

    m_vparams->setProjectionMatrix(projectionMatrix);
    m_vparams->setModelViewMatrix(modelviewMatrix);

    m_vparams->sceneBBox() = m_groot->f_bbox.getValue();
}

void RHIOffscreenViewer::drawScene()
{
    using sofa::simulation::getSimulation;

    if (!m_groot) return;

    QRhiCommandBuffer* cb;

    if (m_rhi->beginOffscreenFrame(&cb) != QRhi::FrameOpSuccess)
        return;

    QRhiResourceUpdateBatch* updates;
    updates = (m_rhi->nextResourceUpdateBatch());

    m_drawTool->beginFrame(m_vparams, updates, cb, m_offscreenViewport);

#ifdef ENABLE_RHI_COMPUTE
    // Optional Compute Stage
    // test if compute shader is available blabla
    if (!m_bHasInitTexture)
    {
        m_rhiloop->initComputeCommandsStep(m_vparams);
    }

    m_rhiloop->updateComputeResourcesStep(m_vparams); // will call Visitor for updating RHI Compute resources for RHIComputeModels
    cb->beginComputePass(updates);
    m_rhiloop->updateComputeCommandsStep(m_vparams); // will call Visitor for updating RHI Compute commands for RHIComputeModels

    cb->endComputePass();
#endif // ENABLE_RHI_COMPUTE

    // Rendering Stage
    if (!m_bHasInitTexture) // "initTexture" is the super old function for initVisual
    {
        getSimulation()->initTextures(m_groot.get()); // will call Visitor for initializing RHI resources  for RHIModels (only)
        m_bHasInitTexture = true;
    }

    //getSimulation()->updateVisual(groot.get()); 
    m_rhiloop->updateRHIResourcesStep(m_vparams); // will call Visitor for updating RHI resources for RHIGraphicModels and Other BaseObjects

    cb->beginPass(m_offscreenTextureRenderTarget, Qt::gray, { 1.0f, 0 }, updates);
    
    getSimulation()->draw(m_vparams, m_groot.get()); // will call Visitor for updating RHI commands for RHIModels (only)

    m_drawTool->executeCommands(); // will execute commands for Other BaseObjects

    updates = (m_rhi->nextResourceUpdateBatch());
    QRhiReadbackDescription rb(m_offscreenTexture);
    QRhiReadbackResult rbResult;
    rbResult.completed = [this] { qDebug("  - readback %d completed", m_currentIterations); };
    updates->readBackTexture(rb, &rbResult);

    cb->endPass();

    m_drawTool->endFrame();

    m_rhi->endOffscreenFrame();

    // The data should be ready either because endOffscreenFrame() waits
    // for completion or because finish() did.
    if (!rbResult.data.isEmpty()) {
        const uchar* p = reinterpret_cast<const uchar*>(rbResult.data.constData());
        QImage image(p, rbResult.pixelSize.width(), rbResult.pixelSize.height(), QImage::Format_RGBA8888);
        QString fn = QString::asprintf("frame%d.png", s_nbIterations);
        fn = QFileInfo(fn).absoluteFilePath();
        qDebug("Saving into %s", qPrintable(fn));
        if (m_rhi->isYUpInFramebuffer())
            image.mirrored().save(fn);
        else
            image.save(fn);
    }
    else {
        qWarning("Readback failed!");
    }

}

void RHIOffscreenViewer::checkScene()
{
    sofa::core::visual::VisualLoop::SPtr vloop;
    m_groot->get(vloop);
    //wont happen because Simulation is setting the default one before :(
    if(!vloop)
    {
        m_rhiloop = sofa::core::objectmodel::New<RHIVisualManagerLoop>();
        m_groot->addObject(m_rhiloop);
    }
    else
    {
        if(dynamic_cast<sofa::rhi::RHIVisualManagerLoop*>(vloop.get()) == nullptr)
        {
            msg_warning("RHIViewer") << "This viewer needs a RHIVisualManagerLoop.";
            msg_warning("RHIViewer") << "Fallback: the viewer will replace the existing one.";

            m_groot->removeObject(vloop);
            m_rhiloop = sofa::core::objectmodel::New<RHIVisualManagerLoop>();
            m_groot->addObject(m_rhiloop);
            m_rhiloop->init();
        }
    }
}

int RHIOffscreenViewer::mainLoop()
{
    if (m_groot)
    {
        if (s_nbIterations != -1)
        {
            msg_info("RHIOffscreenViewer") << "Computing " << s_nbIterations << " iterations." << msgendl;
        }
        else
        {
            msg_info("RHIOffscreenViewer") << "Computing infinite iterations." << msgendl;
        }

        sofa::helper::AdvancedTimer::begin("Animate");
        sofa::simulation::getSimulation()->animate(m_groot.get());
        msg_info("RHIOffscreenViewer") << "Processing." << sofa::helper::AdvancedTimer::end("Animate", m_groot.get()) << msgendl;
        sofa::simulation::Visitor::ctime_t rtfreq = sofa::helper::system::thread::CTime::getRefTicksPerSec();
        sofa::simulation::Visitor::ctime_t tfreq = sofa::helper::system::thread::CTime::getTicksPerSec();
        sofa::simulation::Visitor::ctime_t rt = sofa::helper::system::thread::CTime::getRefTime();
        sofa::simulation::Visitor::ctime_t t = sofa::helper::system::thread::CTime::getFastTime();

        int i = 1; //one simulatin step is animated above  

        while (i <= s_nbIterations || s_nbIterations == -1)
        {
            if (i != s_nbIterations)
            {
                sofa::helper::ScopedAdvancedTimer("Animate");
                sofa::simulation::getSimulation()->animate(m_groot.get());

                drawScene();
            }

            if (i == s_nbIterations || (s_nbIterations == -1 && i % 1000 == 0))
            {
                t = sofa::helper::system::thread::CTime::getFastTime() - t;
                rt = sofa::helper::system::thread::CTime::getRefTime() - rt;

                msg_info("RHIOffscreenViewer") << i << " iterations done in " << ((double)t) / ((double)tfreq) << " s ( " << (((double)tfreq) * i) / ((double)t) << " FPS)." << msgendl;
                msg_info("RHIOffscreenViewer") << i << " iterations done in " << ((double)rt) / ((double)rtfreq) << " s ( " << (((double)rtfreq) * i) / ((double)rt) << " FPS)." << msgendl;

                if (s_nbIterations == -1) // Additional message for infinite iterations
                {
                    msg_info("RHIOffscreenViewer") << "Press Ctrl + C (linux)/ Command + period (mac) to stop " << msgendl;
                }
            }

            i++;
            m_currentIterations++;
        }

    }
    return 0;
}


void RHIOffscreenViewer::redraw()
{
}

int RHIOffscreenViewer::closeGUI()
{
    delete this;
    return 0;
}

void RHIOffscreenViewer::setScene(sofa::simulation::Node::SPtr groot, const char* filename, bool)
{
    this->m_groot = groot;
    this->m_filename = (filename ? filename : "");

    resetScene();
}

void RHIOffscreenViewer::resetScene()
{
    sofa::simulation::Node* root = currentSimulation();

    if (root)
    {
        root->setTime(0.);
        simulation::getSimulation()->reset(root);

        checkScene();

        sofa::simulation::UpdateSimulationContextVisitor(sofa::core::ExecParams::defaultInstance()).execute(root);
    }
}

sofa::simulation::Node* RHIOffscreenViewer::currentSimulation()
{
    return m_groot.get();
}

QApplication* RHIOffscreenViewer::s_qtApplication = nullptr;
RHIOffscreenViewer* RHIOffscreenViewer::s_gui = nullptr;

sofa::gui::BaseGUI* RHIOffscreenViewer::CreateGUI(const char* name, sofa::simulation::Node::SPtr groot, const char* filename)
{
    int* argc = new int;
    char** argv = new char* [2];
    *argc = 1;
    argv[0] = strdup(BaseGUI::GetProgramName());
    argv[1] = nullptr;
    s_qtApplication = new QApplication(*argc, argv);

    QCoreApplication::setOrganizationName("<F.R>");
    QCoreApplication::setOrganizationDomain("sofa");
    QCoreApplication::setApplicationName("runSofa");

    //force locale to Standard C
    //(must be done immediatly after the QApplication has been created)
    QLocale locale(QLocale::C);
    QLocale::setDefault(locale);

    // create interface
    s_gui = new RHIOffscreenViewer();
    if (groot)
    {
        s_gui->setScene(groot, filename);
    }

    return s_gui;
}

void RHIOffscreenViewer::setNumIterations(const std::string& nbIterInp)
{
    size_t inpLen = nbIterInp.length();

    if (nbIterInp == "infinite")
    {
        s_nbIterations = -1;
    }
    else if (inpLen)
    {
        s_nbIterations = std::stoi(nbIterInp);
    }
    else
    {
        s_nbIterations = DEFAULT_NUMBER_OF_ITERATIONS;
    }

}

int RHIOffscreenGUIClass = sofa::gui::GUIManager::RegisterGUI("rhi_offscreen", &RHIOffscreenViewer::CreateGUI, &RHIOffscreenViewer::RegisterGUIParameters, 4);


} // namespace sofa::rhi