#include "RenderTarget.h"

#include "../Misc/Utils.h"

#include <array>
#include <iostream>

RenderTarget::RenderTarget(VkDevice device, VkImage image, VkImageView colorImageView, VkImageView depthImageView, VkExtent2D size, VkFormat format, VkRenderPass renderPass, uint32_t layerCount) : m_Device(device), m_Image(image)
{
	utils::CreateImageView(image, format, layerCount, VK_IMAGE_ASPECT_COLOR_BIT, device, m_ImageView);

	const std::array attachments{ colorImageView, depthImageView, m_ImageView };

	// Create a framebuffer
	VkFramebufferCreateInfo framebufferCreateInfo{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	framebufferCreateInfo.renderPass = renderPass;
	framebufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferCreateInfo.pAttachments = attachments.data();
	framebufferCreateInfo.width = size.width;
	framebufferCreateInfo.height = size.height;
	framebufferCreateInfo.layers = 1u;
	if (vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &m_Framebuffer) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}
}

RenderTarget::~RenderTarget()
{
	if (m_Device)
	{

		if (m_Framebuffer)
		{
			vkDestroyFramebuffer(m_Device, m_Framebuffer, nullptr);
		}

		if (m_ImageView)
		{
			vkDestroyImageView(m_Device, m_ImageView, nullptr);
		}
	}
}
