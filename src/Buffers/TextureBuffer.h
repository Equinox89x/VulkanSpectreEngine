#pragma once

#include "../VulkanBase/VulkanDevice.h"

class TextureBuffer final
{
public:
	TextureBuffer(const VulkanDevice* m_Device, VkFormat format, VkImageUsageFlags usage, VkSampleCountFlagBits samples, VkImageAspectFlags aspect, size_t layerCount, std::string textureName);
	void CreateImage(const size_t& layerCount, VkFormat format, const VkImageUsageFlags& usage, VkSampleCountFlagBits samples, const VkDevice& vkDevice, const VulkanDevice* device, const VkImageAspectFlags& aspect);
	void CreateTextureSampler(const VkDevice& vkDevice);
	~TextureBuffer();

	VkImageView GetImageView() const { return m_ImageView; }
	VkSampler	GetImageSampler() const { return m_TextureSampler; };

private:
	const VulkanDevice* m_Device{ nullptr };
	VkImage				m_Image{ nullptr };
	VkDeviceMemory		deviceMemory{ nullptr };
	VkImageView			m_ImageView{ nullptr };
	VkSampler			m_TextureSampler;

	int		 m_TextureWidth;
	int		 m_TextureHeight;
	int		 m_TextureChannels;
};