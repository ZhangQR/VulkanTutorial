#define GLFW_INCLUDE_VULKAN
#define DEBUG
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <unordered_set>
#include <string>
#include <optional>
#include <set>
#include <algorithm>
#include <limits>
const static int Width = 800;
const static int Height = 640;
std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
#ifdef DEBUG
bool enableValidationLayers = true;
#else
bool enableValidationLayers = false;
#endif

std::vector<const char*>deviceExtents = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool IsComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanApp
{
public:
    void Run();

private:
    void InitWindows();
    void InitVulkan();
    void MainLoop();
    void CleanUp();
    void PickPhysicalDevice();
    void CreateLogicDevice();
    bool IsPhysicalDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice);
    bool CheckPhysicalDeviceExtents(VkPhysicalDevice physicalDevice);
    void CreateSwapChain();
    SwapChainSupportDetails GetSwapChainSupportDetails(VkPhysicalDevice physicalDevice);
    VkSurfaceFormatKHR GetSwapChainImageFormat(SwapChainSupportDetails);
    VkExtent2D GetSwapChainImageExtent(SwapChainSupportDetails);
    VkPresentModeKHR GetSwapChainPresentMode(SwapChainSupportDetails);

    bool CheckExtensionSupport(std::vector<const char*>& extensions);
    void CreateInstance();
    void CreateSurface();
    void SetupValidationLayer();
    bool CheckValidationLayerSupport();
    void GetVkDebugUtilsMessengerCreateInfoEXT(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    VkResult CreateDebugUtilsMessenger(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pMessenger);
    
    VkResult DestroyDebugUtilsMessengerEXT(
        VkInstance instance, 
        VkDebugUtilsMessengerEXT debugMessenger, 
        const VkAllocationCallbacks* pAllocator);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    VkInstance instance;
    GLFWwindow* window;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
};

void VulkanApp::Run()
{
    InitWindows();
    InitVulkan();
    MainLoop();
    CleanUp();
}

void VulkanApp::InitWindows()
{
    if (!glfwInit())
    {
        std::cerr << "glfw init failed" << std::endl;
        return;
    }

    if (!glfwVulkanSupported())
    {
        std::cerr << "glfw is not support" << std::endl;
        return;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(Width, Height, "Vulkan Tutorial", nullptr, nullptr);

}

void VulkanApp::InitVulkan()
{
    CreateInstance();
    CreateSurface();
    SetupValidationLayer();
    PickPhysicalDevice();
    CreateLogicDevice();
    CreateSwapChain();
}

void VulkanApp::MainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

void VulkanApp::CleanUp()
{
    vkDestroySwapchainKHR(device, swapChain, nullptr);
    vkDestroyDevice(device, nullptr);
    if (enableValidationLayers)
    {
        // vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool VulkanApp::CheckExtensionSupport(std::vector<const char*>& extensions)
{
    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        // TODO:check VK_EXT_DEBUG_UTILS_EXTENSION_NAME support
    }
    
    uint32_t count;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&count);

    std::unordered_set<std::string> cache;
    for (uint32_t i = 0; i < count; i++) {
        extensions.push_back(glfwExtensions[i]);
        cache.insert(glfwExtensions[i]);
    }
    uint32_t instanceExtensionCount;
    std::vector<VkExtensionProperties> instanceExtensions;
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);
    if (instanceExtensionCount > 0)
    {
        instanceExtensions.resize(instanceExtensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, instanceExtensions.data());
    }
    for (uint32_t i = 0; i < instanceExtensionCount; i++) {
        cache.erase(instanceExtensions[i].extensionName);
    }
    return cache.empty();
}

void VulkanApp::CreateInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VulkanTutorial";
    appInfo.applicationVersion = 1;
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = 1;
    appInfo.apiVersion = VK_API_VERSION_1_0;
    
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    std::vector<const char*> extensions;

    if (!CheckExtensionSupport(extensions))
    {
        std::cerr << "check support failed" << std::endl;
        return;
    }
    
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (enableValidationLayers)
    {
        if (!CheckValidationLayerSupport())
        {
            std::cerr << "Validation is not support" << std::endl;
        }
        VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{};
        GetVkDebugUtilsMessengerCreateInfoEXT(debugMessengerCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugMessengerCreateInfo;
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }
    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
    {
        std::cerr << "create instance error" << std::endl;
    }
}

void VulkanApp::CreateSurface()
{
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("can't create windows surface");
    }
}

void VulkanApp::SetupValidationLayer()
{
    if (!enableValidationLayers)
    {
        std::cout << "validation layer not enabled!\n";
        return;
    }
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    GetVkDebugUtilsMessengerCreateInfoEXT(createInfo);
    //if (vkCreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    //{
    //    std::cerr << "create validation layers failed" << std::endl;
    //}

    if (CreateDebugUtilsMessenger(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
        std::cerr << "create validation layers failed" << std::endl;
    }
}

void VulkanApp::PickPhysicalDevice()
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance, &count, nullptr);
    if (count <= 0)
    {
        std::cerr << "no physical device" << std::endl;
    }
    std::vector<VkPhysicalDevice> physicalDeviceList(count);
    vkEnumeratePhysicalDevices(instance, &count, physicalDeviceList.data());

    for (auto& device : physicalDeviceList)
    {
        if (IsPhysicalDeviceSuitable(device))
        {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

bool VulkanApp::IsPhysicalDeviceSuitable(VkPhysicalDevice device)
{
    // 还可以检查是独显，核显；支持哪些特性等
    //VkPhysicalDeviceProperties deviceProperties;
    //VkPhysicalDeviceFeatures deviceFeatures;
    //vkGetPhysicalDeviceProperties(device, &deviceProperties);
    //vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    //return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
    //    deviceFeatures.geometryShader;
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(device);
    bool isExtentsSupport = CheckPhysicalDeviceExtents(device);

    bool swapChainAdequate = false;
    SwapChainSupportDetails details = GetSwapChainSupportDetails(device);
    swapChainAdequate = !details.formats.empty() && !details.presentModes.empty();
    
    return queueFamilyIndices.IsComplete() && isExtentsSupport && swapChainAdequate;
}

QueueFamilyIndices VulkanApp::FindQueueFamilies(VkPhysicalDevice physicalDevice)
{
    QueueFamilyIndices indices;
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamily(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, queueFamily.data());
    int i = 0;
    for (auto& qf : queueFamily)
    {
        if (qf.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }
        VkBool32 surfaceSupport = VK_FALSE;
        if (vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &surfaceSupport) == VK_SUCCESS)
        {
            indices.presentFamily = i;
        }
        if (indices.IsComplete())
        {
            break;
        }
        i++;
    }
    return indices;
}

bool VulkanApp::CheckPhysicalDeviceExtents(VkPhysicalDevice physicalDevice)
{
    uint32_t count;
    vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> availableExtents;
    if (count > 0)
    {
        availableExtents.resize(count);
        vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, availableExtents.data());
    }
    std::set<std::string> requiredExtents(deviceExtents.begin(), deviceExtents.end());
    for (auto extent : availableExtents)
    {
        requiredExtents.erase(extent.extensionName);
    }
    
    return requiredExtents.empty();
}


void VulkanApp::CreateLogicDevice()
{
    QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies{ indices.graphicsFamily.value(),indices.presentFamily.value() };
    float priorities = 1.0f;
    for (uint32_t queueFamilyIndex : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.pNext = nullptr;
        queueCreateInfo.flags = 0;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &priorities;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    
    // device validation layers 已经弃用，但是最好保持和 instance 一致，以兼容
    if (enableValidationLayers)
    {
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        deviceCreateInfo.enabledLayerCount = 0;
    }

    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtents.size());
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtents.data();
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
    if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS) 
    {
        throw std::runtime_error("can't create device");
    }
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void VulkanApp::CreateSwapChain()
{
    SwapChainSupportDetails details = GetSwapChainSupportDetails(physicalDevice);
    VkSwapchainCreateInfoKHR createInfo{};
    VkSurfaceFormatKHR imageFormat = GetSwapChainImageFormat(details);
    VkExtent2D imageExtent = GetSwapChainImageExtent(details);
    VkPresentModeKHR presentMode = GetSwapChainPresentMode(details);
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.surface = surface;
    createInfo.minImageCount = details.capabilities.minImageCount + 1;
    createInfo.imageFormat = imageFormat.format;
    createInfo.imageColorSpace = imageFormat.colorSpace;
    createInfo.imageExtent = imageExtent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
    uint32_t indicesArray[] = { indices.presentFamily.value(),indices.graphicsFamily.value() };
    if (indices.presentFamily != indices.graphicsFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = indicesArray;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("create swap chain failed!");
    }
    uint32_t swapChainImageCount;
    vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, nullptr);
    if (swapChainImageCount != 0)
    {
        vkGetSwapchainImagesKHR(device, swapChain, &swapChainImageCount, swapChainImages.data());
    }
    swapChainExtent = imageExtent;
    swapChainImageFormat = imageFormat.format;
}

VkSurfaceFormatKHR VulkanApp::GetSwapChainImageFormat(SwapChainSupportDetails details)
{
    for (auto sf : details.formats)
    {
        if (sf.format == VK_FORMAT_R8G8B8A8_SRGB && sf.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return sf;
        }
    }
    return details.formats[0];
}

VkPresentModeKHR VulkanApp::GetSwapChainPresentMode(SwapChainSupportDetails details)
{
    for (auto mode : details.presentModes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanApp::GetSwapChainImageExtent(SwapChainSupportDetails details)
{
    if (details.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return details.capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        VkExtent2D extent = {
            static_cast<uint32_t>(width),static_cast<uint32_t>(height)
        };
        extent.width = std::clamp(extent.width, details.capabilities.minImageExtent.width, details.capabilities.maxImageExtent.width);
        extent.height = std::clamp(extent.height, details.capabilities.minImageExtent.height, details.capabilities.maxImageExtent.height);
        return extent;
    }
}

SwapChainSupportDetails VulkanApp::GetSwapChainSupportDetails(VkPhysicalDevice physicalDevice)
{
    SwapChainSupportDetails details;
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities) != VK_SUCCESS)
    {
        std::cerr << "query swap chain capabilities failed";
    }
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    if (formatCount > 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
    }
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    if (presentModeCount > 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
    }
    return details;
}

bool VulkanApp::CheckValidationLayerSupport()
{
    if (!enableValidationLayers)
    {
        return true;
    }

    std::unordered_set<std::string> cache;
    for (uint32_t i = 0; i < validationLayers.size(); i++) {
        cache.insert(validationLayers[i]);
    }
    uint32_t instanceLayersCount;
    std::vector<VkLayerProperties> instanceValidationLayers;
    vkEnumerateInstanceExtensionProperties(NULL, &instanceLayersCount, NULL);
    if (instanceLayersCount > 0)
    {
        instanceValidationLayers.resize(instanceLayersCount);
        vkEnumerateInstanceLayerProperties(&instanceLayersCount, instanceValidationLayers.data());
    }
    for (uint32_t i = 0; i < instanceLayersCount; i++) {
        cache.erase(instanceValidationLayers[i].layerName);
    }
    return cache.empty();
}

void VulkanApp::GetVkDebugUtilsMessengerCreateInfoEXT(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

VkResult VulkanApp::CreateDebugUtilsMessenger(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pMessenger)
{
    auto fun = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (fun == nullptr)
    {
        std::cerr << "create debug util messenger failed" << std::endl;
        return VK_INCOMPLETE;
    }
    return fun(instance, pCreateInfo, pAllocator, pMessenger);
}


VkResult VulkanApp::DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator)
{
    auto fun = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (fun == nullptr)
    {
        std::cerr << "destroy debug util messenger failed" << std::endl;
        return VK_INCOMPLETE;
    }
    fun(instance, debugMessenger, pAllocator);
}

int main()
{
    VulkanApp app;
    try
    {
        app.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}