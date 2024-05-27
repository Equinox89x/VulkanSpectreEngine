#pragma once

#include <vulkan/vulkan.h>

#include "../VulkanBase/VulkanDevice.h"

/*
 * The data buffer class is used to store Vulkan data buffers, namely the uniform buffer and the vertex/index buffer. It
 * is unrelated to Vulkan image buffers used for the depth buffer for example. Note that is good for performance to keep
 * Vulkan buffers mapped until destruction. This class offers functionality to do so, but doesn't enforce the principle.
 */
class DataBuffer final
{
public:
	DataBuffer(const VulkanDevice* m_Device, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags memoryProperties, VkDeviceSize size);
	~DataBuffer();

	bool  CopyTo(const DataBuffer& target, VkCommandBuffer m_CommandBuffer, VkQueue queue) const;
	void* MapData() const;
	void  UnmapData() const { vkUnmapMemory(m_Device->GetVkDevice(), deviceMemory); }

	VkBuffer getBuffer() const { return buffer; }

private:
	const VulkanDevice* m_Device{ nullptr };
	VkBuffer			buffer{ nullptr };
	VkDeviceMemory		deviceMemory{ nullptr };
	VkDeviceSize		size{ 0u };
};