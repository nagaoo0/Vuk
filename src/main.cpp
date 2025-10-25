#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <memory>
#include <optional>

class HelloTriangleApplication {
public:
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    GLFWwindow* window;
public:
    void run() {
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    // Vulkan RAII objects
    std::unique_ptr<vk::raii::Context> context;
    std::unique_ptr<vk::raii::Instance> instance;
    std::unique_ptr<vk::raii::DebugUtilsMessengerEXT> debugMessenger;
    void initVulkan() {
        glfwInit();
        createInstance();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void initWindow() {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void createInstance() {
        // Application info
        vk::ApplicationInfo appInfo{};
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = vk::ApiVersion14;

        // Validation layers (enable if available)
        const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

        // Get required extensions from GLFW
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions;
        if (glfwExtensions != nullptr) {
            extensions.insert(extensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);
        }

        // Add debug utils extension for validation messages
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        // Instance create info
        vk::InstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        // Create RAII context and instance
        context = std::make_unique<vk::raii::Context>();
        instance = std::make_unique<vk::raii::Instance>(*context, createInfo);

        // Setup debug messenger (optional - requires VK_EXT_debug_utils enabled)
        vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        debugCreateInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                         vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
        debugCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
    // Cast to Vulkan-Hpp PFN type to satisfy MSVC's stricter type checking
    debugCreateInfo.pfnUserCallback = reinterpret_cast<vk::PFN_DebugUtilsMessengerCallbackEXT>(debugCallback);

        // Create RAII debug messenger
        debugMessenger = std::make_unique<vk::raii::DebugUtilsMessengerEXT>(*instance, debugCreateInfo);
    }

    // Validation/debug callback (C-style signature)
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                       VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                       const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                       void* pUserData) {
        std::cerr << "Validation layer: " << (pCallbackData ? pCallbackData->pMessage : "(no message)") << std::endl;
        return VK_FALSE;
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }

    }

    void cleanup() {
        // Destroy Vulkan RAII objects in reverse order of creation
        debugMessenger.reset();
        instance.reset();
        context.reset();

        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}