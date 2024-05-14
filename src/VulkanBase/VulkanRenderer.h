#pragma once
#include "VulkanBase/VulkanWindow.h"
#include "VulkanBase/VulkanDevice.h"
#include "VulkanBase/VulkanSwapChain.h"

// std includes
#include <memory>
#include <vector>
#include <cassert>

namespace Spectre
{
	class VulkanRenderer final
	{
	public:

		VulkanRenderer(VulkanWindow& window, VulkanDevice& device);
		~VulkanRenderer();
		VulkanRenderer(const VulkanRenderer&) = delete;
		VulkanRenderer(VulkanRenderer&&) = delete;
		VulkanRenderer& operator=(const VulkanRenderer&) = delete;
		VulkanRenderer& operator=(VulkanRenderer&&) = delete;

		VkRenderPass GetRenderPass() const { return m_SwapChain->GetRenderPass(); }
		float GetAspectRatio() const { return m_SwapChain->GetExtentAspectRatio(); }
		bool IsFrameInProgress() const { return m_IsFrameStarted; }
		VkCommandBuffer GetCurrentCommandBuffer() const
		{
			assert(m_IsFrameStarted && "Can't get command buffer.");
			return m_CommandBuffers[m_CurrentFrameIndex];
		}

		VkCommandBuffer BeginFrame();
		void EndFrame();
		void BeginRenderPass(VkCommandBuffer commandBuffer);
		void EndRenderPass(VkCommandBuffer commandBuffer);

		int GetFrameIndex() const
		{
			assert(m_IsFrameStarted && "Frame not in progress.");
			return m_CurrentFrameIndex;
		}


	private:
		VulkanWindow& m_Window;
		VulkanDevice& m_Device;
		std::unique_ptr<VulkanSwapChain> m_SwapChain;
		std::vector<VkCommandBuffer> m_CommandBuffers;

		uint32_t m_CurrentImageIndex;
		int m_CurrentFrameIndex{ 0 };
		bool m_IsFrameStarted{false};

		void CreateCommandBuffers();
		void FreeCommandBuffers();
		void RecreateSwapChain();
	};
}
