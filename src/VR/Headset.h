#pragma once

#include <glm/fwd.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <openxr/openxr.h>
#include <vector>
#include <vulkan/vulkan.h>
#include <glm/gtx/quaternion.hpp>
class VulkanDevice;
class ImageBuffer;
class RenderTarget;


class Headset final
{
public:
	Headset(){};
	Headset(const VulkanDevice* m_Device);
	~Headset();

	enum class BeginFrameResult
	{
		ThrowError,	 // An error occurred
		RenderFully, // Render this frame normally
		SkipRender,	 // Skip rendering the frame but end it
		SkipFully	 // Skip processing this frame entirely without ending it
	};
	BeginFrameResult BeginFrame(uint32_t& outSwapchainImageIndex);
	void			 EndFrame() const;

	bool		 IsExitRequested() const { return m_ExitRequested; }
	XrSession	 GetXrSession() const { return m_Session; }
	XrSpace		 GetXrSpace() const { return m_Space; }
	XrFrameState GetXrFrameState() const { return m_FrameState; }
	VkRenderPass GetVkRenderPass() const { return m_RenderPass; }
	size_t		 GetEyeCount() const { return m_EyeCount; }

	VkExtent2D	  GetEyeResolution(size_t eyeIndex) const;
	glm::mat4	  GetEyeViewMatrix(size_t eyeIndex) const { return m_EyeViewMatrices.at(eyeIndex); }
	glm::mat4	  GetEyeProjectionMatrix(size_t eyeIndex) const { return m_EyeProjectionMatrices.at(eyeIndex); }
	RenderTarget* GetRenderTarget(size_t swapchainImageIndex) const { return m_SwapchainRenderTargets.at(swapchainImageIndex); }
	
	const XrPosef& GetEyePose(size_t eyeIndex) const { return m_EyePoses.at(eyeIndex).pose; };

	glm::vec3 GetViewerPosition() const { return m_ViewerPosition; }
	glm::quat GetViewerOrientation() const { return m_ViewerOrientation; }

	void AddToViewerPosition(glm::vec3 pos) { m_ViewerPositionOffset += pos; };

	glm::mat4 cameraMatrix{ glm::mat4(1.0f) };
	glm::mat4 worldMatrix{ glm::mat4(1.0f) };

private:
	bool m_ExitRequested{ false };

	const VulkanDevice* m_Device{ nullptr };

	size_t				   m_EyeCount = 0u;
	std::vector<glm::mat4> m_EyeViewMatrices;
	std::vector<glm::mat4> m_EyeProjectionMatrices;
	glm::vec3			   m_ViewerPosition;
	glm::vec3			   m_ViewerPositionOffset{0};
	glm::quat			   m_ViewerOrientation;

	XrSession	   m_Session{ nullptr };
	XrSessionState m_SessionState{ XR_SESSION_STATE_UNKNOWN };
	XrSpace		   m_Space{ nullptr };
	XrFrameState   m_FrameState = {};
	XrViewState	   m_ViewState = {};

	std::vector<XrViewConfigurationView>		  m_EyeImageInfos;
	std::vector<XrView>							  m_EyePoses;
	std::vector<XrCompositionLayerProjectionView> m_EyeRenderInfos;

	XrSwapchain				   m_Swapchain{ nullptr };
	std::vector<RenderTarget*> m_SwapchainRenderTargets;

	VkRenderPass m_RenderPass{ nullptr };

	ImageBuffer *m_ColorBuffer{ nullptr }, *m_DepthBuffer{ nullptr };

	bool BeginSession() const;
	bool EndSession() const;
};