#pragma once

#include <vulkan/vulkan.h>

#include <glfw/glfw3.h>
#include <vector>

class VulkanDevice;
struct GLFWwindow;
class Headset;
class VulkanRenderer;

/*
 * The mirror view class handles the creation, updating, resizing, and eventual closing of the desktop window that shows
 * a copy of what is rendered into the headset. It depends on GLFW for handling the operating system, and Vulkan for the
 * blitting into the window surface.
 */
class VulkanWindow final
{
public:
	enum class RenderResult
	{
		ThrowError, // An error occurred
		Visible,	// Visible mirror view for normal rendering
		Invisible	// Minimized window for example without rendering
	};

	VulkanWindow(){};
	VulkanWindow(const VulkanDevice* m_Device);
	~VulkanWindow();

	void OnWindowResize();
	bool Connect(const Headset* headset, const VulkanRenderer* renderer);

	RenderResult Render(uint32_t swapchainImageIndex);
	void		 Present();

	void		 ProcessWindowEvents() const { glfwPollEvents(); }
	bool		 IsExitRequested() const { return static_cast<bool>(glfwWindowShouldClose(m_Window)); }
	VkSurfaceKHR GetSurface() const { return m_Surface; }
	GLFWwindow*	 GetWindow() const { return m_Window; }

private:
	const VulkanDevice*	  m_Device{ nullptr };
	const Headset*		  m_Headset{ nullptr };
	const VulkanRenderer* m_Renderer{ nullptr };
	GLFWwindow*			  m_Window{ nullptr };

	VkSurfaceKHR		 m_Surface{ nullptr };
	VkSwapchainKHR		 m_Swapchain{ nullptr };
	std::vector<VkImage> m_SwapchainImages;
	VkExtent2D			 m_SwapchainResolution{ 0u, 0u };

	uint32_t m_DestinationImageIndex{ 0u };
	bool	 m_ResizeDetected{ false };

	bool RecreateSwapchain();
	bool HandleSurfaceCapabilities(const VkPhysicalDevice& physicalDevice, VkSurfaceCapabilitiesKHR& surfaceCapabilities);
	bool HandleSurfaceFormat(const VkPhysicalDevice& physicalDevice, VkSurfaceFormatKHR& surfaceFormat);
};