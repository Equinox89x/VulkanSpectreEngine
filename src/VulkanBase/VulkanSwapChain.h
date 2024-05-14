#pragma once

#include "VulkanDevice.h"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>

namespace Spectre {

    class VulkanSwapChain final
	{
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        VulkanSwapChain(VulkanDevice& device, VkExtent2D windowExtent);
        VulkanSwapChain(VulkanDevice& device, VkExtent2D windowExtent, std::shared_ptr<VulkanSwapChain> previous);
        ~VulkanSwapChain();
        VulkanSwapChain(const VulkanSwapChain&) = delete;
        VulkanSwapChain(VulkanSwapChain&&) = delete;
        VulkanSwapChain& operator=(const VulkanSwapChain&) = delete;
        VulkanSwapChain& operator=(VulkanSwapChain&&) = delete;

        VkFramebuffer GetFrameBuffer(int index) { return m_Framebuffers[index]; }
        VkRenderPass GetRenderPass() { return m_RenderPass; }
        VkImageView GetImageView(int index) { return m_ImageViews[index]; }
        size_t GetImageCount() { return m_Images.size(); }
        VkFormat GetImageFormat() { return m_ImageFormat; }
        VkExtent2D GetExtent() { return m_Extent; }
        uint32_t GetWidth() { return m_Extent.width; }
        uint32_t GetHeight() { return m_Extent.height; }

        float GetExtentAspectRatio() {
            return static_cast<float>(m_Extent.width) / static_cast<float>(m_Extent.height);
        }
        VkFormat FindDepthFormat();

        VkResult AcquireNextImage(uint32_t* imageIndex);
        VkResult SubmitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

        bool CompareSwapFormats(const VulkanSwapChain& swapChain) const
    	{
			return swapChain.m_DepthFormat == m_DepthFormat && swapChain.m_ImageFormat == m_ImageFormat;
		}


    private:
        VkFormat m_ImageFormat;
        VkFormat m_DepthFormat;
        VkExtent2D m_Extent;

        std::vector<VkFramebuffer> m_Framebuffers;
        VkRenderPass m_RenderPass;

        std::vector<VkImage> m_DepthImages;
        std::vector<VkDeviceMemory> m_DepthImageMemorys;
        std::vector<VkImageView> m_DepthImageViews;

        std::vector<VkImage> m_Images;
        std::vector<VkImageView> m_ImageViews;

        VulkanDevice& m_Device;
        VkExtent2D m_WindowExtent;

        VkSwapchainKHR m_SwapChain;
        std::shared_ptr<VulkanSwapChain> m_OldSwapChain;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;
        std::vector<VkFence> m_ImagesInFlight;
        size_t m_CurrentFrame = 0;

        void Init();
        void CreateSwapChain();
        void CreateImageViews();
        void CreateDepthResources();
        void CreateRenderPass();
        void CreateFramebuffers();
        void CreateSyncObjects();

        // Helper functions
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat( const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    };

}  // namespace Spectre
