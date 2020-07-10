#include <sofa/core/ObjectFactory.h>

#include <SofaRHI/config.h>

namespace sofa
{

//Here are just several convenient functions to help user to know what contains the plugin

extern "C" {
    SOFA_SOFARHI_API void initExternalModule();
    SOFA_SOFARHI_API const char* getModuleName();
    SOFA_SOFARHI_API const char* getModuleVersion();
    SOFA_SOFARHI_API const char* getModuleLicense();
    SOFA_SOFARHI_API const char* getModuleDescription();
    SOFA_SOFARHI_API const char* getModuleComponentList();
}

void initExternalModule()
{
    static bool first = true;
    if (first)
    {
        first = false;
        //sofa::core::ObjectFactory::getInstance()->dump();
    }
}

const char* getModuleName()
{
    return sofa_tostring(SOFA_TARGET);
}

const char* getModuleVersion()
{
    return sofa_tostring(SOFARHI_VERSION);
}

const char* getModuleLicense()
{
    return "LGPL";
}


const char* getModuleDescription()
{
    return "A Sofa plugin with RHI (from Qt)";
}

const char* getModuleComponentList()
{
    /// string containing the names of the classes provided by the plugin
    static std::string classes = sofa::core::ObjectFactory::getInstance()->listClassesFromTarget(sofa_tostring(SOFA_TARGET));
    return classes.c_str();
}

} // namespace sofa
