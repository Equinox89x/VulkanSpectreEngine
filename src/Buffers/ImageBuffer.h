#pragma once

#include "../VulkanBase/VulkanDevice.h"

/*
 * The image buffer class represents a convienent combination of an image, its associated memory, and a corresponding
 * image view in Vulkan. The class is used to bundle all required resources for the color and depth buffer respectively.
 */
class ImageBuffer final
{
public:
	ImageBuffer(const VulkanDevice* m_Device, VkExtent2D size, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples, VkImageAspectFlags aspect, size_t layerCount);
	~ImageBuffer();

	VkImageView GetImageView() const { return m_ImageView; }

private:
	const VulkanDevice* m_Device{ nullptr };
	VkImage				m_Image{ nullptr };
	VkDeviceMemory		deviceMemory{ nullptr };
	VkImageView			m_ImageView{ nullptr };
};