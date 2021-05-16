#pragma once

#include <SofaRHI/DrawToolRHI.h>
#include <SofaRHI/RHIVisualManagerLoop.h>

#include <sofa/gui/BaseGUI.h>
#include <sofa/gui/ViewerFactory.h>

#include <sofa/gui/ArgumentParser.h>

#include <QtGui/private/qrhi_p.h>

namespace sofa::rhi::gui
{

class SOFA_SOFARHI_API RHIOffscreenViewer : public sofa::gui::BaseGUI
{

public:

    static const std::string VIEW_FILE_EXTENSION;

    RHIOffscreenViewer();

    /// Start the GUI loop
    int mainLoop() override;
    /// Update the GUI
    void redraw() override;
    /// Close the GUI
    int closeGUI() override;
    /// Register the scene in our GUI
    void setScene(sofa::simulation::Node::SPtr groot, const char* filename = nullptr, bool temporaryFile = false) override;
    /// Get the rootNode of the sofa scene
    sofa::simulation::Node* currentSimulation() override;

    void resetScene();


    static BaseGUI* CreateGUI(const char* name, sofa::simulation::Node::SPtr groot = nullptr, const char* filename = nullptr);
    static int RegisterGUIParameters(sofa::gui::ArgumentParser* argumentParser);
    static void setNumIterations(const std::string& nbIterInp);
private:
    ~RHIOffscreenViewer() override { }

    void updateVisualParameters();
    void resetView();
    void checkScene();
    void drawScene();

    //Application
    static const int DEFAULT_NUMBER_OF_ITERATIONS;

    static QApplication* s_qtApplication;
    static RHIOffscreenViewer* s_gui;
    static int s_nbIterations;
    int m_currentIterations = 0;

    // Simulation
    sofa::simulation::Node::SPtr m_groot;
    std::string m_filename;
    sofa::component::visualmodel::BaseCamera::SPtr m_currentCamera;

    // Render stuff
    RHIVisualManagerLoop::SPtr m_rhiloop;
    core::visual::VisualParams* m_vparams;
    DrawToolRHI* m_drawTool;
    std::shared_ptr<QRhi> m_rhi;
    std::shared_ptr<QRhiRenderPassDescriptor> m_rpDesc;

    bool m_bHasInitTexture = false;

    //Offscreen resources
    QRhiTexture* m_offscreenTexture = nullptr;
    QRhiTextureRenderTarget* m_offscreenTextureRenderTarget = nullptr;
    QRhiViewport m_offscreenViewport;

    // arguments from argument parser (static because parser is in a static function...)
    static std::string s_keyGgraphicsAPI;
    // other stuff if necessary
};



} // namespace sofa::rhi::gui
