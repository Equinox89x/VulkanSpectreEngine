#pragma once

#include <glm/fwd.hpp>

#include "VulkanPipeline.h"
#include "VulkanRenderSystem.h"
#include <array>
#include <vector>
#include <vulkan/vulkan.h>

class VulkanDevice;
class DataBuffer;
class Headset;
class MeshData;
struct GameObject;
struct Material;
// class VulkanPipeline;

class VulkanRenderer final
{
public:
	VulkanRenderer(){};
	VulkanRenderer(const VulkanDevice* m_Device, const Headset* m_Headset, const MeshData* meshData, const std::vector<Material*>& materials, const std::vector<GameObject*>& gameObjects);
	~VulkanRenderer();

	void Render(const glm::mat4& cameraMatrix, size_t swapchainImageIndex, float time, glm::vec3 lightDirection);
	void submit(bool useSemaphores) const;

	VkCommandBuffer GetCurrentCommandBuffer() const { return m_RenderProcesses.at(m_CurrentRenderProcessIndex)->GetCommandBuffer(); }
	VkSemaphore		GetCurrentDrawableSemaphore() const { return m_RenderProcesses.at(m_CurrentRenderProcessIndex)->GetDrawableSemaphore(); }
	VkSemaphore		GetCurrentPresentableSemaphore() const { return m_RenderProcesses.at(m_CurrentRenderProcessIndex)->GetPresentableSemaphore(); }

private:
	const VulkanDevice* m_Device{ nullptr };
	const Headset*		m_Headset{ nullptr };

	VkCommandPool		  m_CommandPool{ nullptr };
	VkDescriptorPool	  m_DescriptorPool{ nullptr };
	VkDescriptorSetLayout m_DescriptorSetLayout{ nullptr };

	std::vector<VulkanRenderSystem*> m_RenderProcesses;
	VkPipelineLayout				 m_PipelineLayout{ nullptr };
	DataBuffer*						 m_VertexIndexBuffer{ nullptr };

	// VulkanPipeline *m_GridPipeline{ nullptr }, *m_DiffusePipeline{ nullptr }, *m_2DPipeline{ nullptr };
	std::vector<VulkanPipeline*> m_Pipelines;

	std::vector<GameObject*> m_GameObjects;
	std::vector<Material*>	 m_Materials;

	size_t m_IndexOffset{ 0u };
	size_t m_CurrentRenderProcessIndex{ 0u };

	void			CreateVertexIndexBuffer(const MeshData* meshData, const VulkanDevice* m_Device);
	void			DrawModels(VulkanRenderSystem* renderProcess, const VkCommandBuffer& commandBuffer);
	void			UpdateUniformBuffers(VulkanRenderSystem* renderProcess, const glm::mat4& cameraMatrix);
	VulkanPipeline* FindExistingPipeline(const std::string& vertShader, const std::string& fragShader, const Spectre::PipelineMaterialPayload& pipelineData);
};