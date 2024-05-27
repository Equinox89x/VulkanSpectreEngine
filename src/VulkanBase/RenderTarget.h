#pragma once

#include <vulkan/vulkan.h>

/*
 * The render target class represents a convenient combination of an image and a framebuffer in Vulkan. The class is
 * used for the Vulkan swapchain images retrieved by OpenXR for the headset displays.
 */
class RenderTarget final
{
public:
	RenderTarget(VkDevice device, VkImage image, VkImageView colorImageView, VkImageView depthImageView, VkExtent2D size, VkFormat format, VkRenderPass m_RenderPass, uint32_t layerCount);
	~RenderTarget();

	VkImage		  GetImage() const { return m_Image; }
	VkFramebuffer GetFramebuffer() const { return m_Framebuffer; }

private:
	VkDevice	  m_Device{ nullptr };
	VkImage		  m_Image{ nullptr };
	VkImageView	  m_ImageView{ nullptr };
	VkFramebuffer m_Framebuffer{ nullptr };
};