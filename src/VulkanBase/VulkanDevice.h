#pragma once

#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>
#include <vulkan/vulkan.h>

#define XR_USE_GRAPHICS_API_VULKAN
#include <openxr/openxr_platform.h>

namespace Spectre
{
	constexpr XrViewConfigurationType viewType = XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
	constexpr XrEnvironmentBlendMode  environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
} // namespace

class VulkanDevice final
{
public:
	VulkanDevice();
	~VulkanDevice();

	bool CreateXRDevice(VkSurfaceKHR mirrorSurface);

	void					Sync() const { vkDeviceWaitIdle(m_Device); }
	XrViewConfigurationType GetXrViewType() const { return Spectre::viewType; }
	XrInstance				GetXrInstance() const { return m_XrInstance; }
	XrSystemId				GetXrSystemId() const { return m_SystemId; }
	VkInstance				GetVkInstance() const { return m_VkInstance; }
	VkPhysicalDevice		GetVkPhysicalDevice() const { return m_PhysicalDevice; }
	uint32_t				GetVkDrawQueueFamilyIndex() const { return m_DrawQueueFamilyIndex; }
	VkDevice				GetVkDevice() const { return m_Device; }
	VkQueue					GetVkDrawQueue() const { return m_DrawQueue; }
	VkQueue					GetVkPresentQueue() const { return m_PresentQueue; }
	VkDeviceSize			GetUniformBufferOffsetAlignment() const { return m_UniformBufferOffsetAlignment; }
	VkSampleCountFlagBits	GetMultisampleCount() const { return m_MultisampleCount; }

private:
	// Extension function pointers
	PFN_xrGetVulkanInstanceExtensionsKHR   m_XrGetVulkanInstanceExtensionsKHR{ nullptr };
	PFN_xrGetVulkanGraphicsDeviceKHR	   m_XrGetVulkanGraphicsDeviceKHR{ nullptr };
	PFN_xrGetVulkanDeviceExtensionsKHR	   m_XrGetVulkanDeviceExtensionsKHR{ nullptr };
	PFN_xrGetVulkanGraphicsRequirementsKHR m_XrGetVulkanGraphicsRequirementsKHR{ nullptr };

	XrInstance m_XrInstance{ nullptr };
	XrSystemId m_SystemId{ 0u };

	VkInstance			  m_VkInstance{ nullptr };
	VkPhysicalDevice	  m_PhysicalDevice{ nullptr };
	uint32_t			  m_DrawQueueFamilyIndex{ 0u }, m_PresentQueueFamilyIndex{ 0u };
	VkDevice			  m_Device{ nullptr };
	VkQueue				  m_DrawQueue{ nullptr }, m_PresentQueue{ nullptr };
	VkDeviceSize		  m_UniformBufferOffsetAlignment{ 0u };
	VkSampleCountFlagBits m_MultisampleCount{ VK_SAMPLE_COUNT_1_BIT };

	void CreateVulkanInstance(std::vector<const char*>& vulkanInstanceExtensions);
	void AddOpenXRExtentions(XrResult& result, std::vector<const char*>& vulkanInstanceExtensions);
	void CreateXRInstance(std::vector<XrExtensionProperties>& supportedOpenXRInstanceExtensions);
	bool CreateDevice(std::vector<const char*>& vulkanDeviceExtensions);
	bool GetPresentQueueFamilyIndex(const VkSurfaceKHR& mirrorSurface);
	bool FindDrawQueueFamilyIndex();
	void CheckSupportedBlendMode(XrResult& result);
	bool HandleExtentionSupportCheck(std::vector<const char*>& vulkanDeviceExtensions, std::vector<VkExtensionProperties>& supportedVulkanDeviceExtensions);
};