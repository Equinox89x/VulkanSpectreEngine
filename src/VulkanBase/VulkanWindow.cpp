#include "VulkanWindow.h"

#include "../Misc/Utils.h"
#include "../VR/Headset.h"
#include "../VulkanBase/RenderTarget.h"
#include "../VulkanBase/VulkanDevice.h"
#include "VulkanRenderer.h"

#include <glm/common.hpp>

#include <sstream>

namespace Spectre
{
	constexpr VkFormat		   colorFormat{ VK_FORMAT_B8G8R8A8_SRGB };
	constexpr VkPresentModeKHR presentMode{ VK_PRESENT_MODE_FIFO_KHR };
	constexpr size_t		   mirrorEyeIndex{ 1u }; // Eye index to mirror, 0 = left, 1 = right

	void framebufferSizeCallback(GLFWwindow* vkWindow, int width, int height)
	{
		VulkanWindow* window{ reinterpret_cast<VulkanWindow*>(glfwGetWindowUserPointer(vkWindow)) };
		window->OnWindowResize();
	}

	void keyCallback(GLFWwindow* vkWindow, int key, int scancode, int action, int mods)
	{
		if (action == GLFW_RELEASE && key == GLFW_KEY_ESCAPE)
		{
			glfwSetWindowShouldClose(vkWindow, 1);
		}
	}
} // namespace Spectre

VulkanWindow::VulkanWindow(const VulkanDevice* device) : m_Device(device)
{
	// Create a fullscreen window
	GLFWmonitor* monitor{ glfwGetPrimaryMonitor() };

	int width, height;
	glfwGetMonitorWorkarea(monitor, nullptr, nullptr, &width, &height);

#ifdef DEBUG
	// Create a quarter-sized window in debug mode instead
	width /= 1.1f;
	height /= 1.1f;
	monitor = nullptr;
#endif

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_Window = glfwCreateWindow(width, height, Spectre::applicationName.c_str(), monitor, nullptr);
	if (!m_Window)
	{
		std::stringstream s;
		s << width << "x" << height << (monitor ? " fullscreen" : " windowed");
		utils::ThrowError(EError::WindowFailure, s.str());
	}

	glfwSetWindowUserPointer(m_Window, this);
	glfwSetFramebufferSizeCallback(m_Window, Spectre::framebufferSizeCallback);
	glfwSetKeyCallback(m_Window, Spectre::keyCallback);

	// Hide the mouse cursor
	glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	// Create a surface for the window
	VkResult result = glfwCreateWindowSurface(device->GetVkInstance(), m_Window, nullptr, &m_Surface);
	if (result != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericGLFW);
	}
}

VulkanWindow::~VulkanWindow()
{
	const VkDevice vkDevice{ m_Device->GetVkDevice() };
	if (vkDevice && m_Swapchain)
	{
		vkDestroySwapchainKHR(vkDevice, m_Swapchain, nullptr);
	}

	const VkInstance instance = m_Device->GetVkInstance();
	if (instance && m_Surface)
	{
		vkDestroySurfaceKHR(instance, m_Surface, nullptr);
	}

	if (m_Window)
	{
		glfwDestroyWindow(m_Window);
	}

	glfwTerminate();
}

void VulkanWindow::OnWindowResize() { m_ResizeDetected = true; }

bool VulkanWindow::Connect(const Headset* headset, const VulkanRenderer* renderer)
{
	this->m_Headset = headset;
	this->m_Renderer = renderer;

	if (!RecreateSwapchain())
	{
		return false;
	}

	return true;
}

VulkanWindow::RenderResult VulkanWindow::Render(uint32_t swapchainImageIndex)
{
	if (m_SwapchainResolution.width == 0u || m_SwapchainResolution.height == 0u)
	{
		// Just check for maximizing as long as the window is minimized
		if (m_ResizeDetected)
		{
			m_ResizeDetected = false;
			if (!RecreateSwapchain())
			{
				return RenderResult::ThrowError;
			}
		}
		else
		{
			// Otherwise skip minimized frames
			return RenderResult::Invisible;
		}
	}

	const VkResult result = vkAcquireNextImageKHR(m_Device->GetVkDevice(), m_Swapchain, UINT64_MAX, m_Renderer->GetCurrentDrawableSemaphore(), VK_NULL_HANDLE, &m_DestinationImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		// Recreate the swapchain and then stop rendering this frame as it is out of date already
		if (!RecreateSwapchain())
		{
			return RenderResult::ThrowError;
		}

		return RenderResult::Invisible;
	}
	else if (result != VK_SUBOPTIMAL_KHR && result != VK_SUCCESS)
	{
		// Treat a suboptimal like a successful frame
		return RenderResult::Invisible;
	}

	const VkCommandBuffer commandBuffer{ m_Renderer->GetCurrentCommandBuffer() };
	const VkImage		  sourceImage{ m_Headset->GetRenderTarget(swapchainImageIndex)->GetImage() };
	const VkImage		  destinationImage{ m_SwapchainImages.at(m_DestinationImageIndex) };
	const VkExtent2D	  eyeResolution{ m_Headset->GetEyeResolution(Spectre::mirrorEyeIndex) };

	// Convert the source image layout from undefined to transfer source
	VkImageMemoryBarrier imageMemoryBarrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	imageMemoryBarrier.image = sourceImage;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarrier.subresourceRange.layerCount = 1u;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = Spectre::mirrorEyeIndex;
	imageMemoryBarrier.subresourceRange.levelCount = 1u;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0u;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0u, nullptr, 0u, nullptr, 1u, &imageMemoryBarrier);

	// Convert the destination image layout from undefined to transfer destination
	imageMemoryBarrier.image = destinationImage;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemoryBarrier.srcAccessMask = 0u;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarrier.subresourceRange.layerCount = 1u;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0u;
	imageMemoryBarrier.subresourceRange.levelCount = 1u;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0u;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0u, nullptr, 0u, nullptr, 1u, &imageMemoryBarrier);

	// We need to crop the source image region to preserve the aspect ratio of the mirror view window
	const glm::vec2 sourceResolution{ static_cast<float>(eyeResolution.width), static_cast<float>(eyeResolution.height) };
	const float		sourceAspectRatio{ sourceResolution.x / sourceResolution.y };
	const glm::vec2 destinationResolution{ static_cast<float>(m_SwapchainResolution.width), static_cast<float>(m_SwapchainResolution.height) };
	const float		destinationAspectRatio{ destinationResolution.x / destinationResolution.y };
	glm::vec2		cropResolution = sourceResolution, cropOffset = { 0.0f, 0.0f };

	if (sourceAspectRatio < destinationAspectRatio)
	{
		cropResolution.y = sourceResolution.x / destinationAspectRatio;
		cropOffset.y = (sourceResolution.y - cropResolution.y) / 2.0f;
	}
	else if (sourceAspectRatio > destinationAspectRatio)
	{
		cropResolution.x = sourceResolution.y * destinationAspectRatio;
		cropOffset.x = (sourceResolution.x - cropResolution.x) / 2.0f;
	}

	// Blit the source to the destination image
	VkImageBlit imageBlit{};
	imageBlit.srcOffsets[0] = { static_cast<int32_t>(cropOffset.x), static_cast<int32_t>(cropOffset.y), 0 };
	imageBlit.srcOffsets[1] = { static_cast<int32_t>(cropOffset.x + cropResolution.x), static_cast<int32_t>(cropOffset.y + cropResolution.y), 1 };
	imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlit.srcSubresource.mipLevel = 0u;
	imageBlit.srcSubresource.baseArrayLayer = Spectre::mirrorEyeIndex;
	imageBlit.srcSubresource.layerCount = 1u;

	imageBlit.dstOffsets[0] = { 0, 0, 0 };
	imageBlit.dstOffsets[1] = { static_cast<int32_t>(destinationResolution.x), static_cast<int32_t>(destinationResolution.y), 1 };
	imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageBlit.dstSubresource.layerCount = 1u;
	imageBlit.dstSubresource.baseArrayLayer = 0u;
	imageBlit.dstSubresource.mipLevel = 0u;
	vkCmdBlitImage(commandBuffer, sourceImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, destinationImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &imageBlit, VK_FILTER_NEAREST);

	// Convert the source image layout from transfer source to color attachment
	imageMemoryBarrier.image = sourceImage;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarrier.subresourceRange.layerCount = 1u;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = Spectre::mirrorEyeIndex;
	imageMemoryBarrier.subresourceRange.levelCount = 1u;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0u;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0u, nullptr, 0u, nullptr, 1u, &imageMemoryBarrier);

	// Convert the destination image layout from transfer destination to present
	imageMemoryBarrier.image = destinationImage;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = 0u;
	imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageMemoryBarrier.subresourceRange.layerCount = 1u;
	imageMemoryBarrier.subresourceRange.baseArrayLayer = 0u;
	imageMemoryBarrier.subresourceRange.levelCount = 1u;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0u;
	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_DEPENDENCY_BY_REGION_BIT, 0u, nullptr, 0u, nullptr, 1u, &imageMemoryBarrier);

	return RenderResult::Visible;
}

void VulkanWindow::Present()
{
	const VkSemaphore presentableSemaphore{ m_Renderer->GetCurrentPresentableSemaphore() };

	VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.waitSemaphoreCount = 1u;
	presentInfo.pWaitSemaphores = &presentableSemaphore;
	presentInfo.swapchainCount = 1u;
	presentInfo.pSwapchains = &m_Swapchain;
	presentInfo.pImageIndices = &m_DestinationImageIndex;

	const VkResult result = vkQueuePresentKHR(m_Device->GetVkPresentQueue(), &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		// Recreate the swapchain for the next frame if necessary
		if (!RecreateSwapchain())
		{
			return;
		}
	}
	else if (result != VK_SUCCESS)
	{
		return;
	}
}

bool VulkanWindow::RecreateSwapchain()
{
	m_Device->Sync();

	const VkPhysicalDevice physicalDevice{ m_Device->GetVkPhysicalDevice() };

	// Get the surface capabilities and extent
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	HandleSurfaceCapabilities(physicalDevice, surfaceCapabilities);

	// Get the surface formats and pick one with the desired color format support
	VkSurfaceFormatKHR surfaceFormat;
	HandleSurfaceFormat(physicalDevice, surfaceFormat);

	const VkDevice vkDevice{ m_Device->GetVkDevice() };

	// Clean up before recreating the swapchain and render targets
	if (m_Swapchain)
	{
		vkDestroySwapchainKHR(vkDevice, m_Swapchain, nullptr);
	}

	// Create a new swapchain
	VkSwapchainCreateInfoKHR swapchainCreateInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	swapchainCreateInfo.surface = m_Surface;
	swapchainCreateInfo.presentMode = Spectre::presentMode;
	swapchainCreateInfo.minImageCount = surfaceCapabilities.minImageCount + 1u;
	swapchainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
	swapchainCreateInfo.imageFormat = surfaceFormat.format;
	swapchainCreateInfo.imageExtent = m_SwapchainResolution;
	swapchainCreateInfo.imageArrayLayers = 1u;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	if (vkCreateSwapchainKHR(vkDevice, &swapchainCreateInfo, nullptr, &m_Swapchain) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
		return false;
	}

	// Retrieve the new swapchain images
	uint32_t swapchainImageCount{ 0u };
	if (vkGetSwapchainImagesKHR(vkDevice, m_Swapchain, &swapchainImageCount, nullptr) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
		return false;
	}

	m_SwapchainImages.resize(swapchainImageCount);
	if (vkGetSwapchainImagesKHR(vkDevice, m_Swapchain, &swapchainImageCount, m_SwapchainImages.data()) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
		return false;
	}

	return true;
}

bool VulkanWindow::HandleSurfaceCapabilities(const VkPhysicalDevice& physicalDevice, VkSurfaceCapabilitiesKHR& surfaceCapabilities)
{

	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, m_Surface, &surfaceCapabilities) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
		return false;
	}

	if (!(surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
	{
		utils::ThrowError(EError::GenericVulkan);
		return false;
	}

	if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max() && surfaceCapabilities.currentExtent.height != std::numeric_limits<uint32_t>::max())
	{
		// Use any valid extent
		m_SwapchainResolution = surfaceCapabilities.currentExtent;
	}
	else
	{
		// Find the closest extent to use instead of an invalid extent
		int width, height;
		glfwGetFramebufferSize(m_Window, &width, &height);

		m_SwapchainResolution.width = glm::clamp(width, static_cast<int>(surfaceCapabilities.minImageExtent.width), static_cast<int>(surfaceCapabilities.maxImageExtent.width));
		m_SwapchainResolution.height = glm::clamp(height, static_cast<int>(surfaceCapabilities.minImageExtent.height), static_cast<int>(surfaceCapabilities.maxImageExtent.height));
	}

	// Skip the rest if the window was minimized
	if (m_SwapchainResolution.width == 0u || m_SwapchainResolution.height == 0u)
	{
		return true;
	}

	return true;
}

bool VulkanWindow::HandleSurfaceFormat(const VkPhysicalDevice& physicalDevice, VkSurfaceFormatKHR& surfaceFormat)
{
	uint32_t surfaceFormatCount{ 0u };
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &surfaceFormatCount, nullptr) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
		return false;
	}

	std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, m_Surface, &surfaceFormatCount, surfaceFormats.data()) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
		return false;
	}

	// Find the surface format to use
	bool surfaceFormatFound = false;
	for (const VkSurfaceFormatKHR& surfaceFormatCandidate : surfaceFormats)
	{
		if (surfaceFormatCandidate.format == Spectre::colorFormat)
		{
			surfaceFormat = surfaceFormatCandidate;
			surfaceFormatFound = true;
			break;
		}
	}

	if (!surfaceFormatFound)
	{
		utils::ThrowError(EError::FeatureNotSupported, "Vulkan swapchain color format");
		return false;
	}

	return true;
}
