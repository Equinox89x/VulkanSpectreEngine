#pragma once
#include "VulkanDevice.h"

// std includes
#include <string>
#include <vector>

namespace Spectre
{
	struct PipelineConfig
	{
		PipelineConfig() = default;
		PipelineConfig(const PipelineConfig&) = delete;
		PipelineConfig(PipelineConfig&&) = delete;
		PipelineConfig& operator=(const PipelineConfig&) = delete;
		PipelineConfig& operator=(PipelineConfig&&) = delete;

		VkPipelineViewportStateCreateInfo viewportInfo{};
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
		VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
		VkPipelineMultisampleStateCreateInfo multisampleInfo{};
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
		std::vector<VkDynamicState> dynamicStateEnables{};
		VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class VulkanPipeline final
	{
	public:
		VulkanPipeline(VulkanDevice & device,  const std::string& vertFilePath,  const std::string fragFilePath,  const PipelineConfig& configInfo, bool is3D);
		~VulkanPipeline();
		VulkanPipeline(const VulkanPipeline&) = delete;
		VulkanPipeline(VulkanPipeline&&) = delete;
		VulkanPipeline& operator=(const VulkanPipeline&) = delete;
		VulkanPipeline& operator=(VulkanPipeline&&) = delete;

		static void DefaultPipelineConfigInfo(PipelineConfig& configInfo);
		void BindBuffers(VkCommandBuffer commandBuffer);

	private:
		VulkanDevice& m_Device;
		VkPipeline m_GraphicsPipeline;
		VkShaderModule m_VertexShader;
		VkShaderModule m_FragmentShader;
		bool m_Is3D{ true };

		static std::vector<char> ReadFile(const std::string& filePath);
		void CreateGraphicsPipeline(const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfig& configInfo);
		void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
	};
}
