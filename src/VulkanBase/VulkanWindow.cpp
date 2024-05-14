#include "VulkanWindow.h"

#include <stdexcept>

namespace Spectre
{
	VulkanWindow::VulkanWindow(int width, int height, std::string name) : 
		m_Width(width), m_Height(height), m_Name(name)
	{
		InitWindow();
	}

	VulkanWindow::~VulkanWindow()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	void VulkanWindow::CreateSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, m_Window, nullptr, surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create window surface!");
		}
	}

	void VulkanWindow::FrameBufferResizedCallback(GLFWwindow* window, int width, int height)
	{
		auto currentWindow = reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(window));
		currentWindow->m_FrameBufferResized = true;
		currentWindow->m_Width = width;
		currentWindow->m_Height = height;
	}

	void VulkanWindow::InitWindow()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_Window = glfwCreateWindow(m_Width, m_Height, m_Name.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(m_Window, this);
		glfwSetFramebufferSizeCallback(m_Window, FrameBufferResizedCallback);
	}
}
