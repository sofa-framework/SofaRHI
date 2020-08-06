#pragma once

#include <sofa/helper/system/config.h>

#include <sofa/gui/qt/viewer/SofaViewer.h>
#include <sofa/gui/ViewerFactory.h>

#include <sofa/defaulttype/Vec.h>
#include <sofa/defaulttype/Quat.h>

#include <SofaSimulationCommon/xml/Element.h>

#include <SofaRHI/DrawToolRHI.h>
#include <SofaRHI/gui/RHIBackend.h>
#include <SofaRHI/RHIModel.h>

#include <QtGui/private/qrhi_p.h>
#include <QtGui/private/qrhinull_p.h>
#include <QSurfaceFormat>
#include <QOffscreenSurface>
#include <QPainter>

namespace sofa::rhi
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

    RHIViewer( QWidget* parent, const char* name="", const unsigned int nbMSAASamples = 1, bool replaceOgl = true );
    ~RHIViewer() override;

    QWidget* getQWidget() override { return this; }

    bool ready() override {return !_waitForRender;}
    void wait() override;

    void update();
    bool load() override;
    bool unload() override;

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
    virtual void captureEvent()  override { std::cout << "proute" <<std::endl;SofaViewer::captureEvent(); }
    virtual void drawColourPicking (sofa::gui::ColourPickingVisitor::ColourCode code) override ;
    virtual void fitNodeBBox(sofa::core::objectmodel::BaseNode * node )  override { SofaViewer::fitNodeBBox(node); }
    virtual void fitObjectBBox(sofa::core::objectmodel::BaseObject * obj)  override { SofaViewer::fitObjectBBox(obj); }

signals:
    void redrawn() override ;
    void resizeW( int ) override ;
    void resizeH( int ) override ;
    void quit();


protected:
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
    virtual void	drawScene() override;

    helper::vector<RHIModel::SPtr> m_rhiModels;
    core::visual::VisualParams* m_vparams;
    sofa::core::visual::DrawToolRHI* m_drawTool;
    std::shared_ptr<QRhi> m_rhi;
    QRhiSwapChain* m_swapChain;
    std::shared_ptr<QRhiRenderPassDescriptor> m_rpDesc;
    QRhiGraphicsPipeline* m_pipeline;
    QRhiBuffer* m_vbuf;
    QRhiBuffer* m_ubuf;
    QRhiShaderResourceBindings* m_srb;

    QWindow* m_window;
    QWidget* m_container;

    bool m_bFirst;
    QSize m_logoSize;
    float m_lookSpeed;
    float m_linearSpeed;
    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_modelviewMatrix;

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

public slots:
    void resizeView(QSize size);
    void frameUpdated();
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


} // namespace sofa::rhi
