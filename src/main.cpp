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
#include <string>

class HelloTriangleApplication {
public:
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    GLFWwindow* window = nullptr;

    // Vulkan RAII objects
    std::unique_ptr<vk::raii::Context> context;
    std::unique_ptr<vk::raii::Instance> instance;
    std::unique_ptr<vk::raii::DebugUtilsMessengerEXT> debugMessenger;

public:
    void run() {
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    void initVulkan() {
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }

        createInstance();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
        if (!window) {
            throw std::runtime_error("Failed to create GLFW window");
        }
    }

    void createInstance() {
        // Application info
        vk::ApplicationInfo appInfo{};
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = vk::ApiVersion14;

        // Create RAII context early so we can query available layers/extensions
        context = std::make_unique<vk::raii::Context>();

        // Query available layers and extensions
        auto availableLayers = vk::enumerateInstanceLayerProperties();
        auto availableExtensions = vk::enumerateInstanceExtensionProperties();

        // Decide whether to enable validation layers at build-time via macro
#ifdef ENABLE_VALIDATION_LAYERS
        const bool requestValidationLayers = true;
#else
        const bool requestValidationLayers = false;
#endif

        const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
        bool validationLayerPresent = false;
        for (auto const &layer : availableLayers) {
            if (std::string(layer.layerName.data()) == validationLayerName) {
                validationLayerPresent = true;
                break;
            }
        }

        // Get required extensions from GLFW
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions;
        if (glfwExtensions != nullptr) {
            extensions.insert(extensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);
        }

        // Only enable debug utils if available
        bool debugUtilsAvailable = false;
        for (auto const &ext : availableExtensions) {
            if (std::string(ext.extensionName.data()) == VK_EXT_DEBUG_UTILS_EXTENSION_NAME) {
                debugUtilsAvailable = true;
                break;
            }
        }
        if (debugUtilsAvailable) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        // Build lists of enabled layers/extensions
        std::vector<const char*> enabledLayers;
        if (requestValidationLayers && validationLayerPresent) {
            enabledLayers.push_back(validationLayerName);
        }

        // Instance create info
        vk::InstanceCreateInfo createInfo{};
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.empty() ? nullptr : extensions.data();
        createInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
        createInfo.ppEnabledLayerNames = enabledLayers.empty() ? nullptr : enabledLayers.data();

        // Create instance
        instance = std::make_unique<vk::raii::Instance>(*context, createInfo);

        // Setup debug messenger only if extension available and layer requested
        if (debugUtilsAvailable && requestValidationLayers && validationLayerPresent) {
            vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
            debugCreateInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                             vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;
            debugCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                          vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                          vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;
            debugCreateInfo.pfnUserCallback = reinterpret_cast<vk::PFN_DebugUtilsMessengerCallbackEXT>(debugCallback);
            debugMessenger = std::make_unique<vk::raii::DebugUtilsMessengerEXT>(*instance, debugCreateInfo);
        }

        // List physical devices to verify instance/driver
        listPhysicalDevices();
    }

    void listPhysicalDevices() {
        try {
            vk::raii::PhysicalDevices physicalDevices(*instance);
            std::cout << "Available physical devices:\n";
            for (auto const &pd : physicalDevices) {
                auto props = pd.getProperties();
                std::cout << " - " << props.deviceName << " (type=" << static_cast<int>(props.deviceType) << ")\n";
            }
        } catch (const std::exception &e) {
            std::cerr << "Failed to enumerate physical devices: " << e.what() << std::endl;
        }
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

        if (window) {
            glfwDestroyWindow(window);
            window = nullptr;
        }
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