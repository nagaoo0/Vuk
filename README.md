Vuk - Minimal Vulkan + ImGui Engine Template

This template provides a minimal starting point for a Vulkan renderer with GLFW and ImGui on Windows.

Prerequisites
- CMake (>=3.16)
- A C++17 compiler
- Vulkan SDK installed (https://vulkan.lunarg.com/)
- vcpkg (recommended) to install dependencies easily on Windows

Using vcpkg (PowerShell):

# Install dependencies
./vcpkg install glfw3:x64-windows
./vcpkg install imgui:x64-windows

# Configure and build
cmake -S . -B build -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release

Notes
- This template only creates a Vulkan instance and a GLFW window. You'll need to add physical/logical device setup, swapchain, command buffers, synchronization and ImGui integration.
- Consider using a helper library or existing samples (e.g. Sascha Willems Vulkan examples) when implementing advanced features.
