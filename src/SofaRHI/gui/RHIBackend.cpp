#include <SofaRHI/gui/RHIBackend.h>

#include <SofaRHI/gui/RHIViewer.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace sofa::rhi::gui
{

RHIBackend::RHIBackend(RHIViewer* rhiviewer)
    : sofa::gui::qt::viewer::EngineBackend()
    , m_rhiviewer(rhiviewer)
    , m_counter(0)
{
}

void RHIBackend::screenshot(const std::string& filename, int compression_level)
{

}

void RHIBackend::setPrefix(const std::string& prefix)
{
    m_prefix = prefix;
}

const std::string RHIBackend::screenshotName()
{
    bool pngSupport = helper::io::Image::FactoryImage::getInstance()->hasKey("png")
            || helper::io::Image::FactoryImage::getInstance()->hasKey("PNG");

    bool bmpSupport = helper::io::Image::FactoryImage::getInstance()->hasKey("bmp")
            || helper::io::Image::FactoryImage::getInstance()->hasKey("BMP");

    std::string filename = "";

    if(!pngSupport && !bmpSupport)
        return filename;

    char buf[32];
    int c;
    c = 0;
    struct stat st;
    do
    {
        ++c;
        sprintf(buf, "%08d",c);
        filename = m_prefix;
        filename += buf;
        if(pngSupport)
            filename += ".png";
        else
            filename += ".bmp";
    }
    while (stat(filename.c_str(),&st)==0);
    m_counter = c+1;

    sprintf(buf, "%08d",c);
    filename = m_prefix;
    filename += buf;
    if(pngSupport)
        filename += ".png";
    else
        filename += ".bmp";

    return filename;
}

} // namespace sofa::rhi::gui
