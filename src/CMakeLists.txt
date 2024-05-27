set(TARGET_NAME MainProject)

set(SHADER_SRC
  Shaders/Diffuse.vert
  Shaders/Diffuse.frag 
  
  Shaders/Diffuse2D.vert
  Shaders/Diffuse2D.frag

  Shaders/DiffuseTransparent.vert
  Shaders/DiffuseTransparent.frag

  Shaders/Grid.vert
  Shaders/Grid.frag
)

set(SRC
  "Core/Main.cpp"
  "Core/App.cpp"
  "Core/App.h"

  "VulkanBase/VulkanDevice.cpp"
  "VulkanBase/VulkanDevice.h"

  "VR/Controllers.cpp"
  "VR/Controllers.h"

  "Buffers/DataBuffer.cpp"
  "Buffers/DataBuffer.h"
  "Buffers/ImageBuffer.cpp"
  "Buffers/ImageBuffer.h"

  "VR/Headset.cpp"
  "VR/Headset.h"


  "Scene/MeshData.cpp"
  "Scene/MeshData.h"
  "Scene/GameData.h"

  "VulkanBase/VulkanWindow.cpp"
  "VulkanBase/VulkanWindow.h"


  "VulkanBase/VulkanPipeline.cpp"
  "VulkanBase/VulkanPipeline.h"

  "VulkanBase/VulkanRenderer.cpp"
  "VulkanBase/VulkanRenderer.h"

  "VulkanBase/VulkanRenderSystem.cpp"
  "VulkanBase/VulkanRenderSystem.h"

  "VulkanBase/RenderTarget.cpp"
  "VulkanBase/RenderTarget.h"

  "Misc/Utils.cpp"
  "Misc/Utils.h"
  "Misc/Singleton.h"
  "Misc/Timer.h" 
  "Misc/Timer.cpp"


  "Input/InputHandler.cpp"
  "Input/InputHandler.h"
  "Input/InputActions.h" 
  "Input/InputActions.cpp"


  ${SHADER_SRC}
)

add_executable(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE ${SRC})
target_include_directories(${TARGET_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PRIVATE glfw glm openxr_loader tinyobjloader ${Vulkan_LIBRARIES})

target_compile_definitions(${TARGET_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG>) # Add a clean DEBUG prepocessor define if applicable
set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${TARGET_NAME}>") # For MSVC debugging

# Copy models folder
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E copy_directory "${CMAKE_SOURCE_DIR}/Models" "$<TARGET_FILE_DIR:${TARGET_NAME}>/Models")

# Create output folder for compiled shaders
# Otherwise shader compilation fails
add_custom_command(TARGET ${TARGET_NAME} PRE_BUILD COMMAND ${CMAKE_COMMAND} ARGS -E make_directory "$<TARGET_FILE_DIR:${TARGET_NAME}>/Shaders")

# Compile shaders into output folder
foreach(SHADER ${SHADER_SRC})
  set(SHADER_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}")
  add_custom_command(TARGET ${TARGET_NAME} DEPENDS ${SHADER_INPUT} COMMAND glslc ARGS --target-env=vulkan1.3 ${SHADER_INPUT} -std=450core -O -o "$<TARGET_FILE_DIR:${TARGET_NAME}>/${SHADER}.spv" $<$<NOT:$<CONFIG:DEBUG>>:-O> COMMENT ${SHADER})
endforeach()