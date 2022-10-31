#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>

const static int WIDTH = 800;
const static int HEIGHT = 600;

class VulkanApp
{
public:
    void run();

private:
    GLFWwindow *window;
    void initGlfw();
    void initVulkan();
    void mainLoop();
    void cleanup();
};

void VulkanApp::run()
{
    initGlfw();
    initVulkan();
    mainLoop();
    cleanup();
}

void VulkanApp::initGlfw()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    window = glfwCreateWindow(WIDTH, HEIGHT, "My Vulkan Window", NULL, NULL);
}

void VulkanApp::initVulkan()
{
}

void VulkanApp::mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

void VulkanApp::cleanup()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

int main(int argc, const char **argvs)
{
    VulkanApp app;
    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}