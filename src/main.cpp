#define GLFW_INCLUDE_VULKAN
#define DEBUG
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <unordered_set>
#include <string>
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

class VulkanApp
{
public:
    void Run();

private:
    void InitWindows();
    void InitVulkan();
    void MainLoop();
    void CleanUp();

    bool CheckExtensionSupport(std::vector<const char*>& extensions);
    void CreateInstance();
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

    VkInstance instance;
    GLFWwindow* window;
    VkDebugUtilsMessengerEXT debugMessenger;
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

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

    // if (glfwGetPhysicalDevicePresentationSupport(instance, physical_device, queue_family_index))
    // {

    // }

}

void VulkanApp::InitVulkan()
{
    CreateInstance();
    SetupValidationLayer();
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
    if (enableValidationLayers)
    {
        // vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        // DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
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
    vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, NULL);
    if (instanceExtensionCount > 0)
    {
        instanceExtensions.resize(instanceExtensionCount);
        vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, instanceExtensions.data());
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