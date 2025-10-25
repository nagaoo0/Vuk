cmake_minimum_required(VERSION 3.16)
set(CMAKE_GENERATOR_TOOLSET "v143")
project(vkimgui_engine LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional: recommend using vcpkg on Windows for dependencies
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  message(STATUS "Tip: you can use vcpkg and set -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake")
endif()

find_package(Vulkan)
# Try to find GLFW (prefer config mode so vcpkg/MSVC installs work).
# Many GLFW packages provide an imported target named `glfw` or `glfw3::glfw`.
find_package(glfw3 3.3 CONFIG QUIET)

# Determine which imported target (if any) was provided by the package.
set(GLFW_TARGET "")
if(TARGET glfw)
  set(GLFW_TARGET glfw)
elseif(TARGET "glfw3::glfw")
  set(GLFW_TARGET "glfw3::glfw")
elseif(TARGET "glfw::glfw")
  set(GLFW_TARGET "glfw::glfw")
elseif(TARGET glfw3)
  set(GLFW_TARGET glfw3)
endif()

if(GLFW_TARGET STREQUAL "")
  message(STATUS "glfw3 was not found by CMake (no known imported target).")
  message(STATUS "If you use vcpkg on Windows: install with `vcpkg install glfw3` and configure CMake with -DCMAKE_TOOLCHAIN_FILE=path/to/vcpkg/scripts/buildsystems/vcpkg.cmake")
  message(STATUS "Or install GLFW system-wide and set CMAKE_PREFIX_PATH or glfw3_DIR to the folder containing glfw3Config.cmake")
  message(FATAL_ERROR "Could not find GLFW (glfw3). See messages above for install/config instructions.")
endif()

add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/external)

# If find_package(Vulkan) succeeded it provides the imported target Vulkan::Vulkan.
if (TARGET Vulkan::Vulkan)
  target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan ${GLFW_TARGET})
else()
  # Fallback: try the VULKAN_SDK environment variable (LunarG SDK)
  if(DEFINED ENV{VULKAN_SDK})
    message(STATUS "Vulkan not found by CMake; using VULKAN_SDK from the environment: $ENV{VULKAN_SDK}")
    set(Vulkan_INCLUDE_DIR "$ENV{VULKAN_SDK}/Include")
    # prefer 64-bit lib folder; user may need to adjust for x86 or different layout
    set(Vulkan_LIBRARY "$ENV{VULKAN_SDK}/Lib/vulkan-1.lib")
    target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIR})
  target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARY} ${GLFW_TARGET})
  else()
    message(FATAL_ERROR "Vulkan SDK not found. Either install Vulkan SDK (set VULKAN_SDK) or install via vcpkg and provide CMAKE_TOOLCHAIN_FILE.")
  endif()
endif()

# If you add ImGui as a submodule under external/imgui and provide a CMakeLists there,
# uncomment the lines below and link the target (e.g. imgui) to the executable.
# add_subdirectory(external/imgui)
# target_link_libraries(${PROJECT_NAME} PRIVATE imgui)

# Useful output
message(STATUS "Vulkan include: ${Vulkan_INCLUDE_DIR}")
message(STATUS "Vulkan library: ${Vulkan_LIBRARY}")

# Installation / packaging options can be added later
