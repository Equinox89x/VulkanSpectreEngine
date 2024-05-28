#include "Controllers.h"

#include "../Input/InputHandler.h"
#include "../Misc/Utils.h"
#include "../VulkanBase/VulkanDevice.h"
#include <array>
#include <cstring>

Controllers::Controllers(XrInstance instance, XrSession session) : m_Session(session)
{
	// Create an action set
	XrActionSetCreateInfo actionSetCreateInfo{ XR_TYPE_ACTION_SET_CREATE_INFO };

	memcpy(actionSetCreateInfo.actionSetName, Spectre::actionSetName.data(), Spectre::actionSetName.length() + 1u);
	memcpy(actionSetCreateInfo.localizedActionSetName, Spectre::localizedActionSetName.data(), Spectre::localizedActionSetName.length() + 1u);

	XrResult result{ xrCreateActionSet(instance, &actionSetCreateInfo, &m_ActionSetData.actionSet) };
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
	}

	// Create paths
	m_ActionSetData.controllerPaths.resize((int)Spectre::ControllerHand::COUNT);
	m_ActionSetData.controllerPaths[(int)Spectre::ControllerHand::LEFT] = utils::StringToXrPath(instance, "/user/hand/left");
	m_ActionSetData.controllerPaths[(int)Spectre::ControllerHand::RIGHT] = utils::StringToXrPath(instance, "/user/hand/right");

	AddAction("Hand Pose", "handpose", XR_ACTION_TYPE_POSE_INPUT, &m_ActionSetData.aimPoseAction, Spectre::InputActions::MoveAction);
	AddAction("Fly", "fly", XR_ACTION_TYPE_FLOAT_INPUT, &m_ActionSetData.selectClickAction, Spectre::InputActions::FlyAction);
	// AddAction("Walk", "walk", XR_ACTION_TYPE_FLOAT_INPUT, actionSetData.thumbstickAction, Spectre::InputActions::WalkAction);

	// Create actions
	CreateActions();

	// Create spaces
	m_Spaces.resize(Spectre::controllerCount);

	// Bind actions with paths
	BindActions(instance);

	// Attach the controller action set
	XrSessionActionSetsAttachInfo sessionActionSetsAttachInfo{ XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
	sessionActionSetsAttachInfo.countActionSets = 1u;
	sessionActionSetsAttachInfo.actionSets = &m_ActionSetData.actionSet;

	result = xrAttachSessionActionSets(session, &sessionActionSetsAttachInfo);
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
	}

	m_Poses.resize(Spectre::controllerCount);
	m_FlySpeeds.resize(Spectre::controllerCount);

	RegisterInputs();
}

void Controllers::BindActions(const XrInstance& instance)
{
	//These are all supported paths of an controller. You just have to change the XrActionSuggestedBinding vector with the right ones per controller.
	// hand pose that allows applications to point in the world using the input source, according to the platform’s conventions for aiming with that kind of source.
	std::array<XrPath, (int)Spectre::ControllerHand::COUNT> aimPosePath;
	// hand pose that allows applications to reliably render a virtual object held in the user’s hand, whether it is tracked directly or by a motion controller.
	std::array<XrPath, (int)Spectre::ControllerHand::COUNT> gripPosePath;

	std::array<XrPath, (int)Spectre::ControllerHand::COUNT> triggerValuePath;
	std::array<XrPath, (int)Spectre::ControllerHand::COUNT> triggerClickPath;
	std::array<XrPath, (int)Spectre::ControllerHand::COUNT> triggerTouchPath;

	std::array<XrPath, (int)Spectre::ControllerHand::COUNT> squeezeValuePath;
	std::array<XrPath, (int)Spectre::ControllerHand::COUNT> squeezeForcePath;
	std::array<XrPath, (int)Spectre::ControllerHand::COUNT> squeezeClickPath;

	std::array<XrPath, (int)Spectre::ControllerHand::COUNT> thumbstickPath;
	std::array<XrPath, (int)Spectre::ControllerHand::COUNT> thumbstickClickPath;
	std::array<XrPath, (int)Spectre::ControllerHand::COUNT> thumbstickXPath;
	std::array<XrPath, (int)Spectre::ControllerHand::COUNT> thumbstickYPath;
	std::array<XrPath, (int)Spectre::ControllerHand::COUNT> menuClickPath;
	std::array<XrPath, (int)Spectre::ControllerHand::COUNT> selectClickPath;

	std::array<XrPath, (int)Spectre::ControllerHand::COUNT> aClickPath;
	std::array<XrPath, (int)Spectre::ControllerHand::COUNT> bClickPath;
	std::array<XrPath, (int)Spectre::ControllerHand::COUNT> xClickPath;
	std::array<XrPath, (int)Spectre::ControllerHand::COUNT> yClickPath;

	std::array<XrPath, (int)Spectre::ControllerHand::COUNT> hapticPath;

	xrStringToPath(instance, "/user/hand/left/input/aim/pose", &aimPosePath[(int)Spectre::ControllerHand::LEFT]);
	xrStringToPath(instance, "/user/hand/right/input/aim/pose", &aimPosePath[(int)Spectre::ControllerHand::RIGHT]);

	xrStringToPath(instance, "/user/hand/left/input/grip/pose", &gripPosePath[(int)Spectre::ControllerHand::LEFT]);
	xrStringToPath(instance, "/user/hand/right/input/grip/pose", &gripPosePath[(int)Spectre::ControllerHand::RIGHT]);

	xrStringToPath(instance, "/user/hand/left/input/trigger/value", &triggerValuePath[(int)Spectre::ControllerHand::LEFT]);
	xrStringToPath(instance, "/user/hand/right/input/trigger/value", &triggerValuePath[(int)Spectre::ControllerHand::RIGHT]);
	xrStringToPath(instance, "/user/hand/left/input/trigger/click", &triggerClickPath[(int)Spectre::ControllerHand::LEFT]);
	xrStringToPath(instance, "/user/hand/right/input/trigger/click", &triggerClickPath[(int)Spectre::ControllerHand::RIGHT]);
	xrStringToPath(instance, "/user/hand/left/input/trigger/touch", &triggerTouchPath[(int)Spectre::ControllerHand::LEFT]);
	xrStringToPath(instance, "/user/hand/right/input/trigger/touch", &triggerTouchPath[(int)Spectre::ControllerHand::RIGHT]);

	xrStringToPath(instance, "/user/hand/left/input/squeeze/value", &squeezeValuePath[(int)Spectre::ControllerHand::LEFT]);
	xrStringToPath(instance, "/user/hand/right/input/squeeze/value", &squeezeValuePath[(int)Spectre::ControllerHand::RIGHT]);
	xrStringToPath(instance, "/user/hand/left/input/squeeze/force", &squeezeForcePath[(int)Spectre::ControllerHand::LEFT]);
	xrStringToPath(instance, "/user/hand/right/input/squeeze/force", &squeezeForcePath[(int)Spectre::ControllerHand::RIGHT]);
	xrStringToPath(instance, "/user/hand/left/input/squeeze/click", &squeezeClickPath[(int)Spectre::ControllerHand::LEFT]);
	xrStringToPath(instance, "/user/hand/right/input/squeeze/click", &squeezeClickPath[(int)Spectre::ControllerHand::RIGHT]);

	xrStringToPath(instance, "/user/hand/left/input/thumbstick", &thumbstickPath[(int)Spectre::ControllerHand::LEFT]);
	xrStringToPath(instance, "/user/hand/right/input/thumbstick", &thumbstickPath[(int)Spectre::ControllerHand::RIGHT]);
	xrStringToPath(instance, "/user/hand/left/input/thumbstick/click", &thumbstickClickPath[(int)Spectre::ControllerHand::LEFT]);
	xrStringToPath(instance, "/user/hand/right/input/thumbstick/click", &thumbstickClickPath[(int)Spectre::ControllerHand::RIGHT]);

	xrStringToPath(instance, "/user/hand/left/input/thumbstick/x", &thumbstickXPath[(int)Spectre::ControllerHand::LEFT]);
	xrStringToPath(instance, "/user/hand/right/input/thumbstick/y", &thumbstickYPath[(int)Spectre::ControllerHand::RIGHT]);
	xrStringToPath(instance, "/user/hand/left/input/thumbstick/x", &thumbstickXPath[(int)Spectre::ControllerHand::LEFT]);
	xrStringToPath(instance, "/user/hand/right/input/thumbstick/y", &thumbstickYPath[(int)Spectre::ControllerHand::RIGHT]);

	xrStringToPath(instance, "/user/hand/left/input/menu/click", &menuClickPath[(int)Spectre::ControllerHand::LEFT]);
	xrStringToPath(instance, "/user/hand/right/input/menu/click", &menuClickPath[(int)Spectre::ControllerHand::RIGHT]);

	xrStringToPath(instance, "/user/hand/left/input/select/click", &selectClickPath[(int)Spectre::ControllerHand::LEFT]);
	xrStringToPath(instance, "/user/hand/right/input/select/click", &selectClickPath[(int)Spectre::ControllerHand::RIGHT]);

	xrStringToPath(instance, "/user/hand/left/input/a/click", &aClickPath[(int)Spectre::ControllerHand::LEFT]);
	xrStringToPath(instance, "/user/hand/right/input/a/click", &aClickPath[(int)Spectre::ControllerHand::RIGHT]);
	xrStringToPath(instance, "/user/hand/left/input/b/click", &bClickPath[(int)Spectre::ControllerHand::LEFT]);
	xrStringToPath(instance, "/user/hand/right/input/b/click", &bClickPath[(int)Spectre::ControllerHand::RIGHT]);
	xrStringToPath(instance, "/user/hand/left/input/x/click", &xClickPath[(int)Spectre::ControllerHand::LEFT]);
	xrStringToPath(instance, "/user/hand/right/input/x/click", &xClickPath[(int)Spectre::ControllerHand::RIGHT]);
	xrStringToPath(instance, "/user/hand/left/input/y/click", &yClickPath[(int)Spectre::ControllerHand::LEFT]);
	xrStringToPath(instance, "/user/hand/right/input/y/click", &yClickPath[(int)Spectre::ControllerHand::RIGHT]);

	xrStringToPath(instance, "/user/hand/left/output/haptic", &hapticPath[(int)Spectre::ControllerHand::LEFT]);
	xrStringToPath(instance, "/user/hand/right/output/haptic", &hapticPath[(int)Spectre::ControllerHand::RIGHT]);

	std::vector<XrActionSuggestedBinding> bindings = { { m_ActionSetData.aimPoseAction, aimPosePath[(int)Spectre::ControllerHand::LEFT] },	// hand pose
													   { m_ActionSetData.aimPoseAction, aimPosePath[(int)Spectre::ControllerHand::RIGHT] }, // hand pose
													   { m_ActionSetData.selectClickAction, selectClickPath[(int)Spectre::ControllerHand::LEFT] },
													   { m_ActionSetData.selectClickAction, selectClickPath[(int)Spectre::ControllerHand::RIGHT] } };

	XrInteractionProfileSuggestedBinding interactionProfileSuggestedBinding{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
	interactionProfileSuggestedBinding.interactionProfile = utils::StringToXrPath(instance, "/interaction_profiles/khr/simple_controller");
	interactionProfileSuggestedBinding.suggestedBindings = bindings.data();
	interactionProfileSuggestedBinding.countSuggestedBindings = static_cast<uint32_t>(bindings.size());

	XrResult result = xrSuggestInteractionProfileBindings(instance, &interactionProfileSuggestedBinding);
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
	}
}

void Controllers::RegisterInputs()
{
	for (auto& action : m_Actions)
	{
		InputAction* ia{ nullptr };
		switch (action.second->InputAction)
		{
		case Spectre::InputActions::MoveAction:
			InputHandler::GetInstance().AddAction(new MoveAction(this, action.second->Action));
			break;
		case Spectre::InputActions::FlyAction:
			InputHandler::GetInstance().AddAction(new FlyAction(this, action.second->Action));
			break;
		case Spectre::InputActions::WalkAction:
			InputHandler::GetInstance().AddAction(new WalkAction(this, action.second->Action));
			break;
		default:
			break;
		}
	}
}

void Controllers::CreateActions()
{
	for (auto& action : m_Actions)
	{
		utils::CreateAction(m_ActionSetData.actionSet, m_ActionSetData.controllerPaths, action.second->ActionName, action.second->DisplayName, action.second->Type, *action.second->Action);
	}
}

Controllers::~Controllers()
{
	for (const XrSpace& m_Space : m_Spaces)
	{
		xrDestroySpace(m_Space);
	}

	for (auto& action : m_Actions)
	{
		action.second.release();
	}

	if (m_ActionSetData.actionSet)
	{
		xrDestroyActionSet(m_ActionSetData.actionSet);
	}
}

bool Controllers::Sync()
{
	// Sync the actions
	XrActiveActionSet activeActionSet;
	activeActionSet.actionSet = m_ActionSetData.actionSet;
	activeActionSet.subactionPath = XR_NULL_PATH;

	XrActionsSyncInfo actionsSyncInfo{ XR_TYPE_ACTIONS_SYNC_INFO };
	actionsSyncInfo.countActiveActionSets = 1u;
	actionsSyncInfo.activeActionSets = &activeActionSet;

	XrResult result{ xrSyncActions(m_Session, &actionsSyncInfo) };
	if (XR_FAILED(result))
	{
		utils::ThrowError(EError::GenericOpenXR);
	}
	return true;
}

size_t Controllers::GetNumberOfController() { return Spectre::controllerCount; }

void Controllers::AddAction(std::string DisplayName, std::string actionName, XrActionType type, XrAction* action, Spectre::InputActions inputAction) { m_Actions.emplace(std::pair<std::string, std::unique_ptr<ActionDefinition>>{ DisplayName, std::make_unique<ActionDefinition>(actionName, DisplayName, type, action, inputAction) }); }
