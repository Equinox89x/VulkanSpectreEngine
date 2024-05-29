#include "VulkanDevice.h"

#include <glfw/glfw3.h>

#ifdef DEBUG
	#include <array>
#endif
#include "../Misc/Utils.h"

VulkanDevice::VulkanDevice()
{
	// Initialize GLFW
	if (!glfwInit())
	{
		utils::ThrowError(EError::GenericGLFW);
	}

	if (!glfwVulkanSupported())
	{
		utils::ThrowError(EError::VulkanNotSupported);
	}

	// Get all supported OpenXR instance extensions
	std::vector<XrExtensionProperties> supportedOpenXRInstanceExtensions;
	{
		uint32_t instanceExtensionCount;
		XrResult result = xrEnumerateInstanceExtensionProperties(nullptr, 0u, &instanceExtensionCount, nullptr);
		if (XR_FAILED(result))
		{
			utils::ThrowError(EError::GenericOpenXR);
		}

		supportedOpenXRInstanceExtensions.resize(instanceExtensionCount);
		for (XrExtensionProperties& extensionProperty : supportedOpenXRInstanceExtensions)
		{
			extensionProperty.type = XR_TYPE_EXTENSION_PROPERTIES;
			extensionProperty.next = nullptr;
		}

		result = xrEnumerateInstanceExtensionProperties(nullptr, instanceExtensionCount, &instanceExtensionCount, supportedOpenXRInstanceExtensions.data());
		if (XR_FAILED(result))
		{
			utils::ThrowError(EError::GenericOpenXR);
		}
	}

	// Create an OpenXR instance
	CreateXRInstance(supportedOpenXRInstanceExtensions);

	// Load the required OpenXR extension functions
	if (!utils::LoadXrExtensionFunction(m_XrInstance, "xrGetVulkanInstanceExtensionsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&m_XrGetVulkanInstanceExtensionsKHR)))
	{
		utils::ThrowError(EError::FeatureNotSupported, "OpenXR extension function \"xrGetVulkanInstanceExtensionsKHR\"");
	}

	if (!utils::LoadXrExtensionFunction(m_XrInstance, "xrGetVulkanGraphicsDeviceKHR", reinterpret_cast<PFN_xrVoidFunction*>(&m_XrGetVulkanGraphicsDeviceKHR)))
	{
		utils::ThrowError(EError::FeatureNotSupported, "OpenXR extension function \"xrGetVulkanGraphicsDeviceKHR\"");
	}

	if (!utils::LoadXrExtensionFunction(m_XrInstance, "xrGetVulkanDeviceExtensionsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&m_XrGetVulkanDeviceExtensionsKHR)))
	{
		utils::ThrowError(EError::FeatureNotSupported, "OpenXR extension function \"xrGetVulkanDeviceExtensionsKHR\"");
	}

	if (!utils::LoadXrExtensionFunction(m_XrInstance, "xrGetVulkanGraphicsRequirementsKHR", reinterpret_cast<PFN_xrVoidFunction*>(&m_XrGetVulkanGraphicsRequirementsKHR)))
	{
		utils::ThrowError(EError::FeatureNotSupported, "OpenXR extension function \"xrGetVulkanGraphicsRequirementsKHR\"");
	}

	// Get the system ID
	XrSystemGetInfo systemGetInfo{ XR_TYPE_SYSTEM_GET_INFO };
	systemGetInfo.formFactor = XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
	XrResult result = xrGetSystem(m_XrInstance, &systemGetInfo, &m_SystemId);
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::HeadsetNotConnected);
	}

	// Check the supported environment blend modes
	CheckSupportedBlendMode(result);

	// Get all supported Vulkan instance extensions
	std::vector<VkExtensionProperties> supportedVulkanInstanceExtensions;
	{
		uint32_t instanceExtensionCount;
		if (vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr) != VK_SUCCESS)
		{
			utils::ThrowError(EError::GenericVulkan);
		}

		supportedVulkanInstanceExtensions.resize(instanceExtensionCount);
		if (vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, supportedVulkanInstanceExtensions.data()) != VK_SUCCESS)
		{
			utils::ThrowError(EError::GenericVulkan);
		}
	}

	// Get the required Vulkan instance extensions from GLFW
	std::vector<const char*> vulkanInstanceExtensions;
	{
		uint32_t	 requiredExtensionCount;
		const char** buffer{ glfwGetRequiredInstanceExtensions(&requiredExtensionCount) };
		if (!buffer)
		{
			utils::ThrowError(EError::GenericGLFW);
		}

		for (uint32_t extensionIndex = 0u; extensionIndex < requiredExtensionCount; ++extensionIndex)
		{
			vulkanInstanceExtensions.push_back(buffer[extensionIndex]);
		}
	}

	// Get the required Vulkan instance extensions from OpenXR and add them
	AddOpenXRExtentions(result, vulkanInstanceExtensions);

	// Check that all required Vulkan instance extensions are supported
	for (const char* extension : vulkanInstanceExtensions)
	{
		bool extensionSupported{ false };

		for (const VkExtensionProperties& supportedExtension : supportedVulkanInstanceExtensions)
		{
			if (strcmp(extension, supportedExtension.extensionName) == 0)
			{
				extensionSupported = true;
				break;
			}
		}

		if (!extensionSupported)
		{
			std::stringstream s;
			s << "Vulkan instance extension \"" << extension << "\"";
			utils::ThrowError(EError::FeatureNotSupported, s.str());
		}
	}

	// Create a Vulkan instance with all required extensions
	CreateVulkanInstance(vulkanInstanceExtensions);
}

VulkanDevice::~VulkanDevice()
{
	// Clean up OpenXR
	if (m_XrInstance)
	{
		xrDestroyInstance(m_XrInstance);
	}

	// Clean up Vulkan
	if (m_Device)
	{
		vkDestroyDevice(m_Device, nullptr);
	}

	if (m_VkInstance)
	{
		vkDestroyInstance(m_VkInstance, nullptr);
	}
}

void VulkanDevice::CheckSupportedBlendMode(XrResult& result)
{

	uint32_t environmentBlendModeCount;
	result = xrEnumerateEnvironmentBlendModes(m_XrInstance, m_SystemId, Spectre::viewType, 0u, &environmentBlendModeCount, nullptr);
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
	}

	std::vector<XrEnvironmentBlendMode> supportedEnvironmentBlendModes(environmentBlendModeCount);
	result = xrEnumerateEnvironmentBlendModes(m_XrInstance, m_SystemId, Spectre::viewType, environmentBlendModeCount, &environmentBlendModeCount, supportedEnvironmentBlendModes.data());
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
	}

	bool modeFound{ false };
	for (const XrEnvironmentBlendMode& mode : supportedEnvironmentBlendModes)
	{
		if (mode == Spectre::environmentBlendMode)
		{
			modeFound = true;
			break;
		}
	}

	if (!modeFound)
	{
		utils::ThrowError(EError::FeatureNotSupported, "Environment blend mode");
	}
}

void VulkanDevice::CreateVulkanInstance(std::vector<const char*>& vulkanInstanceExtensions)
{

	VkApplicationInfo applicationInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
	applicationInfo.apiVersion = VK_API_VERSION_1_3;
	applicationInfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
	applicationInfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
	applicationInfo.pApplicationName = Spectre::applicationName.c_str();
	applicationInfo.pEngineName = Spectre::engineName.c_str();

	VkInstanceCreateInfo instanceCreateInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(vulkanInstanceExtensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = vulkanInstanceExtensions.data();

	if (vkCreateInstance(&instanceCreateInfo, nullptr, &m_VkInstance) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}
}

void VulkanDevice::AddOpenXRExtentions(XrResult& result, std::vector<const char*>& vulkanInstanceExtensions)
{

	uint32_t count;
	result = m_XrGetVulkanInstanceExtensionsKHR(m_XrInstance, m_SystemId, 0u, &count, nullptr);
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
	}

	std::string buffer;
	buffer.resize(count);
	result = m_XrGetVulkanInstanceExtensionsKHR(m_XrInstance, m_SystemId, count, &count, buffer.data());
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
	}

	const std::vector<const char*> instanceExtensions{ utils::UnpackExtensionString(buffer) };
	for (const char* extension : instanceExtensions)
	{
		vulkanInstanceExtensions.push_back(extension);
	}
}

void VulkanDevice::CreateXRInstance(std::vector<XrExtensionProperties>& supportedOpenXRInstanceExtensions)
{
	XrApplicationInfo applicationInfo;
	applicationInfo.apiVersion = XR_CURRENT_API_VERSION;
	applicationInfo.applicationVersion = static_cast<uint32_t>(XR_MAKE_VERSION(0, 1, 0));
	applicationInfo.engineVersion = static_cast<uint32_t>(XR_MAKE_VERSION(0, 1, 0));

	memcpy(applicationInfo.applicationName, Spectre::applicationName.data(), Spectre::applicationName.length() + 1u);
	memcpy(applicationInfo.engineName, Spectre::engineName.data(), Spectre::engineName.length() + 1u);

	// https://github.khronos.org/OpenXR-Inventory/extension_support.html#meta_pc
	std::vector<const char*> extensions{ XR_KHR_VULKAN_ENABLE_EXTENSION_NAME, "XR_META_touch_controller_plus", "XR_KHR_vulkan_enable2" };

	// Check that all OpenXR instance extensions are supported
	for (const char* extension : extensions)
	{
		bool extensionSupported{ false };
		for (const XrExtensionProperties& supportedExtension : supportedOpenXRInstanceExtensions)
		{
			if (strcmp(extension, supportedExtension.extensionName) == 0)
			{
				extensionSupported = true;
				break;
			}
		}

		if (!extensionSupported)
		{
			std::stringstream s;
			s << "OpenXR instance extension \"" << extension << "\"";
			utils::ThrowError(EError::FeatureNotSupported, s.str());
		}
	}

	XrInstanceCreateInfo instanceCreateInfo{ XR_TYPE_INSTANCE_CREATE_INFO };
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instanceCreateInfo.enabledExtensionNames = extensions.data();
	instanceCreateInfo.applicationInfo = applicationInfo;
	const XrResult result = xrCreateInstance(&instanceCreateInfo, &m_XrInstance);
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::HeadsetNotConnected);
	}
}

bool VulkanDevice::CreateXRDevice(VkSurfaceKHR mirrorSurface)
{
	// Retrieve the physical device from OpenXR
	XrResult result{ m_XrGetVulkanGraphicsDeviceKHR(m_XrInstance, m_SystemId, m_VkInstance, &m_PhysicalDevice) };
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
		return false;
	}

	// Pick the draw queue family index
	FindDrawQueueFamilyIndex();

	// Pick the present queue family index
	GetPresentQueueFamilyIndex(mirrorSurface);

	// Get all supported Vulkan device extensions
	std::vector<VkExtensionProperties> supportedVulkanDeviceExtensions;
	{
		uint32_t deviceExtensionCount;
		if (vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &deviceExtensionCount, nullptr) != VK_SUCCESS)
		{
			utils::ThrowError(EError::GenericVulkan);
			return false;
		}

		supportedVulkanDeviceExtensions.resize(deviceExtensionCount);
		if (vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &deviceExtensionCount, supportedVulkanDeviceExtensions.data()) != VK_SUCCESS)
		{
			utils::ThrowError(EError::GenericVulkan);
			return false;
		}
	}

	// Get the required Vulkan device extensions from OpenXR
	std::vector<const char*> vulkanDeviceExtensions;
	{
		uint32_t count;
		result = m_XrGetVulkanDeviceExtensionsKHR(m_XrInstance, m_SystemId, 0u, &count, nullptr);
		if (XR_FAILED(result))
		{
			utils::ThrowError(EError::GenericOpenXR);
			return false;
		}

		std::string buffer;
		buffer.resize(count);
		result = m_XrGetVulkanDeviceExtensionsKHR(m_XrInstance, m_SystemId, count, &count, buffer.data());
		if (XR_FAILED(result))
		{
			utils::ThrowError(EError::GenericOpenXR);
			return false;
		}

		vulkanDeviceExtensions = utils::UnpackExtensionString(buffer);
	}

	// Add the required swapchain extension for mirror view
	vulkanDeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	// Check that all Vulkan device extensions are supported
	HandleExtentionSupportCheck(vulkanDeviceExtensions, supportedVulkanDeviceExtensions);

	// Create a device
	CreateDevice(vulkanDeviceExtensions);

	// Check the graphics requirements for Vulkan
	XrGraphicsRequirementsVulkanKHR graphicsRequirements{ XR_TYPE_GRAPHICS_REQUIREMENTS_VULKAN_KHR };
	result = m_XrGetVulkanGraphicsRequirementsKHR(m_XrInstance, m_SystemId, &graphicsRequirements);
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
		return false;
	}

	// Retrieve the queues
	vkGetDeviceQueue(m_Device, m_DrawQueueFamilyIndex, 0u, &m_DrawQueue);
	if (!m_DrawQueue)
	{
		utils::ThrowError(EError::GenericVulkan);
		return false;
	}

	vkGetDeviceQueue(m_Device, m_PresentQueueFamilyIndex, 0u, &m_PresentQueue);
	if (!m_PresentQueue)
	{
		utils::ThrowError(EError::GenericVulkan);
		return false;
	}

	return true;
}

bool VulkanDevice::HandleExtentionSupportCheck(std::vector<const char*>& vulkanDeviceExtensions, std::vector<VkExtensionProperties>& supportedVulkanDeviceExtensions)
{

	for (const char* extension : vulkanDeviceExtensions)
	{
		bool extensionSupported{ false };
		for (const VkExtensionProperties& supportedExtension : supportedVulkanDeviceExtensions)
		{
			if (strcmp(extension, supportedExtension.extensionName) == 0)
			{
				extensionSupported = true;
				break;
			}
		}

		if (!extensionSupported)
		{
			std::stringstream s;
			s << "Vulkan device extension \"" << extension << "\"";
			utils::ThrowError(EError::FeatureNotSupported, s.str());
			return false;
		}
	}
	return true;
}

bool VulkanDevice::CreateDevice(std::vector<const char*>& vulkanDeviceExtensions)
{

	// Retrieve the physical device properties
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(m_PhysicalDevice, &physicalDeviceProperties);
	m_UniformBufferOffsetAlignment = physicalDeviceProperties.limits.minUniformBufferOffsetAlignment;

	// Determine the best supported multisample count, up to 4x MSAA
	const VkSampleCountFlags sampleCountFlags = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
	if (sampleCountFlags & VK_SAMPLE_COUNT_4_BIT)
	{
		m_MultisampleCount = VK_SAMPLE_COUNT_4_BIT;
	}
	else if (sampleCountFlags & VK_SAMPLE_COUNT_2_BIT)
	{
		m_MultisampleCount = VK_SAMPLE_COUNT_2_BIT;
	}

	// Verify that the required physical device features are supported
	VkPhysicalDeviceFeatures physicalDeviceFeatures;
	vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &physicalDeviceFeatures);
	if (!physicalDeviceFeatures.shaderStorageImageMultisample)
	{
		utils::ThrowError(EError::FeatureNotSupported, "Vulkan physical device feature \"shaderStorageImageMultisample\"");
		return false;
	}

	VkPhysicalDeviceFeatures2		  physicalDeviceFeatures2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	VkPhysicalDeviceMultiviewFeatures physicalDeviceMultiviewFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES };
	physicalDeviceFeatures2.pNext = &physicalDeviceMultiviewFeatures;
	vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &physicalDeviceFeatures2);
	if (!physicalDeviceMultiviewFeatures.multiview)
	{
		utils::ThrowError(EError::FeatureNotSupported, "Vulkan physical device feature \"multiview\"");
		return false;
	}

	physicalDeviceFeatures.shaderStorageImageMultisample = VK_TRUE; // Needed for some OpenXR implementations
	physicalDeviceMultiviewFeatures.multiview = VK_TRUE;			// Needed for stereo rendering

	constexpr float queuePriority = 1.0f;

	std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;

	VkDeviceQueueCreateInfo deviceQueueCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	deviceQueueCreateInfo.queueFamilyIndex = m_DrawQueueFamilyIndex;
	deviceQueueCreateInfo.queueCount = 1u;
	deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
	deviceQueueCreateInfos.push_back(deviceQueueCreateInfo);

	if (m_DrawQueueFamilyIndex != m_PresentQueueFamilyIndex)
	{
		deviceQueueCreateInfo.queueFamilyIndex = m_PresentQueueFamilyIndex;
		deviceQueueCreateInfos.push_back(deviceQueueCreateInfo);
	}

	VkDeviceCreateInfo deviceCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	deviceCreateInfo.pNext = &physicalDeviceMultiviewFeatures;
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(vulkanDeviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = vulkanDeviceExtensions.data();
	deviceCreateInfo.pEnabledFeatures = &physicalDeviceFeatures;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
	if (vkCreateDevice(m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
		return false;
	}

	return true;
}

bool VulkanDevice::GetPresentQueueFamilyIndex(const VkSurfaceKHR& mirrorSurface)
{

	// Retrieve the queue families
	std::vector<VkQueueFamilyProperties> queueFamilies;
	uint32_t							 queueFamilyCount{ 0u };
	vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);

	queueFamilies.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

	bool presentQueueFamilyIndexFound{ false };
	for (size_t queueFamilyIndexCandidate = 0u; queueFamilyIndexCandidate < queueFamilies.size(); ++queueFamilyIndexCandidate)
	{
		const VkQueueFamilyProperties& queueFamilyCandidate = queueFamilies.at(queueFamilyIndexCandidate);

		// Check that the queue family includes actual queues
		if (queueFamilyCandidate.queueCount == 0u)
		{
			continue;
		}

		// Check the queue family for presenting support
		VkBool32 presentSupport = false;
		if (vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, static_cast<uint32_t>(queueFamilyIndexCandidate), mirrorSurface, &presentSupport) != VK_SUCCESS)
		{
			continue;
		}

		if (!presentQueueFamilyIndexFound && presentSupport)
		{
			m_PresentQueueFamilyIndex = static_cast<uint32_t>(queueFamilyIndexCandidate);
			presentQueueFamilyIndexFound = true;
			break;
		}
	}

	if (!presentQueueFamilyIndexFound)
	{
		utils::ThrowError(EError::FeatureNotSupported, "Present queue family index");
		return false;
	}
	return true;
}

bool VulkanDevice::FindDrawQueueFamilyIndex()
{

	// Retrieve the queue families
	std::vector<VkQueueFamilyProperties> queueFamilies;
	uint32_t							 queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, nullptr);

	queueFamilies.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyCount, queueFamilies.data());

	bool drawQueueFamilyIndexFound = false;
	for (size_t queueFamilyIndexCandidate = 0u; queueFamilyIndexCandidate < queueFamilies.size(); ++queueFamilyIndexCandidate)
	{
		const VkQueueFamilyProperties& queueFamilyCandidate = queueFamilies.at(queueFamilyIndexCandidate);

		// Check that the queue family includes actual queues
		if (queueFamilyCandidate.queueCount == 0u)
		{
			continue;
		}

		// Check the queue family for drawing support
		if (queueFamilyCandidate.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			m_DrawQueueFamilyIndex = static_cast<uint32_t>(queueFamilyIndexCandidate);
			drawQueueFamilyIndexFound = true;
			break;
		}
	}

	if (!drawQueueFamilyIndexFound)
	{
		utils::ThrowError(EError::FeatureNotSupported, "Graphics queue family index");
		return false;
	}
	return true;
}