#pragma once
#include <array>
#include <glm/mat4x4.hpp>
#include <vector>
#include <vulkan/vulkan.h>

class VulkanDevice;
class DataBuffer;

class VulkanRenderSystem final
{
public:
	struct DynamicVertexUniformData
	{
		glm::mat4 worldMatrix;
		glm::vec4 colorMultiplier = glm::vec4(1.0f);
	};
	std::vector<DynamicVertexUniformData> dynamicVertexUniformData;

	struct StaticVertexUniformData
	{
		std::array<glm::mat4, 2u> viewProjectionMatrices; // 0 = left eye, 1 = right eye
	} staticVertexUniformData;

	struct StaticFragmentUniformData
	{
		float time;
	} staticFragmentUniformData;

	VulkanRenderSystem(const VulkanDevice* m_Device, VkCommandPool m_CommandPool, VkDescriptorPool m_DescriptorPool, VkDescriptorSetLayout m_DescriptorSetLayout, size_t modelCount);
	~VulkanRenderSystem();

	VkCommandBuffer GetCommandBuffer() const { return m_CommandBuffer; }
	VkSemaphore		GetDrawableSemaphore() const { return m_DrawableSemaphore; }
	VkSemaphore		GetPresentableSemaphore() const { return m_PresentableSemaphore; }
	VkFence			GetBusyFence() const { return m_BusyFence; }
	VkDescriptorSet GetDescriptorSet() const { return m_DescriptorSet; }
	void			UpdateUniformBufferData() const;

private:
	const VulkanDevice* m_Device{ nullptr };
	VkCommandBuffer		m_CommandBuffer{ nullptr };
	VkSemaphore			m_DrawableSemaphore{ nullptr }, m_PresentableSemaphore{ nullptr };
	VkFence				m_BusyFence{ nullptr };
	DataBuffer*			m_UniformBuffer{ nullptr };
	void*				m_UniformBufferMemory{ nullptr };
	VkDescriptorSet		m_DescriptorSet{ nullptr };
};