#pragma once

#define XR_USE_GRAPHICS_API_OPENGL

#define XR_USE_PLATFORM_WIN32
#include "GL/wglew.h"
#include <unknwn.h>

#include "openxr/openxr.h"
#include "openxr/openxr_platform.h"
#include <vector>

void OPENXR_CHECK(XrResult result, const char *message)
{
    if (result != XR_SUCCESS)
    {
        // Converter o código de resultado em uma string descritiva
        char resultString[XR_MAX_RESULT_STRING_SIZE];
        xrResultToString(nullptr, result, resultString);

        // Imprimir a mensagem de erro com detalhes
        std::cerr << "Erro " << result << " (" << resultString << ") ao " << message << std::endl;
        exit(EXIT_FAILURE);
    }
}

bool OPENXR_CHECK_start_vr(XrResult result, const char *message)
{
    if (result != XR_SUCCESS)
    {
        // Converter o código de resultado em uma string descritiva
        char resultString[XR_MAX_RESULT_STRING_SIZE];
        xrResultToString(nullptr, result, resultString);

        // Imprimir a mensagem de erro com detalhes
        std::cerr << "Erro " << result << " (" << resultString << ") ao " << message << std::endl;

        return 1;
    }
    return 0;
}

XrInstance m_xrInstance = {};
std::vector<const char *> m_activeAPILayers = {};
std::vector<const char *> m_activeInstanceExtensions = {};
std::vector<std::string> m_apiLayers = {};
std::vector<std::string> m_instanceExtensions = {};

XrDebugUtilsMessengerEXT m_debugUtilsMessenger = {};

XrFormFactor m_formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
XrSystemId m_systemID = {};
XrSystemProperties m_systemProperties = {XR_TYPE_SYSTEM_PROPERTIES};

XrApplicationInfo AppInfo;
XrSystemGetInfo systemGI{XR_TYPE_SYSTEM_GET_INFO};

// Função para criar uma instância do OpenXR
void create_openxr_instance()
{

    strncpy(AppInfo.applicationName, "prototipo render vr", XR_MAX_APPLICATION_NAME_SIZE);
    AppInfo.applicationVersion = 1;
    strncpy(AppInfo.engineName, "prototipo render vr", XR_MAX_ENGINE_NAME_SIZE);
    AppInfo.engineVersion = 1;
    AppInfo.apiVersion = XR_CURRENT_API_VERSION;

    m_instanceExtensions.push_back(XR_EXT_DEBUG_UTILS_EXTENSION_NAME);
    m_instanceExtensions.push_back(XR_KHR_OPENGL_ENABLE_EXTENSION_NAME);

    // Get all the API Layers from the OpenXR runtime.
    uint32_t apiLayerCount = 0;
    std::vector<XrApiLayerProperties> apiLayerProperties;
    OPENXR_CHECK(xrEnumerateApiLayerProperties(0, &apiLayerCount, nullptr), "Failed to enumerate ApiLayerProperties.");
    apiLayerProperties.resize(apiLayerCount, {XR_TYPE_API_LAYER_PROPERTIES});
    OPENXR_CHECK(xrEnumerateApiLayerProperties(apiLayerCount, &apiLayerCount, apiLayerProperties.data()), "Failed to enumerate ApiLayerProperties.");

    // Check the requested API layers against the ones from the OpenXR. If found add it to the Active API Layers.
    for (auto &requestLayer : m_apiLayers)
    {
        for (auto &layerProperty : apiLayerProperties)
        {
            // strcmp returns 0 if the strings match.

            if (strcmp(requestLayer.c_str(), layerProperty.layerName) != 0)
            {
                continue;
            }
            else
            {
                m_activeAPILayers.push_back(requestLayer.c_str());
                break;
            }
        }
    }

    // Get all the Instance Extensions from the OpenXR instance.
    uint32_t extensionCount = 0;
    std::vector<XrExtensionProperties> extensionProperties;
    OPENXR_CHECK(xrEnumerateInstanceExtensionProperties(nullptr, 0, &extensionCount, nullptr), "Failed to enumerate InstanceExtensionProperties.");
    extensionProperties.resize(extensionCount, {XR_TYPE_EXTENSION_PROPERTIES});
    OPENXR_CHECK(xrEnumerateInstanceExtensionProperties(nullptr, extensionCount, &extensionCount, extensionProperties.data()), "Failed to enumerate InstanceExtensionProperties.");

    // Check the requested Instance Extensions against the ones from the OpenXR runtime.
    // If an extension is found add it to Active Instance Extensions.
    // Log error if the Instance Extension is not found.
    for (auto &requestedInstanceExtension : m_instanceExtensions)
    {
        bool found = false;
        for (auto &extensionProperty : extensionProperties)
        {
            // strcmp returns 0 if the strings match.
            if (strcmp(requestedInstanceExtension.c_str(), extensionProperty.extensionName) != 0)
            {
                continue;
            }
            else
            {
                m_activeInstanceExtensions.push_back(requestedInstanceExtension.c_str());
                found = true;
                break;
            }
        }
        if (!found)
        {
            // XR_TUT_LOG_ERROR("Failed to find OpenXR instance extension: " << requestedInstanceExtension);
            std::cerr << "Failed to find OpenXR instance extension: " << requestedInstanceExtension << std::endl;
        }
    }

    XrInstanceCreateInfo instanceCI = {XR_TYPE_INSTANCE_CREATE_INFO};
    instanceCI.createFlags = 0;
    instanceCI.applicationInfo = AppInfo;
    instanceCI.enabledApiLayerCount = static_cast<uint32_t>(m_activeAPILayers.size());
    instanceCI.enabledApiLayerNames = m_activeAPILayers.data();
    instanceCI.enabledExtensionCount = static_cast<uint32_t>(m_activeInstanceExtensions.size());
    instanceCI.enabledExtensionNames = m_activeInstanceExtensions.data();

    while (OPENXR_CHECK_start_vr(xrCreateInstance(&instanceCI, &m_xrInstance), "Failed to create Instance."))
    {
    }

    // Get the XrSystemId from the instance and the supplied XrFormFactor.
    systemGI.formFactor = m_formFactor;
    OPENXR_CHECK(xrGetSystem(m_xrInstance, &systemGI, &m_systemID), "Failed to get SystemID.");

    // Get the System's properties for some general information about the hardware and the vendor.
    OPENXR_CHECK(xrGetSystemProperties(m_xrInstance, m_systemID, &m_systemProperties), "Failed to get SystemProperties.");
}

XrSessionCreateInfo sessionCI{XR_TYPE_SESSION_CREATE_INFO};

XrSession m_session = XR_NULL_HANDLE;

//parei em 2.3
void start_openxr()
{
    create_openxr_instance();

}

void update_openxr()
{
}

void end_openxr(){
    OPENXR_CHECK(xrDestroySession(m_session), "Failed to destroy Session.");
}