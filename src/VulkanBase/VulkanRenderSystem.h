#pragma once
#include "VulkanPipeline.h"
#include "VulkanDevice.h"
#include "Scene/GameObject.h"
#include "Camera/Camera.h"

// std includes
#include <memory>
#include <vector>

namespace Spectre
{
	struct ShaderData
	{
		glm::mat4 m_Transform{ 1.f };
		glm::mat4 m_ProjectionTransform{ 1.f };
		glm::mat4 m_ObjectTransform{ 1.f };
		alignas(16) glm::vec3 m_Color;
	};

	class VulkanRenderSystem final
	{
	public:
		VulkanRenderSystem(VulkanDevice& device, VkRenderPass renderPass, std::string vertexShaderFileName, std::string fragmentShaderFileName, bool is3D);
		~VulkanRenderSystem();
		VulkanRenderSystem(const VulkanRenderSystem&) = delete;
		VulkanRenderSystem(VulkanRenderSystem&&) = delete;
		VulkanRenderSystem& operator=(const VulkanRenderSystem&) = delete;
		VulkanRenderSystem& operator=(VulkanRenderSystem&&) = delete;

		void RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects, const Camera& camera);

	private:
		VulkanDevice& m_Device;
		std::unique_ptr<VulkanPipeline> m_Pipeline;
		VkPipelineLayout m_PipelineLayout;

		void CreatePipelineLayout();
		void CreatePipeline(VkRenderPass renderPass, std::string vertexShaderFileName, std::string fragmentShaderFileName, bool is3D);
	};
}
