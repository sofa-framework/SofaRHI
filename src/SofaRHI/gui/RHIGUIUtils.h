#pragma once

#include <QtGui/private/qrhinull_p.h>
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

namespace sofa::rhi::gui
{

struct RHIGUIUtils
{
    enum class GraphicsAPI
    {
        OpenGL,
        Vulkan,
        D3D11,
        Metal
    };

    static std::map<std::string, std::pair<QRhi::Implementation, std::string> > MapGraphicsAPI;

    static std::vector<std::string> GetSupportedAPIs();

    static void DisablePluginComponents(const std::vector<std::string>& pluginNameList);

    static void ReplaceVisualModelAliases(const std::vector<std::string>& aliases);
};

} // namespace sofa::rhi::gui
