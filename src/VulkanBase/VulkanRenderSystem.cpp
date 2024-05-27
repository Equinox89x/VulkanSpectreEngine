#include "VulkanRenderSystem.h"

#include "../Buffers/DataBuffer.h"
#include "../Misc/Utils.h"
#include "VulkanDevice.h"

#include <cstring>

VulkanRenderSystem::VulkanRenderSystem(const VulkanDevice* device, VkCommandPool commandPool, VkDescriptorPool descriptorPool, VkDescriptorSetLayout descriptorSetLayout, size_t modelCount) : m_Device(device)
{
	// Initialize the uniform buffer data
	dynamicVertexUniformData.resize(modelCount);
	for (size_t modelIndex = 0u; modelIndex < modelCount; ++modelIndex)
	{
		dynamicVertexUniformData.at(modelIndex).worldMatrix = glm::mat4(1.0f);
	}

	for (glm::mat4& viewProjectionMatrix : staticVertexUniformData.viewProjectionMatrices)
	{
		viewProjectionMatrix = glm::mat4(1.0f);
	}

	staticFragmentUniformData.time = 0.0f;

	const VkDevice vkDevice{ device->GetVkDevice() };

	// Allocate a command buffer
	VkCommandBufferAllocateInfo commandBufferAllocateInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	commandBufferAllocateInfo.commandPool = commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1u;
	if (vkAllocateCommandBuffers(vkDevice, &commandBufferAllocateInfo, &m_CommandBuffer) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}

	// Create semaphores
	VkSemaphoreCreateInfo semaphoreCreateInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	if (vkCreateSemaphore(vkDevice, &semaphoreCreateInfo, nullptr, &m_DrawableSemaphore) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}

	if (vkCreateSemaphore(vkDevice, &semaphoreCreateInfo, nullptr, &m_PresentableSemaphore) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}

	// Create a memory fence
	VkFenceCreateInfo fenceCreateInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Make sure the fence starts off signaled
	if (vkCreateFence(vkDevice, &fenceCreateInfo, nullptr, &m_BusyFence) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}

	const VkDeviceSize m_UniformBufferOffsetAlignment = device->GetUniformBufferOffsetAlignment();

	// Partition the uniform buffer data
	std::array<VkDescriptorBufferInfo, 3u> descriptorBufferInfos;

	descriptorBufferInfos.at(0u).offset = 0u;
	descriptorBufferInfos.at(0u).range = sizeof(DynamicVertexUniformData);

	descriptorBufferInfos.at(1u).offset = utils::Align(descriptorBufferInfos.at(0u).range, m_UniformBufferOffsetAlignment) * static_cast<VkDeviceSize>(modelCount);
	descriptorBufferInfos.at(1u).range = sizeof(StaticVertexUniformData);

	descriptorBufferInfos.at(2u).offset = descriptorBufferInfos.at(1u).offset + utils::Align(descriptorBufferInfos.at(1u).range, m_UniformBufferOffsetAlignment);
	descriptorBufferInfos.at(2u).range = sizeof(StaticFragmentUniformData);

	// Create an empty uniform buffer
	const VkDeviceSize uniformBufferSize = descriptorBufferInfos.at(2u).offset + descriptorBufferInfos.at(2u).range;
	m_UniformBuffer = new DataBuffer(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBufferSize);

	// Map the uniform buffer memory
	m_UniformBufferMemory = m_UniformBuffer->MapData();

	// Allocate a descriptor set
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	descriptorSetAllocateInfo.descriptorPool = descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1u;
	descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
	const VkResult result = vkAllocateDescriptorSets(vkDevice, &descriptorSetAllocateInfo, &m_DescriptorSet);
	if (result != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}

	// Associate the uniform buffer with each descriptor buffer info
	for (VkDescriptorBufferInfo& descriptorBufferInfo : descriptorBufferInfos)
	{
		descriptorBufferInfo.buffer = m_UniformBuffer->getBuffer();
	}

	// Update the descriptor sets
	std::array<VkWriteDescriptorSet, 3u> writeDescriptorSets;

	writeDescriptorSets.at(0u).sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets.at(0u).pNext = nullptr;
	writeDescriptorSets.at(0u).dstSet = m_DescriptorSet;
	writeDescriptorSets.at(0u).dstBinding = 0u;
	writeDescriptorSets.at(0u).dstArrayElement = 0u;
	writeDescriptorSets.at(0u).descriptorCount = 1u;
	writeDescriptorSets.at(0u).descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	writeDescriptorSets.at(0u).pBufferInfo = &descriptorBufferInfos.at(0u);
	writeDescriptorSets.at(0u).pImageInfo = nullptr;
	writeDescriptorSets.at(0u).pTexelBufferView = nullptr;

	writeDescriptorSets.at(1u).sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets.at(1u).pNext = nullptr;
	writeDescriptorSets.at(1u).dstSet = m_DescriptorSet;
	writeDescriptorSets.at(1u).dstBinding = 1u;
	writeDescriptorSets.at(1u).dstArrayElement = 0u;
	writeDescriptorSets.at(1u).descriptorCount = 1u;
	writeDescriptorSets.at(1u).descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSets.at(1u).pBufferInfo = &descriptorBufferInfos.at(1u);
	writeDescriptorSets.at(1u).pImageInfo = nullptr;
	writeDescriptorSets.at(1u).pTexelBufferView = nullptr;

	writeDescriptorSets.at(2u).sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets.at(2u).pNext = nullptr;
	writeDescriptorSets.at(2u).dstSet = m_DescriptorSet;
	writeDescriptorSets.at(2u).dstBinding = 2u;
	writeDescriptorSets.at(2u).dstArrayElement = 0u;
	writeDescriptorSets.at(2u).descriptorCount = 1u;
	writeDescriptorSets.at(2u).descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSets.at(2u).pBufferInfo = &descriptorBufferInfos.at(2u);
	writeDescriptorSets.at(2u).pImageInfo = nullptr;
	writeDescriptorSets.at(2u).pTexelBufferView = nullptr;

	vkUpdateDescriptorSets(vkDevice, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0u, nullptr);
}

VulkanRenderSystem::~VulkanRenderSystem()
{
	if (m_UniformBuffer)
	{
		m_UniformBuffer->UnmapData();
	}
	delete m_UniformBuffer;

	const VkDevice vkDevice{ m_Device->GetVkDevice() };
	if (vkDevice)
	{
		if (m_BusyFence)
		{
			vkDestroyFence(vkDevice, m_BusyFence, nullptr);
		}

		if (m_PresentableSemaphore)
		{
			vkDestroySemaphore(vkDevice, m_PresentableSemaphore, nullptr);
		}

		if (m_DrawableSemaphore)
		{
			vkDestroySemaphore(vkDevice, m_DrawableSemaphore, nullptr);
		}
	}
}

void VulkanRenderSystem::UpdateUniformBufferData() const
{
	if (!m_UniformBufferMemory)
	{
		return;
	}

	const VkDeviceSize m_UniformBufferOffsetAlignment = m_Device->GetUniformBufferOffsetAlignment();

	char*		 offset = static_cast<char*>(m_UniformBufferMemory);
	VkDeviceSize length = sizeof(DynamicVertexUniformData);
	for (const DynamicVertexUniformData& dynamicData : dynamicVertexUniformData)
	{
		memcpy(offset, &dynamicData, length);
		offset += utils::Align(length, m_UniformBufferOffsetAlignment);
	}

	length = sizeof(StaticVertexUniformData);
	memcpy(offset, &staticVertexUniformData, length);
	offset += utils::Align(length, m_UniformBufferOffsetAlignment);

	length = sizeof(StaticFragmentUniformData);
	memcpy(offset, &staticFragmentUniformData, length);
}