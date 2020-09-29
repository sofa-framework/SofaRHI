#pragma once

#include <SofaRHI/DrawToolRHI.h>
#include <SofaRHI/gui/RHIBackend.h>
#include <SofaRHI/RHIVisualManagerLoop.h>

#include <sofa/gui/qt/viewer/SofaViewer.h>
#include <sofa/gui/ViewerFactory.h>

#include <sofa/defaulttype/Vec.h>
#include <sofa/defaulttype/Quat.h>
#include <sofa/helper/ArgumentParser.h>

#include <SofaSimulationCommon/xml/Element.h>

#include <QtGui/private/qrhi_p.h>
#include <QtGui/private/qrhinull_p.h>

namespace sofa::rhi::gui
{

class SOFA_SOFARHI_API RHIViewer
        : public QWidget
        , public sofa::gui::qt::viewer::SofaViewer
{
    Q_OBJECT

private:

    QTimer* timerAnimate;
    bool _waitForRender;

public:

    static const std::string VIEW_FILE_EXTENSION;


    static RHIViewer* create(RHIViewer*, sofa::gui::BaseViewerArgument& arg)
    {
        sofa::gui::BaseViewerArgument* pArg = &arg;
        sofa::gui::ViewerQtArgument* viewerArg = dynamic_cast<sofa::gui::ViewerQtArgument*>(pArg);
        return viewerArg ?
                new RHIViewer(viewerArg->getParentWidget(), viewerArg->getName().c_str(), viewerArg->getNbMSAASamples() ) :
                new RHIViewer(nullptr, pArg->getName().c_str(), pArg->getNbMSAASamples() )
                ;
    }

    static const char* viewerName()
    {
        return "RHI";
    }

    static const char* acceleratedName()
    {
        return "RHI";
    }

    /// Activate this class of viewer.
    /// This method is called before the viewer is actually created
    /// and can be used to register classes associated with in the the ObjectFactory.
    static int EnableViewer();

    /// Disable this class of viewer.
    /// This method is called after the viewer is destroyed
    /// and can be used to unregister classes associated with in the the ObjectFactory.
    static int DisableViewer();

    RHIViewer( QWidget* parent, const char* name="", const unsigned int nbMSAASamples = 1);
    ~RHIViewer() override;

    QWidget* getQWidget() override { return this; }

    bool ready() override {return !_waitForRender;}
    void wait() override;

    bool load() override;
    bool unload() override;

    static int RegisterGUIParameters(sofa::helper::ArgumentParser* argumentParser);

public slots:
    void resetView() override;
    virtual void saveView() override;
    virtual void setSizeW(int) override;
    virtual void setSizeH(int) override;
    virtual int getWidth() override;
    virtual int getHeight() override;

    virtual void getView(defaulttype::Vector3& pos, defaulttype::Quat& ori) const override;
    virtual void setView(const defaulttype::Vector3& pos, const defaulttype::Quat &ori) override ;
    virtual void newView() override ;
    virtual void captureEvent()  override { SofaViewer::captureEvent(); }
    virtual void drawColourPicking (sofa::gui::ColourPickingVisitor::ColourCode code) override ;
    virtual void fitNodeBBox(sofa::core::objectmodel::BaseNode * node )  override { SofaViewer::fitNodeBBox(node); }
    virtual void fitObjectBBox(sofa::core::objectmodel::BaseObject * obj)  override { SofaViewer::fitObjectBBox(obj); }

signals:
    void redrawn() override ;
    void resizeW( int ) override ;
    void resizeH( int ) override ;
    void quit();


protected:
    void setupRHI();
    
    void setupDefaultLight();
    void setupBoundingBox();
    void setupFrameAxis();
    void setupLogo();

    void setupMeshes();

    virtual void viewAll()  override {}
public:

    sofa::simulation::Node* getScene() override
    {
        return groot.get();
    }

    void moveRayPickInteractor(int eventX, int eventY) override ;

    QString helpString() const  override ;
    void screenshot(const std::string& filename, int compression_level = -1) override;
    void setBackgroundImage(std::string imageFileName = std::string("textures/SOFA_logo.bmp")) override;

private:
    virtual void drawScene() override;

    bool m_notExposed { false };
    bool m_newlyExposed { false };
    void resizeSwapChain(); 
    RHIVisualManagerLoop::SPtr m_rhiloop;
    core::visual::VisualParams* m_vparams;
    DrawToolRHI* m_drawTool;
    std::shared_ptr<QRhi> m_rhi;
    QRhiSwapChain* m_swapChain;
    bool m_bHasSwapChain{false};
    QRhiRenderBuffer* m_ds = nullptr;
    std::shared_ptr<QRhiRenderPassDescriptor> m_rpDesc;
    QRhiGraphicsPipeline* m_pipeline;
    QRhiBuffer* m_vbuf;
    QRhiBuffer* m_ubuf;
    QRhiShaderResourceBindings* m_srb;

    //
    QRhiRenderPassDescriptor* m_rp = nullptr;
    bool m_vbufReady = false;
    QRhiGraphicsPipeline* m_ps = nullptr;
    QMatrix4x4 m_proj;
    float m_rotation = 0;
    float m_opacity = 1;
    int m_opacityDir = -1;
    //

    QWindow* m_window;
    QWidget* m_container;

    bool m_bhasInit = false;
    bool m_bHasInitTexture = false;
    QSize m_logoSize;
    float m_lookSpeed;
    float m_linearSpeed;
    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_modelviewMatrix;

    // arguments from argument parser (static because parser is in a static function...)
    static std::string s_keyGgraphicsAPI;
    // other stuff if necessary

    

protected:
    friend class InteractionEventManager;

    void keyPressEvent ( QKeyEvent * e ) override;
    void keyReleaseEvent ( QKeyEvent * e ) override;
    void mousePressEvent ( QMouseEvent * e ) override;
    void mouseReleaseEvent ( QMouseEvent * e ) override;
    void mouseMoveEvent ( QMouseEvent * e ) override;
    void wheelEvent ( QWheelEvent* e) override;
    virtual bool mouseEvent ( QMouseEvent * e ) override;
    // reimplementation needed to handle resize events
    // http://doc.qt.io/qt-5/qwidget.html#resizeEvent
    void resizeEvent ( QResizeEvent * event ) override;
    void paintEvent(QPaintEvent* event) override;
    void exposeEvent(QExposeEvent* event, bool isExposed) ;

public slots:
    void resizeView(QSize size);
    void updateVisualParameters();

    friend class RHIBackend;
};

class InteractionEventManager : public QObject
{
    Q_OBJECT

public:
    InteractionEventManager(RHIViewer* parent)
        : QObject(parent)
        , m_viewer(parent)
    {}

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;
    RHIViewer* m_viewer;
};


} // namespace sofa::rhi::gui
