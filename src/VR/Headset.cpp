#include "Headset.h"

#include "../Buffers/ImageBuffer.h"
#include "../Misc/Utils.h"
#include "../VulkanBase/RenderTarget.h"
#include "../VulkanBase/VulkanDevice.h"

#include <glm/mat4x4.hpp>

#include <array>

namespace
{
	constexpr XrReferenceSpaceType spaceType = XR_REFERENCE_SPACE_TYPE_STAGE;
	constexpr VkFormat			   colorFormat = VK_FORMAT_R8G8B8A8_SRGB;
	constexpr VkFormat			   depthFormat = VK_FORMAT_D32_SFLOAT;
} // namespace

Headset::Headset(const VulkanDevice* device) : m_Device(device)
{
	const VkDevice				vkDevice = device->GetVkDevice();
	const VkSampleCountFlagBits m_MultisampleCount = device->GetMultisampleCount();

	CreateRenderPass(m_MultisampleCount, vkDevice);

	const XrInstance	   m_XrInstance = device->GetXrInstance();
	const XrSystemId	   xrSystemId = device->GetXrSystemId();
	const VkPhysicalDevice vkPhysicalDevice = device->GetVkPhysicalDevice();
	const uint32_t		   vkDrawQueueFamilyIndex = device->GetVkDrawQueueFamilyIndex();

	// Create a session with Vulkan graphics binding
	XrGraphicsBindingVulkanKHR graphicsBinding{ XR_TYPE_GRAPHICS_BINDING_VULKAN_KHR };
	graphicsBinding.device = vkDevice;
	graphicsBinding.instance = device->GetVkInstance();
	graphicsBinding.physicalDevice = vkPhysicalDevice;
	graphicsBinding.queueFamilyIndex = vkDrawQueueFamilyIndex;
	graphicsBinding.queueIndex = 0u;

	XrSessionCreateInfo sessionCreateInfo{ XR_TYPE_SESSION_CREATE_INFO };
	sessionCreateInfo.next = &graphicsBinding;
	sessionCreateInfo.systemId = xrSystemId;
	XrResult result = xrCreateSession(m_XrInstance, &sessionCreateInfo, &m_Session);
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
	}

	// Create a play space
	XrReferenceSpaceCreateInfo referenceSpaceCreateInfo{ XR_TYPE_REFERENCE_SPACE_CREATE_INFO };
	referenceSpaceCreateInfo.referenceSpaceType = spaceType;
	referenceSpaceCreateInfo.poseInReferenceSpace = utils::MakeIdentityPose();
	result = xrCreateReferenceSpace(m_Session, &referenceSpaceCreateInfo, &m_Space);
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
	}

	const XrViewConfigurationType viewType = device->GetXrViewType();

	// Get the number of eyes
	result = xrEnumerateViewConfigurationViews(m_XrInstance, xrSystemId, viewType, 0u, reinterpret_cast<uint32_t*>(&m_EyeCount), nullptr);
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
	}

	// Get the eye image info per eye
	m_EyeImageInfos.resize(m_EyeCount);
	for (XrViewConfigurationView& eyeInfo : m_EyeImageInfos)
	{
		eyeInfo.type = XR_TYPE_VIEW_CONFIGURATION_VIEW;
		eyeInfo.next = nullptr;
	}

	result = xrEnumerateViewConfigurationViews(m_XrInstance, xrSystemId, viewType, static_cast<uint32_t>(m_EyeImageInfos.size()), reinterpret_cast<uint32_t*>(&m_EyeCount), m_EyeImageInfos.data());
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
	}

	// Allocate the eye poses
	m_EyePoses.resize(m_EyeCount);
	for (XrView& eyePose : m_EyePoses)
	{
		eyePose.type = XR_TYPE_VIEW;
		eyePose.next = nullptr;
	}

	// Verify that the desired color format is supported
	VerifyColorFormatSupport(result);

	const VkExtent2D eyeResolution = GetEyeResolution(0u);

	m_ColorBuffer = new ImageBuffer(device, eyeResolution, colorFormat, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, device->GetMultisampleCount(), VK_IMAGE_ASPECT_COLOR_BIT, 2u);
	m_DepthBuffer = new ImageBuffer(device, eyeResolution, depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, device->GetMultisampleCount(), VK_IMAGE_ASPECT_DEPTH_BIT, 2u);

	// Create a swapchain and render targets
	CreateSwapChain(result, vkDevice, eyeResolution);

	// Create the eye render infos
	m_EyeRenderInfos.resize(m_EyeCount);
	for (size_t eyeIndex = 0u; eyeIndex < m_EyeRenderInfos.size(); ++eyeIndex)
	{
		XrCompositionLayerProjectionView& eyeRenderInfo{ m_EyeRenderInfos.at(eyeIndex) };
		eyeRenderInfo.type = XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW;
		eyeRenderInfo.next = nullptr;

		// Associate this eye with the swapchain
		const XrViewConfigurationView& eyeImageInfo{ m_EyeImageInfos.at(eyeIndex) };
		eyeRenderInfo.subImage.swapchain = m_Swapchain;
		eyeRenderInfo.subImage.imageArrayIndex = static_cast<uint32_t>(eyeIndex);
		eyeRenderInfo.subImage.imageRect.offset = { 0, 0 };
		eyeRenderInfo.subImage.imageRect.extent = { static_cast<int32_t>(eyeImageInfo.recommendedImageRectWidth), static_cast<int32_t>(eyeImageInfo.recommendedImageRectHeight) };
	}

	// Allocate view and projection matrices
	m_EyeViewMatrices.resize(m_EyeCount);
	m_EyeProjectionMatrices.resize(m_EyeCount);
}

void Headset::CreateSwapChain(XrResult& result, const VkDevice& vkDevice, const VkExtent2D& eyeResolution)
{

	const XrViewConfigurationView& eyeImageInfo{ m_EyeImageInfos.at(0u) };

	// Create a swapchain
	XrSwapchainCreateInfo swapchainCreateInfo{ XR_TYPE_SWAPCHAIN_CREATE_INFO };
	swapchainCreateInfo.format = colorFormat;
	swapchainCreateInfo.sampleCount = eyeImageInfo.recommendedSwapchainSampleCount;
	swapchainCreateInfo.width = eyeImageInfo.recommendedImageRectWidth;
	swapchainCreateInfo.height = eyeImageInfo.recommendedImageRectHeight;
	swapchainCreateInfo.arraySize = static_cast<uint32_t>(m_EyeCount);
	swapchainCreateInfo.faceCount = 1u;
	swapchainCreateInfo.mipCount = 1u;

	result = xrCreateSwapchain(m_Session, &swapchainCreateInfo, &m_Swapchain);
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
	}

	// Get the number of swapchain images
	uint32_t swapchainImageCount;
	result = xrEnumerateSwapchainImages(m_Swapchain, 0u, &swapchainImageCount, nullptr);
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
	}

	// Retrieve the swapchain images
	std::vector<XrSwapchainImageVulkanKHR> swapchainImages;
	swapchainImages.resize(swapchainImageCount);
	for (XrSwapchainImageVulkanKHR& swapchainImage : swapchainImages)
	{
		swapchainImage.type = XR_TYPE_SWAPCHAIN_IMAGE_VULKAN_KHR;
	}

	XrSwapchainImageBaseHeader* data{ reinterpret_cast<XrSwapchainImageBaseHeader*>(swapchainImages.data()) };
	result = xrEnumerateSwapchainImages(m_Swapchain, static_cast<uint32_t>(swapchainImages.size()), &swapchainImageCount, data);
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
	}

	// Create the render targets
	m_SwapchainRenderTargets.resize(swapchainImages.size());
	for (size_t renderTargetIndex = 0u; renderTargetIndex < m_SwapchainRenderTargets.size(); ++renderTargetIndex)
	{
		RenderTarget*& renderTarget = m_SwapchainRenderTargets.at(renderTargetIndex);

		const VkImage image = swapchainImages.at(renderTargetIndex).image;
		renderTarget = new RenderTarget(vkDevice, image, m_ColorBuffer->GetImageView(), m_DepthBuffer->GetImageView(), eyeResolution, colorFormat, m_RenderPass, 2u);
	}
}

void Headset::CreateRenderPass(const VkSampleCountFlagBits& m_MultisampleCount, const VkDevice& vkDevice)
{

	constexpr uint32_t viewMask = 0b00000011;
	constexpr uint32_t correlationMask = 0b00000011;

	VkRenderPassMultiviewCreateInfo renderPassMultiviewCreateInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_MULTIVIEW_CREATE_INFO };
	renderPassMultiviewCreateInfo.subpassCount = 1u;
	renderPassMultiviewCreateInfo.pViewMasks = &viewMask;
	renderPassMultiviewCreateInfo.correlationMaskCount = 1u;
	renderPassMultiviewCreateInfo.pCorrelationMasks = &correlationMask;

	VkAttachmentDescription colorAttachmentDescription{};
	colorAttachmentDescription.format = colorFormat;
	colorAttachmentDescription.samples = m_MultisampleCount;
	colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentReference;
	colorAttachmentReference.attachment = 0u;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachmentDescription{};
	depthAttachmentDescription.format = depthFormat;
	depthAttachmentDescription.samples = m_MultisampleCount;
	depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentReference;
	depthAttachmentReference.attachment = 1u;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription resolveAttachmentDescription{};
	resolveAttachmentDescription.format = colorFormat;
	resolveAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	resolveAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	resolveAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	resolveAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	resolveAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	resolveAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	resolveAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference resolveAttachmentReference;
	resolveAttachmentReference.attachment = 2u;
	resolveAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription{};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1u;
	subpassDescription.pColorAttachments = &colorAttachmentReference;
	subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
	subpassDescription.pResolveAttachments = &resolveAttachmentReference;

	const std::array attachments = { colorAttachmentDescription, depthAttachmentDescription, resolveAttachmentDescription };

	VkRenderPassCreateInfo renderPassCreateInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	renderPassCreateInfo.pNext = &renderPassMultiviewCreateInfo;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassCreateInfo.pAttachments = attachments.data();
	renderPassCreateInfo.subpassCount = 1u;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	if (vkCreateRenderPass(vkDevice, &renderPassCreateInfo, nullptr, &m_RenderPass) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}
}

void Headset::VerifyColorFormatSupport(XrResult& result)
{

	uint32_t formatCount = 0u;
	result = xrEnumerateSwapchainFormats(m_Session, 0u, &formatCount, nullptr);
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
	}

	std::vector<int64_t> formats(formatCount);
	result = xrEnumerateSwapchainFormats(m_Session, formatCount, &formatCount, formats.data());
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
	}

	bool formatFound = false;
	for (const int64_t& format : formats)
	{
		if (format == static_cast<int64_t>(colorFormat))
		{
			formatFound = true;
			break;
		}
	}

	if (!formatFound)
	{
		utils::ThrowError(EError::FeatureNotSupported, "OpenXR swapchain color format");
	}
}

Headset::~Headset()
{
	// Clean up OpenXR
	if (m_Session)
	{
		// End the session if it is running
		if (m_SessionState == XR_SESSION_STATE_SYNCHRONIZED || m_SessionState == XR_SESSION_STATE_VISIBLE || m_SessionState == XR_SESSION_STATE_FOCUSED || m_SessionState == XR_SESSION_STATE_STOPPING || m_SessionState == XR_SESSION_STATE_READY)
		{
			xrEndSession(m_Session);
		}

		if (m_Swapchain)
		{
			xrDestroySwapchain(m_Swapchain);
		}

		xrDestroySession(m_Session);
	}

	if (m_Space)
	{
		xrDestroySpace(m_Space);
	}

	if (m_DepthBuffer)
	{
		delete m_DepthBuffer;
	}

	if (m_ColorBuffer)
	{
		delete m_ColorBuffer;
	}

	const VkDevice vkDevice = m_Device->GetVkDevice();
	if (vkDevice && m_RenderPass)
	{
		vkDestroyRenderPass(vkDevice, m_RenderPass, nullptr);
	}

	for (RenderTarget* renderTarget : m_SwapchainRenderTargets)
	{
		delete renderTarget;
	}
}

Headset::BeginFrameResult Headset::BeginFrame(uint32_t& outSwapchainImageIndex)
{
	const XrInstance instance = m_Device->GetXrInstance();

	// Poll OpenXR events
	XrEventDataBuffer buffer{ XR_TYPE_EVENT_DATA_BUFFER };
	while (xrPollEvent(instance, &buffer) == XR_SUCCESS)
	{
		switch (buffer.type)
		{
		case XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING:
			m_ExitRequested = true;
			return BeginFrameResult::SkipFully;
		case XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED:
		{
			XrEventDataSessionStateChanged* event = reinterpret_cast<XrEventDataSessionStateChanged*>(&buffer);
			m_SessionState = event->state;

			if (event->state == XR_SESSION_STATE_READY)
			{
				if (!BeginSession())
				{
					return BeginFrameResult::ThrowError;
				}
			}
			else if (event->state == XR_SESSION_STATE_STOPPING)
			{
				if (!EndSession())
				{
					return BeginFrameResult::ThrowError;
				}
			}
			else if (event->state == XR_SESSION_STATE_LOSS_PENDING || event->state == XR_SESSION_STATE_EXITING)
			{
				m_ExitRequested = true;
				return BeginFrameResult::SkipFully;
			}

			break;
		}
		}

		buffer.type = XR_TYPE_EVENT_DATA_BUFFER;
	}

	if (m_SessionState != XR_SESSION_STATE_READY && m_SessionState != XR_SESSION_STATE_SYNCHRONIZED && m_SessionState != XR_SESSION_STATE_VISIBLE && m_SessionState != XR_SESSION_STATE_FOCUSED)
	{
		return BeginFrameResult::SkipFully;
	}

	m_FrameState.type = XR_TYPE_FRAME_STATE;
	XrFrameWaitInfo frameWaitInfo{ XR_TYPE_FRAME_WAIT_INFO };
	XrResult		result = xrWaitFrame(m_Session, &frameWaitInfo, &m_FrameState);
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
		return BeginFrameResult::ThrowError;
	}

	XrFrameBeginInfo frameBeginInfo{ XR_TYPE_FRAME_BEGIN_INFO };
	result = xrBeginFrame(m_Session, &frameBeginInfo);
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
		return BeginFrameResult::ThrowError;
	}

	if (!m_FrameState.shouldRender)
	{
		return BeginFrameResult::SkipRender;
	}

	// Update the eye poses
	m_ViewState.type = XR_TYPE_VIEW_STATE;
	uint32_t		 viewCount;
	XrViewLocateInfo viewLocateInfo{ XR_TYPE_VIEW_LOCATE_INFO };
	viewLocateInfo.viewConfigurationType = m_Device->GetXrViewType();
	viewLocateInfo.displayTime = m_FrameState.predictedDisplayTime;
	viewLocateInfo.space = m_Space;
	result = xrLocateViews(m_Session, &viewLocateInfo, &m_ViewState, static_cast<uint32_t>(m_EyePoses.size()), &viewCount, m_EyePoses.data());
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
		return BeginFrameResult::ThrowError;
	}

	if (viewCount != m_EyeCount)
	{
		utils::ThrowError(EError::GenericOpenXR);
		return BeginFrameResult::ThrowError;
	}

	// Update the eye render infos, view and projection matrices
	for (size_t eyeIndex = 0u; eyeIndex < m_EyeCount; ++eyeIndex)
	{
		const XrView&					  eyePose = m_EyePoses.at(eyeIndex);
		XrCompositionLayerProjectionView& eyeRenderInfo = m_EyeRenderInfos.at(eyeIndex);
		eyeRenderInfo.pose = eyePose.pose;
		eyeRenderInfo.fov = eyePose.fov;
		const XrPosef& pose = eyeRenderInfo.pose;
		m_EyeViewMatrices.at(eyeIndex) = glm::inverse(utils::ToMatrix(pose));
		m_EyeProjectionMatrices.at(eyeIndex) = utils::CreateProjectionMatrix(eyeRenderInfo.fov, 0.01f, 250.0f);

		m_ViewerPosition = glm::vec3(pose.position.x, pose.position.y, pose.position.z) - m_ViewerPositionOffset;
		m_ViewerOrientation = glm::quat(pose.orientation.w, pose.orientation.x, pose.orientation.y, pose.orientation.z);

		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), m_ViewerPosition);
		glm::mat4 rotationMatrix = glm::mat4_cast(m_ViewerOrientation);
		worldMatrix = translationMatrix * rotationMatrix;
	}

	XrSwapchainImageAcquireInfo swapchainImageAcquireInfo{ XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO };
	result = xrAcquireSwapchainImage(m_Swapchain, &swapchainImageAcquireInfo, &outSwapchainImageIndex);
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
		return BeginFrameResult::ThrowError;
	}

	XrSwapchainImageWaitInfo swapchainImageWaitInfo{ XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO };
	swapchainImageWaitInfo.timeout = XR_INFINITE_DURATION;
	result = xrWaitSwapchainImage(m_Swapchain, &swapchainImageWaitInfo);
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
		return BeginFrameResult::ThrowError;
	}

	return BeginFrameResult::RenderFully;
}

void Headset::EndFrame() const
{
	// Release the swapchain image
	XrSwapchainImageReleaseInfo swapchainImageReleaseInfo{ XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO };
	XrResult					result = xrReleaseSwapchainImage(m_Swapchain, &swapchainImageReleaseInfo);
	if (XR_FAILED(result))
	{
		return;
	}

	// End the frame
	XrCompositionLayerProjection compositionLayerProjection{ XR_TYPE_COMPOSITION_LAYER_PROJECTION };
	compositionLayerProjection.space = m_Space;
	compositionLayerProjection.viewCount = static_cast<uint32_t>(m_EyeRenderInfos.size());
	compositionLayerProjection.views = m_EyeRenderInfos.data();

	std::vector<XrCompositionLayerBaseHeader*> layers;

	const bool positionValid = m_ViewState.viewStateFlags & XR_VIEW_STATE_POSITION_VALID_BIT;
	const bool orientationValid = m_ViewState.viewStateFlags & XR_VIEW_STATE_ORIENTATION_VALID_BIT;
	if (m_FrameState.shouldRender && positionValid && orientationValid)
	{
		layers.push_back(reinterpret_cast<XrCompositionLayerBaseHeader*>(&compositionLayerProjection));
	}

	XrFrameEndInfo frameEndInfo{ XR_TYPE_FRAME_END_INFO };
	frameEndInfo.displayTime = m_FrameState.predictedDisplayTime;
	frameEndInfo.layerCount = static_cast<uint32_t>(layers.size());
	frameEndInfo.layers = layers.data();
	frameEndInfo.environmentBlendMode = XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
	result = xrEndFrame(m_Session, &frameEndInfo);
	if (XR_FAILED(result))
	{
		return;
	}
}

VkExtent2D Headset::GetEyeResolution(size_t eyeIndex) const
{
	const XrViewConfigurationView& eyeInfo = m_EyeImageInfos.at(eyeIndex);
	return { eyeInfo.recommendedImageRectWidth, eyeInfo.recommendedImageRectHeight };
}

bool Headset::BeginSession() const
{
	// Start the session
	XrSessionBeginInfo sessionBeginInfo{ XR_TYPE_SESSION_BEGIN_INFO };
	sessionBeginInfo.primaryViewConfigurationType = m_Device->GetXrViewType();
	const XrResult result = xrBeginSession(m_Session, &sessionBeginInfo);
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
	}

	return true;
}

bool Headset::EndSession() const
{
	// End the session
	const XrResult result = xrEndSession(m_Session);
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
	}

	return true;
}