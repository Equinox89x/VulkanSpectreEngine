#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

class VulkanDevice;

namespace Spectre
{
	struct PipelineMaterialPayload
	{
		VkBlendFactor	   srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		VkBlendFactor	   dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		VkBlendOp		   colorBlendOp = VK_BLEND_OP_ADD;
		VkBlendFactor	   srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		VkBlendFactor	   dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		VkBlendOp		   alphaBlendOp = VK_BLEND_OP_ADD;
		VkCullModeFlagBits cullMode = VkCullModeFlagBits::VK_CULL_MODE_NONE;
		VkBool32		   depthTestEnable = VK_TRUE;
		VkBool32		   depthWriteEnable = VK_TRUE;

		bool operator==(const PipelineMaterialPayload& other) const
		{
			return (srcColorBlendFactor == other.srcColorBlendFactor) && (dstColorBlendFactor == other.dstColorBlendFactor) && (colorBlendOp == other.colorBlendOp) && (srcAlphaBlendFactor == other.srcAlphaBlendFactor) && (dstAlphaBlendFactor == other.dstAlphaBlendFactor) && (alphaBlendOp == other.alphaBlendOp) && (cullMode == other.cullMode) && (depthTestEnable == other.depthTestEnable) && (depthWriteEnable == other.depthWriteEnable);
		}
	};																																																																																																				  
} // namespace Spectre

class VulkanPipeline final
{
public:
	VulkanPipeline(const VulkanDevice* device, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, const std::string& vertexFilename, const std::string& fragmentFilename, const std::vector<VkVertexInputBindingDescription>& vertexInputBindingDescriptions, const std::vector<VkVertexInputAttributeDescription>& vertexInputAttributeDescriptions, Spectre::PipelineMaterialPayload& materialPayload);
	~VulkanPipeline();

	void Bind(VkCommandBuffer m_CommandBuffer) const { vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline); };

	const std::string						GetVertShaderName() const { return m_VertShaderName; }
	const std::string						GetFragShaderName() const { return m_FragShaderName; }
	const Spectre::PipelineMaterialPayload& GetPipelineMaterialData() const { return m_PipelineData; }

private:
	const VulkanDevice* m_Device{ nullptr };
	VkPipeline			m_Pipeline{ nullptr };
	std::string			m_VertShaderName;
	std::string			m_FragShaderName;

	Spectre::PipelineMaterialPayload m_PipelineData;
};