#include "ImageBuffer.h"

#include "../Misc/Utils.h"

#include <sstream>

ImageBuffer::ImageBuffer(const VulkanDevice* device, VkExtent2D size, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples, VkImageAspectFlags aspect, size_t layerCount) : m_Device(device)
{
	const VkDevice vkDevice{ device->GetVkDevice() };

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
	if (vkCreateImage(vkDevice, &imageCreateInfo, nullptr, &m_Image) != VK_SUCCESS)
	{
		(EError::GenericVulkan);
		return;
	}

	// Find a suitable memory type index
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(vkDevice, m_Image, &memoryRequirements);

	uint32_t suitableMemoryTypeIndex{ 0u };
	if (!utils::FindSuitableMemoryTypeIndex(device->GetVkPhysicalDevice(), memoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, suitableMemoryTypeIndex))
	{
		utils::ThrowError(EError::FeatureNotSupported, "Suitable image buffer memory type");
	}

	// Allocate the device memory for the buffer
	VkMemoryAllocateInfo memoryAllocateInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = suitableMemoryTypeIndex;
	if (vkAllocateMemory(vkDevice, &memoryAllocateInfo, nullptr, &deviceMemory) != VK_SUCCESS)
	{
		std::stringstream s;
		s << memoryRequirements.size << " bytes for image buffer";
		utils::ThrowError(EError::OutOfMemory, s.str());
	}

	// Bind the image to the allocated device memory
	if (vkBindImageMemory(vkDevice, m_Image, deviceMemory, 0u) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}

	// Create an image view
	utils::CreateImageView(m_Image, format, layerCount, aspect, vkDevice, m_ImageView);
}

ImageBuffer::~ImageBuffer()
{
	const VkDevice vkDevice{ m_Device->GetVkDevice() };
	if (vkDevice)
	{
		if (m_ImageView)
		{
			vkDestroyImageView(vkDevice, m_ImageView, nullptr);
		}

		if (deviceMemory)
		{
			vkFreeMemory(vkDevice, deviceMemory, nullptr);
		}

		if (m_Image)
		{
			vkDestroyImage(vkDevice, m_Image, nullptr);
		}
	}
}
