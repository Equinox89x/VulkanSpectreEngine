#include "Utils.h"

#include <glm/gtx/quaternion.hpp>

#include <cstring>
#include <fstream>
#include <sstream>

void utils::ThrowError(EError error, const std::string& details)
{
	switch (error)
	{
	case EError::FeatureNotSupported:
		throw std::runtime_error("Required feature is not supported");
		break;
	case EError::FileMissing:
		throw std::runtime_error("Failed to find file");
		break;
	case EError::GenericGLFW:
		throw std::runtime_error("Program encountered a generic GLFW error");
		break;
	case EError::GenericOpenXR:
		throw std::runtime_error("Program encountered a generic OpenXR error");
		break;
	case EError::GenericVulkan:
		throw std::runtime_error("Program encountered a generic Vulkan error");
		break;
	case EError::HeadsetNotConnected:
		throw std::runtime_error("No headset detected.\nPlease make sure that your headset is connected and running");
		break;
	case EError::ModelLoadingFailure:
		throw std::runtime_error("Failed to load model");
		break;
	case EError::OutOfMemory:
		throw std::runtime_error("Program ran out of memory");
		break;
	case EError::VulkanNotSupported:
		throw std::runtime_error("Vulkan is not supported");
		break;
	case EError::WindowFailure:
		throw std::runtime_error("Failed to create window");
		break;
	}
}

void utils::CreateImageView(const VkImage& image, VkFormat format, const uint32_t& layerCount, const VkImageAspectFlags aspectFlags, const VkDevice& device, VkImageView& outImageView)
{
	// Create an image view
	VkImageViewCreateInfo imageViewCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	imageViewCreateInfo.image = image;
	imageViewCreateInfo.format = format;
	imageViewCreateInfo.viewType = (layerCount == 1u ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_2D_ARRAY);
	imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	imageViewCreateInfo.subresourceRange.layerCount = layerCount;
	imageViewCreateInfo.subresourceRange.aspectMask = aspectFlags;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0u;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0u;
	imageViewCreateInfo.subresourceRange.levelCount = 1u;

	if (vkCreateImageView(device, &imageViewCreateInfo, nullptr, &outImageView) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}
}

bool utils::LoadXrExtensionFunction(XrInstance instance, const std::string& name, PFN_xrVoidFunction* function)
{
	const XrResult result = xrGetInstanceProcAddr(instance, name.c_str(), function);
	if (XR_FAILED(result))
	{
		return false;
	}

	return true;
}

PFN_vkVoidFunction utils::LoadVkExtensionFunction(VkInstance instance, const std::string& name) { return vkGetInstanceProcAddr(instance, name.c_str()); }

std::vector<const char*> utils::UnpackExtensionString(const std::string& string)
{
	std::vector<const char*> out;
	std::istringstream		 stream(string);
	std::string				 extension;
	while (getline(stream, extension, ' '))
	{
		const size_t len = extension.size() + 1u;
		char*		 str = new char[len];
		memcpy(str, extension.c_str(), len);
		out.push_back(str);
	}

	return out;
}

bool utils::LoadShaderFromFile(VkDevice m_Device, const std::string& filename, VkShaderModule& shaderModule)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open())
	{
		return false;
	}

	const size_t	  fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> code(fileSize);
	file.seekg(0);
	file.read(code.data(), fileSize);
	file.close();

	VkShaderModuleCreateInfo shaderModuleCreateInfo;
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.pNext = nullptr;
	shaderModuleCreateInfo.flags = 0u;
	shaderModuleCreateInfo.codeSize = code.size();
	shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	if (vkCreateShaderModule(m_Device, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		return false;
	}

	return true;
}

bool utils::FindSuitableMemoryTypeIndex(VkPhysicalDevice m_PhysicalDevice, VkMemoryRequirements requirements, VkMemoryPropertyFlags properties, uint32_t& typeIndex)
{
	VkPhysicalDeviceMemoryProperties supportedMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &supportedMemoryProperties);

	const VkMemoryPropertyFlags typeFilter = requirements.memoryTypeBits;
	for (uint32_t memoryTypeIndex = 0u; memoryTypeIndex < supportedMemoryProperties.memoryTypeCount; ++memoryTypeIndex)
	{
		const VkMemoryPropertyFlags propertyFlags = supportedMemoryProperties.memoryTypes[memoryTypeIndex].propertyFlags;
		if (typeFilter & (1u << memoryTypeIndex) && (propertyFlags & properties) == properties)
		{
			typeIndex = memoryTypeIndex;
			return true;
		}
	}

	return false;
}

VkDeviceSize utils::Align(VkDeviceSize value, VkDeviceSize alignment)
{
	if (value == 0u)
	{
		return value;
	}

	return (value + alignment - 1u) & ~(alignment - 1u);
}

XrPath utils::StringToXrPath(XrInstance instance, const std::string& string)
{
	XrPath		   path;
	const XrResult result = xrStringToPath(instance, string.c_str(), &path);
	if (XR_FAILED(result))
	{
		return XR_NULL_PATH;
	}

	return path;
}

bool utils::CreateAction(XrActionSet actionSet, const std::vector<XrPath>& paths, const std::string& actionName, const std::string& localizedActionName, XrActionType type, XrAction& action)
{
	XrActionCreateInfo actionCreateInfo{ XR_TYPE_ACTION_CREATE_INFO };
	actionCreateInfo.actionType = type;
	actionCreateInfo.countSubactionPaths = static_cast<uint32_t>(paths.size());
	actionCreateInfo.subactionPaths = paths.data();

	memcpy(actionCreateInfo.actionName, actionName.data(), actionName.length() + 1u);
	memcpy(actionCreateInfo.localizedActionName, localizedActionName.data(), localizedActionName.length() + 1u);

	XrResult result = xrCreateAction(actionSet, &actionCreateInfo, &action);
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
		return false;
	}

	return true;
}

XrPosef utils::MakeIdentityPose()
{
	XrPosef identity;
	identity.position = { 0.0f, 0.0f, 0.0f };
	identity.orientation = { 0.0f, 0.0f, 0.0f, 1.0f };
	return identity;
}

glm::mat4 utils::ToMatrix(const XrPosef& pose)
{
	const glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(pose.position.x, pose.position.y, pose.position.z));
	const glm::mat4 rotation = glm::toMat4(glm::quat(pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z));
	return translation * rotation;
}

glm::mat4 utils::CreateProjectionMatrix(XrFovf fov, float nearClip, float farClip)
{
	const float l = glm::tan(fov.angleLeft);
	const float r = glm::tan(fov.angleRight);
	const float d = glm::tan(fov.angleDown);
	const float u = glm::tan(fov.angleUp);

	const float w = r - l;
	const float h = d - u;

	glm::mat4 projectionMatrix;
	projectionMatrix[0] = { 2.0f / w, 0.0f, 0.0f, 0.0f };
	projectionMatrix[1] = { 0.0f, 2.0f / h, 0.0f, 0.0f };
	projectionMatrix[2] = { (r + l) / w, (u + d) / h, -(farClip + nearClip) / (farClip - nearClip), -1.0f };
	projectionMatrix[3] = { 0.0f, 0.0f, -(farClip * (nearClip + nearClip)) / (farClip - nearClip), 0.0f };
	return projectionMatrix;
}

bool utils::UpdateActionStatePose(XrSession session, XrAction action, XrPath path, XrActionStatePose& outState)
{
	XrActionStateGetInfo actionStateGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
	actionStateGetInfo.action = action;
	actionStateGetInfo.subactionPath = path;

	const XrResult result = xrGetActionStatePose(session, &actionStateGetInfo, &outState);
	if (XR_FAILED(result))
	{
		return false;
	}

	return true;
}

bool utils::UpdateActionStateFloat(XrSession session, XrAction action, XrPath path, XrActionStateFloat& outState)
{
	XrActionStateGetInfo actionStateGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
	actionStateGetInfo.action = action;
	actionStateGetInfo.subactionPath = path;

	const XrResult result = xrGetActionStateFloat(session, &actionStateGetInfo, &outState);
	if (XR_FAILED(result))
	{
		return false;
	}

	return true;
}

bool utils::UpdateActionStateVector2(XrSession session, XrAction action, XrPath path, XrActionStateVector2f& outState)
{
	XrActionStateGetInfo actionStateGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
	actionStateGetInfo.action = action;
	actionStateGetInfo.subactionPath = path;

	const XrResult result = xrGetActionStateVector2f(session, &actionStateGetInfo, &outState);
	if (XR_FAILED(result))
	{
		return false;
	}

	return true;
}

bool utils::UpdateActionStateBoolean(XrSession session, XrAction action, XrPath path, XrActionStateBoolean& outState)
{
	XrActionStateGetInfo actionStateGetInfo{ XR_TYPE_ACTION_STATE_GET_INFO };
	actionStateGetInfo.action = action;
	actionStateGetInfo.subactionPath = path;

	const XrResult result = xrGetActionStateBoolean(session, &actionStateGetInfo, &outState);
	if (XR_FAILED(result))
	{
		return false;
	}

	return true;
}
