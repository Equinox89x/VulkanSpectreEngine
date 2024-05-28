#include "TextureBuffer.h"

#include "../Misc/Utils.h"
#include <sstream>
//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>


TextureBuffer::TextureBuffer(const VulkanDevice* device, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples, VkImageAspectFlags aspect, size_t layerCount, std::string textureName) : m_Device(device)
{
	const VkDevice vkDevice = device->GetVkDevice();

	stbi_uc* textureData = stbi_load(textureName.c_str(), &m_TextureWidth, &m_TextureHeight, &m_TextureChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = m_TextureWidth * m_TextureHeight * 4;

	if (!textureData)
	{
		utils::ThrowError(EError::ImageFailure);
	}

	CreateTextureSampler(vkDevice);

	CreateImage(layerCount, format, usage, samples, vkDevice, device, aspect);
}

void TextureBuffer::CreateImage(const size_t& layerCount, VkFormat format, const VkImageUsageFlags& usage, VkSampleCountFlagBits samples, const VkDevice& vkDevice, const VulkanDevice* device, const VkImageAspectFlags& aspect)
{
	// Create an image
	VkImageCreateInfo imageCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width = m_TextureWidth;
	imageCreateInfo.extent.height = m_TextureHeight;
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

	uint32_t suitableMemoryTypeIndex = 0u;
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

void TextureBuffer::CreateTextureSampler(const VkDevice& vkDevice)
{
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if (vkCreateSampler(vkDevice, &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}
}

TextureBuffer::~TextureBuffer()
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
