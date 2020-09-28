#include <SofaRHI/gui/RHIGUIUtils.h>

namespace sofa::rhi::gui
{

std::map<std::string, std::pair<QRhi::Implementation, std::string> > RHIGUIUtils::MapGraphicsAPI
{
    { "ogl", { QRhi::OpenGLES2, "OpenGL Core" } },
    { "vlk", { QRhi::Vulkan, "Vulkan"} },
    { "d3d", { QRhi::D3D11, "Direct3D 11"} },
    { "mtl", { QRhi::Metal, "Metal"} }
};

std::vector<std::string> RHIGUIUtils::GetSupportedAPIs()
{
    std::vector<std::string> supportedAPIs;

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

    return supportedAPIs;
}

} // namespace sofa::rhi::gui