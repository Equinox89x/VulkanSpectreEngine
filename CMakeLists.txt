set(PROJECT_NAME MainProject)

cmake_minimum_required(VERSION 3.1)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FindVulkan)
include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.8 # Specify the version you need
)

FetchContent_Declare(
  tiny_obj_loader
  GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
  GIT_TAG        v2.0.0rc13 # Specify the version you need
)

FetchContent_Declare(
  openxr
  GIT_REPOSITORY https://github.com/KhronosGroup/OpenXR-SDK.git
  GIT_TAG        release-1.0.25 # Specify the version you need
)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        0.9.9.8 # Specify the version you need
)

FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(tiny_obj_loader)
FetchContent_MakeAvailable(openxr)
FetchContent_MakeAvailable(glm)

include_directories(${openxr_SOURCE_DIR}/include)
include_directories(${Vulkan_INCLUDE_DIRS})
add_subdirectory(src)