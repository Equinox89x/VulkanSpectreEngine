#pragma once

#include <glm/fwd.hpp>

#include <vulkan/vulkan.h>

#include <functional>
#include <openxr/openxr.h>
#include <string>
#include <vector>

namespace Spectre
{
	const std::string applicationName = "GP2 Exam";
	const std::string engineName = " Spectre Engine";
} // namespace Spectre

// All the things that can go wrong
enum class EError
{
	FeatureNotSupported,
	FileMissing,
	GenericGLFW,
	GenericOpenXR,
	GenericVulkan,
	HeadsetNotConnected,
	ModelLoadingFailure,
	OutOfMemory,
	VulkanNotSupported,
	WindowFailure
};

namespace utils
{
	// Reports an error with optional details through a system-native message box
	void					 ThrowError(EError error, const std::string& details = "");
	void					 CreateImageView(const VkImage& image, VkFormat format, const uint32_t& layerCount, const VkImageAspectFlags aspectFlags, const VkDevice& device, VkImageView& outImageView);
	bool					 LoadXrExtensionFunction(XrInstance instance, const std::string& name, PFN_xrVoidFunction* function);
	PFN_vkVoidFunction		 LoadVkExtensionFunction(VkInstance instance, const std::string& name);
	std::vector<const char*> UnpackExtensionString(const std::string& string);
	bool					 LoadShaderFromFile(VkDevice m_Device, const std::string& filename, VkShaderModule& shaderModule);
	bool					 FindSuitableMemoryTypeIndex(VkPhysicalDevice m_PhysicalDevice, VkMemoryRequirements requirements, VkMemoryPropertyFlags properties, uint32_t& typeIndex);

	// Aligns a value to an alignment
	VkDeviceSize Align(VkDeviceSize value, VkDeviceSize alignment);
	XrPath		 StringToXrPath(XrInstance instance, const std::string& string);

	// Creates an OpenXR action with a given names, returns false on error
	bool	CreateAction(XrActionSet actionSet, const std::vector<XrPath>& paths, const std::string& actionName, const std::string& localizedActionName, XrActionType type, XrAction& action);
	XrPosef MakeIdentityPose();

	// Converts an OpenXR pose to a transformation matrix
	glm::mat4 ToMatrix(const XrPosef& pose);
	glm::mat4 CreateProjectionMatrix(XrFovf fov, float nearClip, float farClip);

	bool UpdateActionStatePose(XrSession session, XrAction action, XrPath path, XrActionStatePose& outState);
	bool UpdateActionStateFloat(XrSession session, XrAction action, XrPath path, XrActionStateFloat& outState);	
	bool UpdateActionStateVector2(XrSession session, XrAction action, XrPath path, XrActionStateVector2f& outState);
	bool UpdateActionStateBoolean(XrSession session, XrAction action, XrPath path, XrActionStateBoolean& outState);

} // namespace utils