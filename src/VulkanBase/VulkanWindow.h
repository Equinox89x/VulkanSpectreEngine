#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <string>

namespace Spectre
{
	class VulkanWindow final
	{
	public:
		VulkanWindow(int width, int height, std::string name);
		~VulkanWindow();
		VulkanWindow(const VulkanWindow&) = delete;
		VulkanWindow(VulkanWindow&&) = delete;
		VulkanWindow& operator=(const VulkanWindow&) = delete;
		VulkanWindow& operator=(VulkanWindow&&) = delete;

		bool ShouldWindowClose() { return glfwWindowShouldClose(m_Window); }
		VkExtent2D GetExtent() { return { static_cast<uint32_t>(m_Width), static_cast<uint32_t>(m_Height) }; }
		bool WasWindowResized() { return m_FrameBufferResized; }
		void ResetWindowResizedFlag() { m_FrameBufferResized = false; }
		GLFWwindow* GetWindow() const { return m_Window; }

		void CreateSurface(VkInstance instance, VkSurfaceKHR* surface);


	private:
		static void FrameBufferResizedCallback(GLFWwindow* window, int width, int height);
		void InitWindow();

		int m_Width;
		int m_Height;
		bool m_FrameBufferResized{ false };

		std::string m_Name;
		GLFWwindow* m_Window;
	};
}
