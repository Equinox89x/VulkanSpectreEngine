#include "ImageBuffer.h"

#include "Utils.h"

#include <sstream>

ImageBuffer::ImageBuffer(const VulkanDevice* device, VkExtent2D size, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples, VkImageAspectFlags aspect, size_t layerCount) : m_Device(device)
{
	const VkDevice device = device->getVkDevice();

	// Create an image
	VkImageCreateInfo imageCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = size.width;
	imageCreateInfo.extent.height = size.height;
	imageCreateInfo.extent.depth = 1u;
	imageCreateInfo.mipLevels = 1u;
	imageCreateInfo.arrayLayers = static_cast<uint32_t>(layerCount);
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage = usage;
	imageCreateInfo.samples = samples;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (vkCreateImage(device, &imageCreateInfo, nullptr, &image) != VK_SUCCESS)
	{

		(EError::GenericVulkan);
		valid = false;
		return;
	}

	// Find a suitable memory type index
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(device, image, &memoryRequirements);

	uint32_t suitableMemoryTypeIndex = 0u;
	if (!utils::findSuitableMemoryTypeIndex(device->getVkPhysicalDevice(), memoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, suitableMemoryTypeIndex))
	{
		utils::Error(EError::FeatureNotSupported, "Suitable image buffer memory type");
		valid = false;
		return;
	}

	// Allocate the device memory for the buffer
	VkMemoryAllocateInfo memoryAllocateInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = suitableMemoryTypeIndex;
	if (vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &deviceMemory) != VK_SUCCESS)
	{
		std::stringstream s;
		s << memoryRequirements.size << " bytes for image buffer";
		utils::Error(EError::OutOfMemory, s.str());
	}

	// Bind the image to the allocated device memory
	if (vkBindImageMemory(device, image, deviceMemory, 0u) != VK_SUCCESS)
	{
		utils::Error(EError::GenericVulkan);
	}

	utils::CreateImageView(image, format, layerCount, aspect, device, imageView);

	//// Create an image view
	//VkImageViewCreateInfo imageViewCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	//imageViewCreateInfo.image = image;
	//imageViewCreateInfo.format = format;
	//imageViewCreateInfo.viewType = (layerCount == 1u ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY);
	//imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	//imageViewCreateInfo.subresourceRange.layerCount = static_cast<uint32_t>(layerCount);
	//imageViewCreateInfo.subresourceRange.aspectMask = aspect;
	//imageViewCreateInfo.subresourceRange.baseArrayLayer = 0u;
	//imageViewCreateInfo.subresourceRange.baseMipLevel = 0u;
	//imageViewCreateInfo.subresourceRange.levelCount = 1u;
	//if (vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView) != VK_SUCCESS)
	//{
	//	utils::Error(EError::GenericVulkan);
	//	valid = false;
	//	return;
	//}
}

ImageBuffer::~ImageBuffer()
{
	const VkDevice device = m_Device->getVkDevice();
	if (device)
	{
		if (imageView)
		{
			vkDestroyImageView(device, imageView, nullptr);
		}

		if (deviceMemory)
		{
			vkFreeMemory(device, deviceMemory, nullptr);
		}

		if (image)
		{
			vkDestroyImage(device, image, nullptr);
		}
	}
}
