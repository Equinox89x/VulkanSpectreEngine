#include "VulkanRenderSystem.h"

// std
#include <stdexcept>
#include <array>
#include <cassert>

//library
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace Spectre
{
	VulkanRenderSystem::VulkanRenderSystem(VulkanDevice& device, VkRenderPass renderPass, std::string vertexShaderFileName, std::string fragmentShaderFileName, bool is3D) :
		m_Device(device)
	{
		CreatePipelineLayout();
		CreatePipeline(renderPass, vertexShaderFileName, fragmentShaderFileName, is3D);
	}

	VulkanRenderSystem::~VulkanRenderSystem()
	{
		vkDestroyPipelineLayout(m_Device.GetDevice(), m_PipelineLayout, nullptr);
	}

	void VulkanRenderSystem::CreatePipelineLayout()
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(ShaderData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(m_Device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void VulkanRenderSystem::CreatePipeline(VkRenderPass renderPass, std::string vertexShaderFileName, std::string fragmentShaderFileName, bool is3D)
	{
		PipelineConfig pipelineConfig{};
		VulkanPipeline::DefaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = m_PipelineLayout;
		m_Pipeline = std::make_unique<VulkanPipeline>(m_Device, vertexShaderFileName, fragmentShaderFileName, pipelineConfig, is3D);
	}

	void VulkanRenderSystem::RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<GameObject>& gameObjects, const Camera& camera)
	{
		m_Pipeline->BindBuffers(commandBuffer);

		auto projectionMat = camera.GetProjectionMatrix();
		auto viewMat = camera.GetViewMatrix();
		auto projectionView = projectionMat * viewMat;

		for (auto& gameObject : gameObjects)
		{
			ShaderData push{};
			push.m_Color = gameObject.m_Color;
			push.m_Transform = projectionView * gameObject.m_Transform.Mat4();
			push.m_ProjectionTransform = projectionMat;
			push.m_ObjectTransform = gameObject.m_Transform.Mat4();

			vkCmdPushConstants(commandBuffer, m_PipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(ShaderData), &push);
			gameObject.m_Mesh->BindBuffers(commandBuffer);
			gameObject.m_Mesh->Draw(commandBuffer);
		}
	}
}
