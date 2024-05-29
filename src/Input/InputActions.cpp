#include "InputActions.h"
#include "../Misc/Utils.h"
#include <glm/ext/matrix_transform.hpp>
#include "../Core/App.h"
#include "../Misc/Timer.h"

void MoveAction::Init() {
	for (size_t controllerIndex = 0u; controllerIndex < Spectre::controllerCount; ++controllerIndex)
	{
		const XrPath& path{ m_Controller->GetPaths().at(controllerIndex) };

		XrActionSpaceCreateInfo actionSpaceCreateInfo{ XR_TYPE_ACTION_SPACE_CREATE_INFO };
		actionSpaceCreateInfo.action = *m_Action->Action;
		actionSpaceCreateInfo.poseInActionSpace = utils::MakeIdentityPose();
		actionSpaceCreateInfo.subactionPath = path;

		XrResult result = xrCreateActionSpace(m_Controller->GetSession(), &actionSpaceCreateInfo, &m_Controller->GetSpaces().at(controllerIndex));
		if (XR_FAILED(result))
		{
			utils::ThrowError(EError::GenericOpenXR);
		}
	}
}

void MoveAction::Update(size_t controllerIndex, Headset* headset, const XrPath& path)
{
	// Pose
	XrActionStatePose poseState{ XR_TYPE_ACTION_STATE_POSE };
	utils::UpdateActionStatePose(m_Controller->GetSession(), *m_Action->Action, path, poseState);

	if (poseState.isActive)
	{
		XrSpaceLocation spaceLocation{ XR_TYPE_SPACE_LOCATION };
		XrResult		result = xrLocateSpace(m_Controller->GetSpaces().at(controllerIndex), headset->GetXrSpace(), headset->GetXrFrameState().predictedDisplayTime, &spaceLocation);
		if (XR_FAILED(result))
		{
			utils::ThrowError(EError::GenericOpenXR);
		}

		// Check that the position and orientation are valid and tracked
		constexpr XrSpaceLocationFlags checkFlags = XR_SPACE_LOCATION_POSITION_VALID_BIT | XR_SPACE_LOCATION_POSITION_TRACKED_BIT | XR_SPACE_LOCATION_ORIENTATION_VALID_BIT | XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT;
		if ((spaceLocation.locationFlags & checkFlags) == checkFlags)
		{
			m_Controller->GetPoses().at(controllerIndex) = utils::ToMatrix(spaceLocation.pose);
		}
	}
}

void FlyAction::Update(size_t controllerIndex, Headset* headset, const XrPath& path)
{
	float deltaTime{ Timer::GetInstance().GetDeltaTime() };

	// Fly speed
	XrActionStateFloat flySpeedState{ XR_TYPE_ACTION_STATE_FLOAT };
	utils::UpdateActionStateFloat(m_Controller->GetSession(), *m_Action->Action, path, flySpeedState);

	if (flySpeedState.isActive)
	{
		m_Controller->GetFlySpeeds().at(controllerIndex) = flySpeedState.currentState;
	}

	for (size_t controllerIndex = 0u; controllerIndex < 2u; ++controllerIndex)
	{
		const float flySpeed{ m_Controller->GetFlySpeed(controllerIndex) };
		if (flySpeed > 0.0f)
		{
			const glm::vec3 forward{ glm::normalize(m_Controller->GetPose(controllerIndex)[2]) };
			//headset->worldMatrix = glm::translate(headset->worldMatrix, forward * flySpeed * flySpeedMultiplier * deltaTime);
			headset->cameraMatrix = glm::translate(headset->cameraMatrix, forward * flySpeed * flySpeedMultiplier * deltaTime);
			headset->AddToViewerPosition(forward * flySpeed * flySpeedMultiplier * deltaTime);
		}
	}
}

void WalkAction::Update(size_t controllerIndex, Headset* headset, const XrPath& path)
{
	float deltaTime{ Timer::GetInstance().GetDeltaTime() };

	XrActionStateFloat walkHorizontalState{ XR_TYPE_ACTION_STATE_FLOAT };
	utils::UpdateActionStateFloat(m_Controller->GetSession(), *m_Action->Action, path, walkHorizontalState);

	if (walkHorizontalState.isActive)
	{
		m_Controller->GetWalkSpeeds().at(controllerIndex) = walkHorizontalState.currentState;
	}

	for (size_t controllerIndex = 0u; controllerIndex < 2u; ++controllerIndex)
	{
		const float flySpeed{ m_Controller->GetWalkSpeed(controllerIndex) };
		if (flySpeed > 0.0f)
		{
			const glm::vec3 forward{ glm::normalize(m_Controller->GetPose(controllerIndex)[2]) };
			// headset->worldMatrix = glm::translate(headset->worldMatrix, forward * flySpeed * flySpeedMultiplier * deltaTime);
			headset->cameraMatrix = glm::translate(headset->cameraMatrix, forward * flySpeed * flySpeedMultiplier * deltaTime);
			headset->AddToViewerPosition(forward * flySpeed * flySpeedMultiplier * deltaTime);
		}
	}
}