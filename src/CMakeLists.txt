# Source files
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Shaders")
set(SHADER_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/Shaders")

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${SHADER_SOURCE_DIR}/*.frag"
    "${SHADER_SOURCE_DIR}/*.vert"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${SHADER_BINARY_DIR}/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)


add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)

set(SOURCES
    "Core/main.cpp"
    "VulkanBase/VulkanWindow.h" 
    "VulkanBase/VulkanWindow.cpp" 
    "Core/App.h" 
    "Core/App.cpp" 
    "VulkanBase/VulkanPipeline.h" 
    "VulkanBase/VulkanPipeline.cpp" 
    "VulkanBase/VulkanDevice.h" 
    "VulkanBase/VulkanDevice.cpp" 
    "VulkanBase/VulkanSwapChain.h" 
    "VulkanBase/VulkanSwapChain.cpp" 
    "Scene/Mesh.h" 
    "Scene/Mesh.cpp" 
    "Scene/GameObject.h" 
    "VulkanBase/VulkanRenderer.h" 
    "VulkanBase/VulkanRenderer.cpp" 
    "VulkanBase/VulkanRenderSystem.h" 
    "VulkanBase/VulkanRenderSystem.cpp" 
    "Camera/Camera.h" 
    "Camera/Camera.cpp" 
    "Input/InputHandler.h" 
    "Input/InputHandler.cpp"
)

# Create the executable
add_executable(MainProject ${SOURCES} ${GLSL_SOURCE_FILES}  )
add_dependencies(${PROJECT_NAME} Shaders)

set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)

# Link libraries
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(MainProject PRIVATE ${Vulkan_LIBRARIES} glfw)
target_link_libraries(MainProject PRIVATE ${Vulkan_LIBRARIES} tinyobjloader)

 add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                             COMMAND ${CMAKE_COMMAND} -E copy_directory
                             ${CMAKE_SOURCE_DIR}/src/Models
                             $<TARGET_FILE_DIR:${PROJECT_NAME}>/Models)

 add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                             COMMAND ${CMAKE_COMMAND} -E copy_directory
                             ${CMAKE_SOURCE_DIR}/src/Textures
                             $<TARGET_FILE_DIR:${PROJECT_NAME}>/Textures)