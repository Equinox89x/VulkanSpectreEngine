#pragma once
#include "VulkanBase/VulkanWindow.h"
#include "VulkanBase/VulkanDevice.h"
#include "Scene/GameObject.h"
#include "Camera/Camera.h"
#include "Input/InputHandler.h"
#include "VulkanBase/VulkanRenderer.h"

// std includes
#include <memory>
#include <vector>
#include <unordered_map>

namespace Spectre
{
	class App final
	{
	public:
		static constexpr int m_WIDTH{ 800 };
		static constexpr int m_HEIGHT{ 600 };

		App() { CreateGameObjects(); };
		~App() = default;
		App(const App&) = delete;
		App(App&&) = delete;
		App& operator=(const App&) = delete;
		App& operator=(App&&) = delete;

		void Run();

	private:
		void CreateGameObjects();

		VulkanWindow m_Window{ m_WIDTH, m_HEIGHT, "Spectre Engine" };
		VulkanDevice m_Device{ m_Window };

		VulkanRenderer m_Renderer{ m_Window, m_Device };

		std::vector<GameObject> m_GameObjects;
		std::vector<GameObject> m_GameObjects2D;

		Camera camera{};
		InputHandler inputController{};
	};
}
