#pragma once

#define XR_USE_GRAPHICS_API_OPENGL

#define XR_USE_PLATFORM_WIN32
#include "GL/wglew.h"
#include "GL/gl.h"
#include <unknwn.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3native.h>

#include "openxr/openxr.h"
#include "openxr/openxr_platform.h"
#include <vector>
#include <algorithm>
#include <map>
#include <unordered_map>
#include "print.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/constants.hpp>

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

XrSessionCreateInfo sessionCI{XR_TYPE_SESSION_CREATE_INFO};

XrSession m_session = XR_NULL_HANDLE;
XrSessionState m_sessionState = XR_SESSION_STATE_UNKNOWN;

XrEventDataSessionStateChanged *sessionStateChanged;

XrInstanceCreateInfo instanceCI = {XR_TYPE_INSTANCE_CREATE_INFO};

int openxr_create_instance()
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
            // print_ERROR("Failed to find OpenXR instance extension: " << requestedInstanceExtension);
            std::cerr << "Failed to find OpenXR instance extension: " << requestedInstanceExtension << std::endl;
        }
    }

    instanceCI.createFlags = 0;
    instanceCI.applicationInfo = AppInfo;
    instanceCI.enabledApiLayerCount = static_cast<uint32_t>(m_activeAPILayers.size());
    instanceCI.enabledApiLayerNames = m_activeAPILayers.data();
    instanceCI.enabledExtensionCount = static_cast<uint32_t>(m_activeInstanceExtensions.size());
    instanceCI.enabledExtensionNames = m_activeInstanceExtensions.data();

    if (OPENXR_CHECK_start_vr(xrCreateInstance(&instanceCI, &m_xrInstance), "Failed to create Instance."))
    {
        return 1;
    }

    // Get the XrSystemId from the instance and the supplied XrFormFactor.
    systemGI.formFactor = m_formFactor;
    OPENXR_CHECK(xrGetSystem(m_xrInstance, &systemGI, &m_systemID), "Failed to get SystemID.");

    // Get the System's properties for some general information about the hardware and the vendor.
    OPENXR_CHECK(xrGetSystemProperties(m_xrInstance, m_systemID, &m_systemProperties), "Failed to get SystemProperties.");

    return 0;
}

#if defined(XR_USE_PLATFORM_WIN32)
XrGraphicsBindingOpenGLWin32KHR graphicsBinding{};
#elif defined(XR_USE_PLATFORM_XLIB)
XrGraphicsBindingOpenGLXlibKHR graphicsBinding{};
#elif defined(XR_USE_PLATFORM_XCB)
XrGraphicsBindingOpenGLXcbKHR graphicsBinding{};
#elif defined(XR_USE_PLATFORM_WAYLAND)
XrGraphicsBindingOpenGLWaylandKHR graphicsBinding{};
#endif

void *gl_GetGraphicsBinding()
{

    // https://github.com/KhronosGroup/OpenXR-SDK-Source/blob/f122f9f1fc729e2dc82e12c3ce73efa875182854/src/tests/hello_xr/graphicsplugin_opengl.cpp#L123-L144
#if defined(XR_USE_PLATFORM_WIN32)

    graphicsBinding = {XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR};
    XrGraphicsBindingOpenGLWin32KHR graphicsBinding = {XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR};
    HWND hWnd = glfwGetWin32Window(win);
    graphicsBinding.type = XR_TYPE_GRAPHICS_BINDING_OPENGL_WIN32_KHR;
    graphicsBinding.next = nullptr;
    graphicsBinding.hDC = GetDC(hWnd); // Obtém o HDC da janela
    graphicsBinding.hGLRC = wglCreateContext(graphicsBinding.hDC);

#elif defined(XR_USE_PLATFORM_XLIB)
    graphicsBinding = {XR_TYPE_GRAPHICS_BINDING_OPENGL_XLIB_KHR};
    graphicsBinding.xDisplay = window.context.xDisplay;
    graphicsBinding.visualid = window.context.visualid;
    graphicsBinding.glxFBConfig = window.context.glxFBConfig;
    graphicsBinding.glxDrawable = window.context.glxDrawable;
    graphicsBinding.glxContext = window.context.glxContext;
#elif defined(XR_USE_PLATFORM_XCB)
    graphicsBinding = {XR_TYPE_GRAPHICS_BINDING_OPENGL_XCB_KHR};
    graphicsBinding.connection = window.connection;
    graphicsBinding.visualid = window.context.visualid;
    graphicsBinding.glxDrawable = window.context.glxDrawable;
#elif defined(XR_USE_PLATFORM_WAYLAND)
    graphicsBinding = {XR_TYPE_GRAPHICS_BINDING_OPENGL_WAYLAND};
    graphicsBinding.display = reinterpret_cast<wl_display *>(0xFFFFFFFF);
#endif
    return &graphicsBinding;
}

int openxr_create_section()
{
    XrSessionCreateInfo sessionCI{XR_TYPE_SESSION_CREATE_INFO};

    sessionCI.next = gl_GetGraphicsBinding();
    sessionCI.createFlags = 0;
    sessionCI.systemId = m_systemID;

    

    OPENXR_CHECK(xrCreateSession(m_xrInstance, &sessionCI, &m_session), "Failed to create Session.");

    return 0;
}

std::vector<XrViewConfigurationType> m_applicationViewConfigurations = {XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO, XR_VIEW_CONFIGURATION_TYPE_PRIMARY_MONO};
std::vector<XrViewConfigurationType> m_viewConfigurations;
XrViewConfigurationType m_viewConfiguration = XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM;
std::vector<XrViewConfigurationView> m_viewConfigurationViews;

struct SwapchainInfo
{
    XrSwapchain swapchain = XR_NULL_HANDLE;
    int64_t swapchainFormat = 0;
    std::vector<void *> imageViews;
};
std::vector<SwapchainInfo> m_colorSwapchainInfos = {};
std::vector<SwapchainInfo> m_depthSwapchainInfos = {};

enum SwapchainType
{
    COLOR = 0,
    DEPTH = 1
};

std::unordered_map<XrSwapchain, std::pair<SwapchainType, std::vector<XrSwapchainImageOpenGLKHR>>> gl_swapchainImagesMap{};

XrSwapchainImageBaseHeader *gl_AllocateSwapchainImageData(XrSwapchain swapchain, SwapchainType type, uint32_t count)
{
    gl_swapchainImagesMap[swapchain].first = type;
    gl_swapchainImagesMap[swapchain].second.resize(count, {XR_TYPE_SWAPCHAIN_IMAGE_OPENGL_KHR});
    return reinterpret_cast<XrSwapchainImageBaseHeader *>(gl_swapchainImagesMap[swapchain].second.data());
}

void *gl_GetSwapchainImage(XrSwapchain swapchain, uint32_t index) { return (void *)(uint64_t)gl_swapchainImagesMap[swapchain].second[index].image; }

class gl_ImageViewCreateInfo
{
public:
    void *image;
    uint32_t view;
    uint32_t format;
    SwapchainType aspect;
    uint32_t baseMipLevel;
    uint32_t levelCount;
    uint32_t baseArrayLayer;
    uint32_t layerCount;

    gl_ImageViewCreateInfo()
    {
        view = GL_TEXTURE_2D;
        format = GL_SRGB8_ALPHA8;
        this->aspect = aspect;
        baseMipLevel = 0;
        levelCount = 1;
        baseArrayLayer = 0;
        layerCount = 1;
    }
};

std::unordered_map<GLenum, gl_ImageViewCreateInfo> gl_imageViews = {};

void *gl_CreateImageView(gl_ImageViewCreateInfo &imageViewCI)
{
    GLuint framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);

    GLenum attachment = imageViewCI.aspect == COLOR ? GL_COLOR_ATTACHMENT0 : GL_DEPTH_ATTACHMENT;

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, GL_TEXTURE_2D, (GLuint)(uint64_t)imageViewCI.image, imageViewCI.baseMipLevel);

    GLenum result = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
    if (result != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR: OPENGL: Framebuffer is not complete." << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    gl_imageViews[framebuffer] = imageViewCI;
    return (void *)(uint64_t)framebuffer;
}

int openxr_create_spaw_chains()
{
    // Gets the View Configuration Types. The first call gets the count of the array that will be returned. The next call fills out the array.
    uint32_t viewConfigurationCount = 0;
    OPENXR_CHECK(xrEnumerateViewConfigurations(m_xrInstance, m_systemID, 0, &viewConfigurationCount, nullptr), "Failed to enumerate View Configurations.");
    m_viewConfigurations.resize(viewConfigurationCount);
    OPENXR_CHECK(xrEnumerateViewConfigurations(m_xrInstance, m_systemID, viewConfigurationCount, &viewConfigurationCount, m_viewConfigurations.data()), "Failed to enumerate View Configurations.");

    // Pick the first application supported View Configuration Type con supported by the hardware.
    for (const XrViewConfigurationType &viewConfiguration : m_applicationViewConfigurations)
    {
        if (std::find(m_viewConfigurations.begin(), m_viewConfigurations.end(), viewConfiguration) != m_viewConfigurations.end())
        {
            m_viewConfiguration = viewConfiguration;
            break;
        }
    }
    if (m_viewConfiguration == XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM)
    {
        std::cerr << "Failed to find a view configuration type. Defaulting to XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO." << std::endl;
        m_viewConfiguration = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    }

    // Gets the View Configuration Views. The first call gets the count of the array that will be returned. The next call fills out the array.
    uint32_t viewConfigurationViewCount = 0;
    OPENXR_CHECK(xrEnumerateViewConfigurationViews(m_xrInstance, m_systemID, m_viewConfiguration, 0, &viewConfigurationViewCount, nullptr), "Failed to enumerate ViewConfiguration Views.");
    m_viewConfigurationViews.resize(viewConfigurationViewCount, {XR_TYPE_VIEW_CONFIGURATION_VIEW});
    OPENXR_CHECK(xrEnumerateViewConfigurationViews(m_xrInstance, m_systemID, m_viewConfiguration, viewConfigurationViewCount, &viewConfigurationViewCount, m_viewConfigurationViews.data()), "Failed to enumerate ViewConfiguration Views.");

    // Get the supported swapchain formats as an array of int64_t and ordered by runtime preference.
    uint32_t formatCount = 0;
    OPENXR_CHECK(xrEnumerateSwapchainFormats(m_session, 0, &formatCount, nullptr), "Failed to enumerate Swapchain Formats");
    std::vector<int64_t> formats(formatCount);
    OPENXR_CHECK(xrEnumerateSwapchainFormats(m_session, formatCount, &formatCount, formats.data()), "Failed to enumerate Swapchain Formats");
    if (GL_SRGB8_ALPHA8 == 0)
    {
        std::cout << "Failed to find depth format for Swapchain." << std::endl;
        return 1;
    }
    for (size_t i = 0; i < m_viewConfigurationViews.size(); i++)
    {
        SwapchainInfo &colorSwapchainInfo = m_colorSwapchainInfos[i];
        SwapchainInfo &depthSwapchainInfo = m_depthSwapchainInfos[i];

        // Fill out an XrSwapchainCreateInfo structure and create an XrSwapchain.
        // Color.
        XrSwapchainCreateInfo swapchainCI{XR_TYPE_SWAPCHAIN_CREATE_INFO};
        swapchainCI.createFlags = 0;
        swapchainCI.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCI.format = GL_SRGB8_ALPHA8;                                                  // Use GraphicsAPI to select the first compatible format.
        swapchainCI.sampleCount = m_viewConfigurationViews[i].recommendedSwapchainSampleCount; // Use the recommended values from the XrViewConfigurationView.
        swapchainCI.width = m_viewConfigurationViews[i].recommendedImageRectWidth;
        swapchainCI.height = m_viewConfigurationViews[i].recommendedImageRectHeight;
        swapchainCI.faceCount = 1;
        swapchainCI.arraySize = 1;
        swapchainCI.mipCount = 1;
        OPENXR_CHECK(xrCreateSwapchain(m_session, &swapchainCI, &colorSwapchainInfo.swapchain), "Failed to create Color Swapchain");
        colorSwapchainInfo.swapchainFormat = swapchainCI.format; // Save the swapchain format for later use.

        // Depth.
        swapchainCI.createFlags = 0;
        swapchainCI.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        swapchainCI.format = GL_DEPTH24_STENCIL8;                                              // Use GraphicsAPI to select the first compatible format.
        swapchainCI.sampleCount = m_viewConfigurationViews[i].recommendedSwapchainSampleCount; // Use the recommended values from the XrViewConfigurationView.
        swapchainCI.width = m_viewConfigurationViews[i].recommendedImageRectWidth;
        swapchainCI.height = m_viewConfigurationViews[i].recommendedImageRectHeight;
        swapchainCI.faceCount = 1;
        swapchainCI.arraySize = 1;
        swapchainCI.mipCount = 1;
        OPENXR_CHECK(xrCreateSwapchain(m_session, &swapchainCI, &depthSwapchainInfo.swapchain), "Failed to create Depth Swapchain");
        depthSwapchainInfo.swapchainFormat = swapchainCI.format; // Save the swapchain format for later use.

        // Get the number of images in the color/depth swapchain and allocate Swapchain image data via GraphicsAPI to store the returned array.
        uint32_t colorSwapchainImageCount = 0;
        OPENXR_CHECK(xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, 0, &colorSwapchainImageCount, nullptr), "Failed to enumerate Color Swapchain Images.");
        XrSwapchainImageBaseHeader *colorSwapchainImages = gl_AllocateSwapchainImageData(colorSwapchainInfo.swapchain, SwapchainType::COLOR, colorSwapchainImageCount);
        OPENXR_CHECK(xrEnumerateSwapchainImages(colorSwapchainInfo.swapchain, colorSwapchainImageCount, &colorSwapchainImageCount, colorSwapchainImages), "Failed to enumerate Color Swapchain Images.");

        uint32_t depthSwapchainImageCount = 0;
        OPENXR_CHECK(xrEnumerateSwapchainImages(depthSwapchainInfo.swapchain, 0, &depthSwapchainImageCount, nullptr), "Failed to enumerate Depth Swapchain Images.");
        XrSwapchainImageBaseHeader *depthSwapchainImages = gl_AllocateSwapchainImageData(depthSwapchainInfo.swapchain, SwapchainType::DEPTH, depthSwapchainImageCount);
        OPENXR_CHECK(xrEnumerateSwapchainImages(depthSwapchainInfo.swapchain, depthSwapchainImageCount, &depthSwapchainImageCount, depthSwapchainImages), "Failed to enumerate Depth Swapchain Images.");

        // Per image in the swapchains, fill out a GraphicsAPI::ImageViewCreateInfo structure and create a color/depth image view.
        for (uint32_t j = 0; j < colorSwapchainImageCount; j++)
        {
            gl_ImageViewCreateInfo imageViewCI;
            imageViewCI.image = gl_GetSwapchainImage(colorSwapchainInfo.swapchain, j);
            imageViewCI.aspect = COLOR;
            imageViewCI.baseMipLevel = 0;
            imageViewCI.levelCount = 1;
            imageViewCI.baseArrayLayer = 0;
            imageViewCI.layerCount = 1;
            colorSwapchainInfo.imageViews.push_back(gl_CreateImageView(imageViewCI));
        }
        for (uint32_t j = 0; j < depthSwapchainImageCount; j++)
        {
            gl_ImageViewCreateInfo imageViewCI;
            imageViewCI.image = gl_GetSwapchainImage(colorSwapchainInfo.swapchain, j);
            imageViewCI.aspect = DEPTH;
            imageViewCI.baseMipLevel = 0;
            imageViewCI.levelCount = 1;
            imageViewCI.baseArrayLayer = 0;
            imageViewCI.layerCount = 1;
            depthSwapchainInfo.imageViews.push_back(gl_CreateImageView(imageViewCI));
        }
    }

    // parei em 3.2

    return 0;
}

int start_openxr()
{
    if (openxr_create_instance())
        return 1;
    if (openxr_create_section())
        return 1;
    if (openxr_create_spaw_chains())
        return 1;

    return 0;
}

bool m_sessionRunning = true;
bool m_applicationRunning = true;

void openxr_base_loop()
{
    // Poll OpenXR for a new event.
    XrEventDataBuffer eventData{XR_TYPE_EVENT_DATA_BUFFER};
    auto XrPollEvents = [&]() -> bool
    {
        eventData = {XR_TYPE_EVENT_DATA_BUFFER};
        return xrPollEvent(m_xrInstance, &eventData) == XR_SUCCESS;
    };

    while (XrPollEvents())
    {
        switch (eventData.type)
        {
        // Log the number of lost events from the runtime.
        case XR_TYPE_EVENT_DATA_EVENTS_LOST:
        {
            XrEventDataEventsLost *eventsLost = reinterpret_cast<XrEventDataEventsLost *>(&eventData);
            print("OPENXR: Events Lost: ", eventsLost->lostEventCount);
            break;
        }
        // Log that an instance loss is pending and shutdown the application.
        case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
        {
            XrEventDataInstanceLossPending *instanceLossPending = reinterpret_cast<XrEventDataInstanceLossPending *>(&eventData);
            print("OPENXR: Instance Loss Pending at: ", instanceLossPending->lossTime);
            m_sessionRunning = false;
            m_applicationRunning = false;
            break;
        }
        // Log that the interaction profile has changed.
        case XR_TYPE_EVENT_DATA_INTERACTION_PROFILE_CHANGED:
        {
            XrEventDataInteractionProfileChanged *interactionProfileChanged = reinterpret_cast<XrEventDataInteractionProfileChanged *>(&eventData);
            print("OPENXR: Interaction Profile changed for Session: ", interactionProfileChanged->session);
            if (interactionProfileChanged->session != m_session)
            {
                print("XrEventDataInteractionProfileChanged for unknown Session");
                break;
            }
            break;
        }
        // Log that there's a reference space change pending.
        case XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING:
        {
            XrEventDataReferenceSpaceChangePending *referenceSpaceChangePending = reinterpret_cast<XrEventDataReferenceSpaceChangePending *>(&eventData);
            print("OPENXR: Reference Space Change pending for Session: ", referenceSpaceChangePending->session);
            if (referenceSpaceChangePending->session != m_session)
            {
                print("XrEventDataReferenceSpaceChangePending for unknown Session");
                break;
            }
            break;
        }
        // Session State changes:
        case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
        {
            sessionStateChanged = reinterpret_cast<XrEventDataSessionStateChanged *>(&eventData);
            if (sessionStateChanged->session != m_session)
            {
                print("XrEventDataSessionStateChanged for unknown Session");
                break;
            }

            if (sessionStateChanged->state == XR_SESSION_STATE_READY)
            {
                // SessionState is ready. Begin the XrSession using the XrViewConfigurationType.
                XrSessionBeginInfo sessionBeginInfo{XR_TYPE_SESSION_BEGIN_INFO};
                sessionBeginInfo.primaryViewConfigurationType = m_viewConfiguration;
                // sessionBeginInfo.primaryViewConfigurationType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
                OPENXR_CHECK(xrBeginSession(m_session, &sessionBeginInfo), "Failed to begin Session.");
                m_sessionRunning = true;
            }
            if (sessionStateChanged->state == XR_SESSION_STATE_STOPPING)
            {
                // SessionState is stopping. End the XrSession.
                OPENXR_CHECK(xrEndSession(m_session), "Failed to end Session.");
                m_sessionRunning = false;
            }
            if (sessionStateChanged->state == XR_SESSION_STATE_EXITING)
            {
                // SessionState is exiting. Exit the application.
                m_sessionRunning = false;
                m_applicationRunning = false;
            }
            if (sessionStateChanged->state == XR_SESSION_STATE_LOSS_PENDING)
            {
                // SessionState is loss pending. Exit the application.
                // It's possible to try a reestablish an XrInstance and XrSession, but we will simply exit here.
                m_sessionRunning = false;
                m_applicationRunning = false;
            }
            // Store state for reference across the application.
            m_sessionState = sessionStateChanged->state;

            break;
        }
        default:
        {
            break;
        }
        }
    }
}

/*
struct headset_info_struct
{
    glm::mat4 views[2];
    glm::mat4 projections[2];

    glm::vec3 position;
    glm::quat rotation;
};
typedef struct headset_info_struct headset_info;
*/

uint32_t viewConfigurationCount;

uint32_t viewConfigurationViewCount = 0;

void get_view()
{
    // Gets the View Configuration Types. The first call gets the count of the array that will be returned. The next call fills out the array.
    viewConfigurationCount = 0;
    OPENXR_CHECK(xrEnumerateViewConfigurations(m_xrInstance, m_systemID, 0, &viewConfigurationCount, nullptr), "Failed to enumerate View Configurations.");
    m_viewConfigurations.resize(viewConfigurationCount);
    OPENXR_CHECK(xrEnumerateViewConfigurations(m_xrInstance, m_systemID, viewConfigurationCount, &viewConfigurationCount, m_viewConfigurations.data()), "Failed to enumerate View Configurations.");

    // Pick the first application supported View Configuration Type con supported by the hardware.
    for (const XrViewConfigurationType &viewConfiguration : m_applicationViewConfigurations)
    {
        if (std::find(m_viewConfigurations.begin(), m_viewConfigurations.end(), viewConfiguration) != m_viewConfigurations.end())
        {
            m_viewConfiguration = viewConfiguration;
            break;
        }
    }
    if (m_viewConfiguration == XR_VIEW_CONFIGURATION_TYPE_MAX_ENUM)
    {
        std::cerr << "Failed to find a view configuration type. Defaulting to XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO." << std::endl;
        m_viewConfiguration = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
    }

    // Gets the View Configuration Views. The first call gets the count of the array that will be returned. The next call fills out the array.

    OPENXR_CHECK(xrEnumerateViewConfigurationViews(m_xrInstance, m_systemID, m_viewConfiguration, 0, &viewConfigurationViewCount, nullptr), "Failed to enumerate ViewConfiguration Views.");
    m_viewConfigurationViews.resize(viewConfigurationViewCount, {XR_TYPE_VIEW_CONFIGURATION_VIEW});
    OPENXR_CHECK(xrEnumerateViewConfigurationViews(m_xrInstance, m_systemID, m_viewConfiguration, viewConfigurationViewCount, &viewConfigurationViewCount, m_viewConfigurationViews.data()), "Failed to enumerate ViewConfiguration Views.");
}

void update_openxr()
{
    openxr_base_loop();
}

bool the_vr_show_should_continue() { return m_sessionRunning && m_applicationRunning; }

void end_openxr()
{
    OPENXR_CHECK(xrDestroySession(m_session), "Failed to destroy Session.");
}