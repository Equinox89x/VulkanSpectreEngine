#include "VulkanPipeline.h"

#include "../Misc/Utils.h"
#include "../VulkanBase/VulkanDevice.h"

#include <array>
#include <sstream>

VulkanPipeline::VulkanPipeline(const VulkanDevice* device, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, const std::string& vertexFilename, const std::string& fragmentFilename, const std::vector<VkVertexInputBindingDescription>& vertexInputBindingDescriptions, const std::vector<VkVertexInputAttributeDescription>& vertexInputAttributeDescriptions, Spectre::PipelineMaterialPayload& materialPayload)
	: m_Device(device), m_VertShaderName{ vertexFilename }, m_FragShaderName{ fragmentFilename }, m_PipelineData{ materialPayload }
{

	const VkDevice vkDevice{ m_Device->GetVkDevice() };
	this->m_PipelineData = materialPayload;

	// Load the vertex shader
	VkShaderModule vertexShaderModule;
	if (!utils::LoadShaderFromFile(vkDevice, vertexFilename, vertexShaderModule))
	{
		std::stringstream s;
		s << "Vertex shader \"" << vertexFilename << "\"";
		utils::ThrowError(EError::FileMissing, s.str());
	}

	// Load the fragment shader
	VkShaderModule fragmentShaderModule;
	if (!utils::LoadShaderFromFile(vkDevice, fragmentFilename, fragmentShaderModule))
	{
		std::stringstream s;
		s << "Fragment shader \"" << fragmentFilename << "\"";
		utils::ThrowError(EError::FileMissing, s.str());
	}

	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfoVertex{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	pipelineShaderStageCreateInfoVertex.module = vertexShaderModule;
	pipelineShaderStageCreateInfoVertex.stage = VK_SHADER_STAGE_VERTEX_BIT;
	pipelineShaderStageCreateInfoVertex.pName = "main";

	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfoFragment{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	pipelineShaderStageCreateInfoFragment.module = fragmentShaderModule;
	pipelineShaderStageCreateInfoFragment.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	pipelineShaderStageCreateInfoFragment.pName = "main";

	const std::array shaderStages{ pipelineShaderStageCreateInfoVertex, pipelineShaderStageCreateInfoFragment };

	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

	pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindingDescriptions.size());
	pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vertexInputBindingDescriptions.data();
	pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributeDescriptions.size());
	pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	pipelineViewportStateCreateInfo.viewportCount = 1u;
	pipelineViewportStateCreateInfo.scissorCount = 1u;

	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
	pipelineRasterizationStateCreateInfo.cullMode = m_PipelineData.cullMode;
	pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	pipelineMultisampleStateCreateInfo.rasterizationSamples = m_Device->GetMultisampleCount();

	VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
	pipelineColorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	pipelineColorBlendAttachmentState.blendEnable = VK_TRUE;
	pipelineColorBlendAttachmentState.srcColorBlendFactor = m_PipelineData.srcColorBlendFactor;
	pipelineColorBlendAttachmentState.dstColorBlendFactor = m_PipelineData.dstColorBlendFactor;
	pipelineColorBlendAttachmentState.colorBlendOp = m_PipelineData.colorBlendOp;
	pipelineColorBlendAttachmentState.srcAlphaBlendFactor = m_PipelineData.srcAlphaBlendFactor;
	pipelineColorBlendAttachmentState.dstAlphaBlendFactor = m_PipelineData.dstAlphaBlendFactor;
	pipelineColorBlendAttachmentState.alphaBlendOp = m_PipelineData.alphaBlendOp;

	pipelineColorBlendStateCreateInfo.attachmentCount = 1;
	pipelineColorBlendStateCreateInfo.pAttachments = &pipelineColorBlendAttachmentState;

	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	constexpr std::array			 dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	pipelineDynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

	VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	pipelineDepthStencilStateCreateInfo.depthTestEnable = m_PipelineData.depthTestEnable;
	pipelineDepthStencilStateCreateInfo.depthWriteEnable = m_PipelineData.depthWriteEnable;
	pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	graphicsPipelineCreateInfo.flags = VK_PIPELINE_CREATE_LINK_TIME_OPTIMIZATION_BIT_EXT;
	graphicsPipelineCreateInfo.layout = pipelineLayout;
	graphicsPipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	graphicsPipelineCreateInfo.pStages = shaderStages.data();
	graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
	graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
	graphicsPipelineCreateInfo.renderPass = renderPass;
	if (vkCreateGraphicsPipelines(vkDevice, nullptr, 1u, &graphicsPipelineCreateInfo, nullptr, &m_Pipeline) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}

	// These shader modules can now be destroyed
	vkDestroyShaderModule(vkDevice, vertexShaderModule, nullptr);
	vkDestroyShaderModule(vkDevice, fragmentShaderModule, nullptr);
}

VulkanPipeline::~VulkanPipeline()
{
	const VkDevice vkDevice{ m_Device->GetVkDevice() };
	if (vkDevice && m_Pipeline)
	{
		vkDestroyPipeline(vkDevice, m_Pipeline, nullptr);
	}
}