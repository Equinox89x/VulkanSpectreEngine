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

	AddAction("handpose", "Hand Pose", XR_ACTION_TYPE_POSE_INPUT, &m_ActionSetData.aimPoseAction, Spectre::InputActions::MoveAction);
	AddAction("grippose", "Grip Pose", XR_ACTION_TYPE_POSE_INPUT, &m_ActionSetData.gripPoseAction, Spectre::InputActions::MoveAction);
	AddAction("trigger_v_action", "Trigger", XR_ACTION_TYPE_FLOAT_INPUT, &m_ActionSetData.triggerValueAction, Spectre::InputActions::FlyAction);
	AddAction("squeeze_v_action", "Squeeze", XR_ACTION_TYPE_FLOAT_INPUT, &m_ActionSetData.squeezeValueAction, Spectre::InputActions::SqueezeAction);
	AddAction("grab_action", "Grab Action", XR_ACTION_TYPE_FLOAT_INPUT, &m_ActionSetData.grabAction, Spectre::InputActions::WalkAction);
	AddAction("thumbstick_action", "Thumbstick Action", XR_ACTION_TYPE_FLOAT_INPUT, &m_ActionSetData.thumbstickAction, Spectre::InputActions::WalkAction); // ThumbstickAction
	AddAction("menu_action", "Menu Action", XR_ACTION_TYPE_FLOAT_INPUT, &m_ActionSetData.menuClickAction, Spectre::InputActions::MenuAction);
	AddAction("select_action", "Select Action", XR_ACTION_TYPE_FLOAT_INPUT, &m_ActionSetData.selectClickAction, Spectre::InputActions::FlyAction); // MenuAction
	AddAction("vibrate_action", "Vibration Action", XR_ACTION_TYPE_VIBRATION_OUTPUT, &m_ActionSetData.vibrateAction, Spectre::InputActions::VibrationAction);

	// Create actions
	CreateActions();

	// Create spaces
	m_Spaces.resize(Spectre::controllerCount);

	//m_ActionSetData.controllerReferenceSpaces_aim.resize((int)Spectre::ControllerHand::COUNT);
	//m_ActionSetData.controllerReferenceSpaces_grip.resize((int)Spectre::ControllerHand::COUNT);
	/*for (size_t ci = 0u; ci < (int)Spectre::ControllerHand::COUNT; ci++)
	{
		XrActionSpaceCreateInfo actionSpaceInfo{ XR_TYPE_ACTION_SPACE_CREATE_INFO };
		actionSpaceInfo.action = m_ActionSetData.aimPoseAction;
		actionSpaceInfo.subactionPath = m_ActionSetData.controllerPaths[ci];
		actionSpaceInfo.poseInActionSpace.orientation = { .x = 0, .y = 0, .z = 0, .w = 1.0 };
		actionSpaceInfo.poseInActionSpace.position = { .x = 0, .y = 0, .z = 0 };

		result = xrCreateActionSpace(session, &actionSpaceInfo, &m_ActionSetData.controllerReferenceSpaces_aim[ci]);
		if (XR_FAILED(result))
		{
			utils::ThrowError(EError::GenericOpenXR);
		}

		actionSpaceInfo.action = m_ActionSetData.gripPoseAction;
		result = xrCreateActionSpace(session, &actionSpaceInfo, &m_ActionSetData.controllerReferenceSpaces_grip[ci]);
		if (XR_FAILED(result))
		{
			utils::ThrowError(EError::GenericOpenXR);
		}
	}*/

	// Bind actions with paths
	BindActions(instance);

	m_Poses.resize(Spectre::controllerCount);
	m_FlySpeeds.resize(Spectre::controllerCount);
	m_WalkSpeeds.resize(Spectre::controllerCount);

	RegisterInputs();
}

void Controllers::BindActions(const XrInstance& instance)
{
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


	// For other controller types (vive, oculus,....) we need a different interaction profile with different XrActionSuggestedBinding bindings
	// To check which path we need go to:
	// https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#semantic-path-interaction-profiles Chapter 6.4 - Interaction Profile Paths

	// Suggest bindings for KHR Simple.
	XrPath	 khrSimpleInteractionProfilePath;
	XrResult result{ xrStringToPath(instance, "/interaction_profiles/khr/simple_controller", &khrSimpleInteractionProfilePath) };

	std::vector<XrActionSuggestedBinding> bindings{ { { m_ActionSetData.grabAction, selectClickPath[(int)Spectre::ControllerHand::LEFT] },
													  { m_ActionSetData.grabAction, selectClickPath[(int)Spectre::ControllerHand::RIGHT] },
													  { m_ActionSetData.gripPoseAction, gripPosePath[(int)Spectre::ControllerHand::LEFT] },	 // hand pose
													  { m_ActionSetData.gripPoseAction, gripPosePath[(int)Spectre::ControllerHand::RIGHT] }, // hand pose
													  { m_ActionSetData.aimPoseAction, aimPosePath[(int)Spectre::ControllerHand::LEFT] },	 // hand pose
													  { m_ActionSetData.aimPoseAction, aimPosePath[(int)Spectre::ControllerHand::RIGHT] },	 // hand pose

													  { m_ActionSetData.selectClickAction, selectClickPath[(int)Spectre::ControllerHand::LEFT] },
													  { m_ActionSetData.selectClickAction, selectClickPath[(int)Spectre::ControllerHand::RIGHT] },
													  { m_ActionSetData.menuClickAction, menuClickPath[(int)Spectre::ControllerHand::LEFT] },
													  { m_ActionSetData.menuClickAction, menuClickPath[(int)Spectre::ControllerHand::RIGHT] },

													  { m_ActionSetData.vibrateAction, hapticPath[(int)Spectre::ControllerHand::LEFT] },
													  { m_ActionSetData.vibrateAction, hapticPath[(int)Spectre::ControllerHand::RIGHT] } } };

	XrInteractionProfileSuggestedBinding suggestedBindings{ XR_TYPE_INTERACTION_PROFILE_SUGGESTED_BINDING };
	suggestedBindings.interactionProfile = khrSimpleInteractionProfilePath;
	suggestedBindings.suggestedBindings = bindings.data();
	suggestedBindings.countSuggestedBindings = static_cast<uint32_t>(bindings.size());
	result = xrSuggestInteractionProfileBindings(instance, &suggestedBindings);




	// Suggest bindings for the Oculus Touch.
	// https://docs.unity3d.com/Packages/com.unity.xr.openxr@1.6/manual/features/oculustouchcontrollerprofile.html
	XrPath oculusTouchInteractionProfilePath;
	result = xrStringToPath(instance, "/interaction_profiles/oculus/touch_controller", &oculusTouchInteractionProfilePath);

	bindings = { { m_ActionSetData.grabAction, squeezeValuePath[(int)Spectre::ControllerHand::LEFT] },
				 { m_ActionSetData.grabAction, squeezeValuePath[(int)Spectre::ControllerHand::RIGHT] },
				 { m_ActionSetData.gripPoseAction, gripPosePath[(int)Spectre::ControllerHand::LEFT] },	// hand pose
				 { m_ActionSetData.gripPoseAction, gripPosePath[(int)Spectre::ControllerHand::RIGHT] }, // hand pose
				 { m_ActionSetData.aimPoseAction, aimPosePath[(int)Spectre::ControllerHand::LEFT] },	// hand pose
				 { m_ActionSetData.aimPoseAction, aimPosePath[(int)Spectre::ControllerHand::RIGHT] },	// hand pose
				 { m_ActionSetData.thumbstickAction, thumbstickPath[(int)Spectre::ControllerHand::LEFT] },
				 { m_ActionSetData.thumbstickAction, thumbstickPath[(int)Spectre::ControllerHand::RIGHT] },

				 { m_ActionSetData.selectClickAction, aClickPath[(int)Spectre::ControllerHand::RIGHT] },
				 // note: only on left side!
				 { m_ActionSetData.menuClickAction, menuClickPath[(int)Spectre::ControllerHand::LEFT] },

				 // Old oculus Rifts for some reason fail on this path, Comment if you're supporting old devices.
				 //{ m_ActionSetData.triggerAction, triggerValuePath[(int)Spectre::ControllerHand::LEFT] },
				 //{ m_ActionSetData.triggerAction, triggerValuePath[(int)Spectre::ControllerHand::RIGHT] },

				 { m_ActionSetData.vibrateAction, hapticPath[(int)Spectre::ControllerHand::LEFT] },
				 { m_ActionSetData.vibrateAction, hapticPath[(int)Spectre::ControllerHand::RIGHT] } };

	suggestedBindings.interactionProfile = oculusTouchInteractionProfilePath;
	suggestedBindings.suggestedBindings = bindings.data();
	suggestedBindings.countSuggestedBindings = static_cast<uint32_t>(bindings.size());
	result = xrSuggestInteractionProfileBindings(instance, &suggestedBindings);




	// Suggest bindings for the Vive Controller.
	XrPath viveControllerInteractionProfilePath;
	result = xrStringToPath(instance, "/interaction_profiles/htc/vive_controller", &viveControllerInteractionProfilePath);

	bindings = { { m_ActionSetData.grabAction, triggerValuePath[(int)Spectre::ControllerHand::LEFT] },
				 { m_ActionSetData.grabAction, triggerValuePath[(int)Spectre::ControllerHand::RIGHT] },
				 { m_ActionSetData.gripPoseAction, gripPosePath[(int)Spectre::ControllerHand::LEFT] },	// hand pose
				 { m_ActionSetData.gripPoseAction, gripPosePath[(int)Spectre::ControllerHand::RIGHT] }, // hand pose
				 { m_ActionSetData.aimPoseAction, aimPosePath[(int)Spectre::ControllerHand::LEFT] },	// hand pose
				 { m_ActionSetData.aimPoseAction, aimPosePath[(int)Spectre::ControllerHand::RIGHT] },	// hand pose

				 { m_ActionSetData.menuClickAction, menuClickPath[(int)Spectre::ControllerHand::LEFT] },
				 { m_ActionSetData.menuClickAction, menuClickPath[(int)Spectre::ControllerHand::RIGHT] },

				 //{m_ActionSetData.triggerAction, triggerValuePath[(int)Spectre::ControllerHand::LEFT]},
				 //{m_ActionSetData.triggerAction, triggerValuePath[(int)Spectre::ControllerHand::RIGHT]},

				 { m_ActionSetData.vibrateAction, hapticPath[(int)Spectre::ControllerHand::LEFT] },
				 { m_ActionSetData.vibrateAction, hapticPath[(int)Spectre::ControllerHand::RIGHT] } };
	suggestedBindings.interactionProfile = viveControllerInteractionProfilePath;
	suggestedBindings.suggestedBindings = bindings.data();
	suggestedBindings.countSuggestedBindings = static_cast<uint32_t>(bindings.size());
	result = xrSuggestInteractionProfileBindings(instance, &suggestedBindings);




	// Suggest bindings for the Valve Index Controller.
	XrPath valveIndexControllerInteractionProfilePath;
	result = xrStringToPath(instance, "/interaction_profiles/valve/index_controller", &valveIndexControllerInteractionProfilePath);

	bindings = { { m_ActionSetData.grabAction, squeezeForcePath[(int)Spectre::ControllerHand::LEFT] },
				 { m_ActionSetData.grabAction, squeezeForcePath[(int)Spectre::ControllerHand::RIGHT] },
				 { m_ActionSetData.gripPoseAction, gripPosePath[(int)Spectre::ControllerHand::LEFT] },	// hand pose
				 { m_ActionSetData.gripPoseAction, gripPosePath[(int)Spectre::ControllerHand::RIGHT] }, // hand pose
				 { m_ActionSetData.aimPoseAction, aimPosePath[(int)Spectre::ControllerHand::LEFT] },	// hand pose
				 { m_ActionSetData.aimPoseAction, aimPosePath[(int)Spectre::ControllerHand::RIGHT] },	// hand pose
				 { m_ActionSetData.thumbstickAction, thumbstickPath[(int)Spectre::ControllerHand::LEFT] },
				 { m_ActionSetData.thumbstickAction, thumbstickPath[(int)Spectre::ControllerHand::RIGHT] },
				 { m_ActionSetData.selectClickAction, aClickPath[(int)Spectre::ControllerHand::LEFT] },
				 { m_ActionSetData.selectClickAction, aClickPath[(int)Spectre::ControllerHand::RIGHT] },

				 { m_ActionSetData.menuClickAction, bClickPath[(int)Spectre::ControllerHand::LEFT] },
				 { m_ActionSetData.menuClickAction, bClickPath[(int)Spectre::ControllerHand::RIGHT] },

				 //{m_ActionSetData.triggerAction, triggerValuePath[(int)Spectre::ControllerHand::LEFT]},
				 //{m_ActionSetData.triggerAction, triggerValuePath[(int)Spectre::ControllerHand::RIGHT]},

				 { m_ActionSetData.vibrateAction, hapticPath[(int)Spectre::ControllerHand::LEFT] },
				 { m_ActionSetData.vibrateAction, hapticPath[(int)Spectre::ControllerHand::RIGHT] } };

	suggestedBindings.interactionProfile = valveIndexControllerInteractionProfilePath;
	suggestedBindings.suggestedBindings = bindings.data();
	suggestedBindings.countSuggestedBindings = static_cast<uint32_t>(bindings.size());
	result = xrSuggestInteractionProfileBindings(instance, &suggestedBindings);




	// Suggest bindings for the Microsoft Mixed Reality Motion Controller.
	XrPath microsoftMixedRealityInteractionProfilePath;
	result = xrStringToPath(instance, "/interaction_profiles/microsoft/motion_controller", &microsoftMixedRealityInteractionProfilePath);

	bindings = { { m_ActionSetData.grabAction, squeezeClickPath[(int)Spectre::ControllerHand::LEFT] },
				 { m_ActionSetData.grabAction, squeezeClickPath[(int)Spectre::ControllerHand::RIGHT] },
				 { m_ActionSetData.gripPoseAction, gripPosePath[(int)Spectre::ControllerHand::LEFT] },	// hand pose
				 { m_ActionSetData.gripPoseAction, gripPosePath[(int)Spectre::ControllerHand::RIGHT] }, // hand pose
				 { m_ActionSetData.aimPoseAction, aimPosePath[(int)Spectre::ControllerHand::LEFT] },	// hand pose
				 { m_ActionSetData.aimPoseAction, aimPosePath[(int)Spectre::ControllerHand::RIGHT] },	// hand pose
				 { m_ActionSetData.thumbstickAction, thumbstickPath[(int)Spectre::ControllerHand::LEFT] },
				 { m_ActionSetData.thumbstickAction, thumbstickPath[(int)Spectre::ControllerHand::RIGHT] },

				 { m_ActionSetData.selectClickAction, menuClickPath[(int)Spectre::ControllerHand::LEFT] },
				 { m_ActionSetData.selectClickAction, menuClickPath[(int)Spectre::ControllerHand::RIGHT] },

				 { m_ActionSetData.menuClickAction, menuClickPath[(int)Spectre::ControllerHand::LEFT] },
				 { m_ActionSetData.menuClickAction, menuClickPath[(int)Spectre::ControllerHand::RIGHT] },

				 //{m_ActionSetData.triggerAction, triggerValuePath[(int)Spectre::ControllerHand::LEFT]},
				 //{m_ActionSetData.triggerAction, triggerValuePath[(int)Spectre::ControllerHand::RIGHT]},

				 { m_ActionSetData.vibrateAction, hapticPath[(int)Spectre::ControllerHand::LEFT] },
				 { m_ActionSetData.vibrateAction, hapticPath[(int)Spectre::ControllerHand::RIGHT] } };

	suggestedBindings.interactionProfile = microsoftMixedRealityInteractionProfilePath;
	suggestedBindings.suggestedBindings = bindings.data();
	suggestedBindings.countSuggestedBindings = static_cast<uint32_t>(bindings.size());
	result = xrSuggestInteractionProfileBindings(instance, &suggestedBindings);




	// Attach the controller action set
	XrSessionActionSetsAttachInfo sessionActionSetsAttachInfo{ XR_TYPE_SESSION_ACTION_SETS_ATTACH_INFO };
	sessionActionSetsAttachInfo.countActionSets = 1u;
	sessionActionSetsAttachInfo.actionSets = &m_ActionSetData.actionSet;

	result = xrAttachSessionActionSets(m_Session, &sessionActionSetsAttachInfo);
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
			InputHandler::GetInstance().AddAction(new MoveAction(this, action.second.get()));
			break;
		case Spectre::InputActions::FlyAction:
			InputHandler::GetInstance().AddAction(new FlyAction(this, action.second.get()));
			break;
		case Spectre::InputActions::WalkAction:
			InputHandler::GetInstance().AddAction(new WalkAction(this, action.second.get()));
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

void Controllers::AddAction(std::string actionName, std::string displayName, XrActionType type, XrAction* action, Spectre::InputActions inputAction) { m_Actions.emplace(std::pair<std::string, std::unique_ptr<ActionDefinition>>{ displayName, std::make_unique<ActionDefinition>(actionName, displayName, type, action, inputAction) }); }
