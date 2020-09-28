#pragma once

#include <SofaRHI/config.h>
#include <sofa/gui/qt/viewer/EngineBackend.h>


namespace sofa::rhi::gui
{
class RHIViewer;

class RHIBackend : public sofa::gui::qt::viewer::EngineBackend
{
public:
    RHIBackend(RHIViewer* rhiviewer);
    virtual ~RHIBackend() {}

    virtual void setPickingMethod(sofa::gui::PickHandler* pick, sofa::component::configurationsetting::ViewerSetting* viewerConf){}
    virtual void setPrefix(const std::string& prefix);
    virtual const std::string screenshotName();
    virtual void screenshot(const std::string& filename, int compression_level);
    virtual void setBackgroundImage(helper::io::Image* image){}
    virtual void drawBackgroundImage(const int screenWidth, const int screenHeight) {};

    virtual bool initRecorder(int width, int height, unsigned int framerate, unsigned int bitrate, const std::string& codecExtension="", const std::string& codecName="")  {return true;}
    virtual void endRecorder(){}
    virtual void addFrameRecorder(){}

private:
    RHIViewer* m_rhiviewer;
    std::string m_prefix;
    int m_counter;
};

} // namespace sofa::rhi::gui



