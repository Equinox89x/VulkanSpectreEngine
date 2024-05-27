#include "DataBuffer.h"

#include "../Misc/Utils.h"

#include <sstream>

DataBuffer::DataBuffer(const VulkanDevice* device, const VkBufferUsageFlags bufferUsageFlags, const VkMemoryPropertyFlags memoryProperties, const VkDeviceSize size) : m_Device(device), size(size)
{
	const VkDevice vkDevice{ device->GetVkDevice() };

	VkBufferCreateInfo bufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = bufferUsageFlags;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (vkCreateBuffer(vkDevice, &bufferCreateInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(vkDevice, buffer, &memoryRequirements);

	uint32_t suitableMemoryTypeIndex = 0u;
	if (!utils::FindSuitableMemoryTypeIndex(device->GetVkPhysicalDevice(), memoryRequirements, memoryProperties, suitableMemoryTypeIndex))
	{
		utils::ThrowError(EError::FeatureNotSupported, "Suitable data buffer memory type");
	}

	VkMemoryAllocateInfo memoryAllocateInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = suitableMemoryTypeIndex;
	if (vkAllocateMemory(vkDevice, &memoryAllocateInfo, nullptr, &deviceMemory) != VK_SUCCESS)
	{
		std::stringstream s;
		s << memoryRequirements.size << " bytes for buffer";
		utils::ThrowError(EError::OutOfMemory, s.str());
	}

	if (vkBindBufferMemory(vkDevice, buffer, deviceMemory, 0u) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}
}

DataBuffer::~DataBuffer()
{
	const VkDevice vkDevice{ m_Device->GetVkDevice() };
	if (vkDevice)
	{
		if (deviceMemory)
		{
			vkFreeMemory(vkDevice, deviceMemory, nullptr);
		}

		if (buffer)
		{
			vkDestroyBuffer(vkDevice, buffer, nullptr);
		}
	}
}

bool DataBuffer::CopyTo(const DataBuffer& target, VkCommandBuffer commandBuffer, VkQueue queue) const
{
	VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, buffer, target.getBuffer(), 1u, &copyRegion);

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}

	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1u;
	submitInfo.pCommandBuffers = &commandBuffer;
	if (vkQueueSubmit(queue, 1u, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}

	if (vkQueueWaitIdle(queue) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}

	return true;
}

void* DataBuffer::MapData() const
{
	void* data;

	const VkResult result = vkMapMemory(m_Device->GetVkDevice(), deviceMemory, 0u, size, 0, &data);
	if (result != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}

	return data;
}