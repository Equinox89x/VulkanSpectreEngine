#include "App.h"
#include "VulkanBase/VulkanRenderSystem.h"

// std
#include <stdexcept>
#include <chrono>
#include <array>
#include <cassert>

//library
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Spectre
{
	void App::Run()
	{
		VulkanRenderSystem renderSystem{m_Device, m_Renderer.GetRenderPass(), "Shaders/3DShader.vert.spv", "Shaders/3DShader.frag.spv", true };
		VulkanRenderSystem renderSystem2D{m_Device, m_Renderer.GetRenderPass(), "Shaders/2DShader.vert.spv", "Shaders/2DShader.frag.spv", false };

        camera.SetViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));
        auto cameraObject{ GameObject::CreateGameObject() };

        auto currentTime = std::chrono::high_resolution_clock::now();
		while(!m_Window.ShouldWindowClose())
		{
			glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            inputController.HandleKeyboard(m_Window.GetWindow(), deltaTime, cameraObject);
            inputController.HandleMouse(m_Window.GetWindow(), deltaTime, cameraObject);
            camera.SetViewYXZ(cameraObject.m_Transform.translation, cameraObject.m_Transform.rotation);
            camera.SetPerspectiveProjection(glm::radians(50.f), m_Renderer.GetAspectRatio(), 0.1f, 10.f);

            if (auto commandBuffer = m_Renderer.BeginFrame())
            {
                m_Renderer.BeginRenderPass(commandBuffer);
                renderSystem.RenderGameObjects(commandBuffer, m_GameObjects, camera);
                renderSystem2D.RenderGameObjects(commandBuffer, m_GameObjects2D, camera);
                //inputController.HandleKeyboard(m_Window.GetWindow(), deltaTime, m_GameObjects[1]);
                m_Renderer.EndRenderPass(commandBuffer);
                m_Renderer.EndFrame();
            }
		}

		vkDeviceWaitIdle(m_Device.GetDevice());
	}

	void App::CreateGameObjects()
	{
        std::unique_ptr<Mesh> m_Mesh{ std::make_unique<Mesh>(m_Device, 600, 800, "Models/vikingroom.obj", "Textures/viking_room.png") };
        auto cube{ GameObject::CreateGameObject() };
        cube.m_Mesh = std::move(m_Mesh);
        cube.m_Transform.translation = { 1.5f, 0.f, 2.5f };
        cube.m_Transform.scale = { 0.5f, 0.5f, 0.5f };
        cube.m_Transform.rotation = { 1.5,3.0,1.5 };
        m_GameObjects.push_back(std::move(cube));

        std::unique_ptr<Mesh> vikingroom{ std::make_unique<Mesh>(m_Device, 600, 800, "Models/vikingroom.obj", "Textures/viking_room.png")};
        auto room{ GameObject::CreateGameObject() };
        room.m_Mesh = std::move(vikingroom);
        room.m_Transform.translation = { 0.f, 0.5f, 3.f };
        room.m_Transform.scale = { 1.5f, 1.5f, 1.5f };
        room.m_Transform.rotation = { 1.5,3.0,1.5 };
        m_GameObjects.push_back(std::move(room));

        std::unique_ptr<Mesh> Square{ std::make_unique<Mesh>(m_Device, glm::vec2{0,0})};
        Square->CreateSquare(m_Device, glm::vec2{ 0,0});
        auto squareObject{ GameObject::CreateGameObject() };
        squareObject.m_Mesh = std::move(Square);
        squareObject.m_Transform.translation = { -0.6f, 0.6f, 0.f };
        squareObject.m_Transform.scale = { 0.3, 0.3, 0.3f };
        m_GameObjects2D.push_back(std::move(squareObject));        
        
        std::unique_ptr<Mesh> triangle{ std::make_unique<Mesh>(m_Device, glm::vec2{0,0})};
        triangle->CreateOval(m_Device, glm::vec2{ 0, 0 });
        auto triangleObject{ GameObject::CreateGameObject() };
        triangleObject.m_Mesh = std::move(triangle);
        triangleObject.m_Transform.translation = { 0.6f, 0.6f, 0.f };
        triangleObject.m_Transform.scale = { 0.5f, 0.5f, 0.5f };
        m_GameObjects2D.push_back(std::move(triangleObject));


	}
}
