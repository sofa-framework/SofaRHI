#include <SofaRHI/gui/RHIGUIUtils.h>

#include <sofa/helper/system/PluginManager.h>
#include <sofa/core/ObjectFactory.h>

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


void RHIGUIUtils::DisablePluginComponents(const std::vector<std::string>& pluginNameList)
{
    for (const auto& pluginName : pluginNameList)
    {
        auto& pluginManager = sofa::helper::system::PluginManager::getInstance();
        if (pluginManager.pluginIsLoaded(pluginName))
        {
            msg_warning("RHIGUI") << pluginName << " plugin has been loaded and is incompatible with SofaRHI";
            msg_warning("RHIGUI") << "SofaRHI will disable all its components to prevent crashing.";

            std::vector<sofa::core::ObjectFactory::ClassEntry::SPtr> listComponents;
            sofa::core::ObjectFactory::getInstance()->getEntriesFromTarget(listComponents, pluginName);
            sofa::core::ObjectFactory::ClassEntry::SPtr classEntry;
            for (auto component : listComponents)
            {
                sofa::core::ObjectFactory::AddAlias(component->className, "DisabledObject", true,
                    &classEntry);
            }
        }
    }
}

void RHIGUIUtils::ReplaceVisualModelAliases(const std::vector<std::string>& aliases)
{
    for (const auto& alias : aliases)
    {
        msg_warning("RHIGUI") << "All occurences of " << alias << " will be replaced by RHIModel";

        sofa::core::ObjectFactory::ClassEntry::SPtr classEntry;
        sofa::core::ObjectFactory::AddAlias(alias, "RHIModel", true,
            &classEntry);
    }
}

} // namespace sofa::rhi::gui