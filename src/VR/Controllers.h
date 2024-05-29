#pragma once

#include <glm/fwd.hpp>
#include <glm/mat4x4.hpp>
#include <openxr/openxr.h>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Spectre
{
	constexpr size_t controllerCount = 2u;

	const std::string actionSetName = "actionset";
	const std::string localizedActionSetName = "Actions";

	enum class Headsets
	{
		Oculus,
		KHR,
		Vive,
		ValveIndex,
		MSMR
	};

	enum class ControllerButtonName
	{
		aimPose,
		gripPose,
		triggerValue,
		triggerClick,
		squeezeValue,
		squeezeForce,
		squeezeClick,
		thumbstick,
		thumbstickClick,
		menuClick,
		selectClick,
		trigger,
		aClick,
		bClick,
		xClick,
		yClick,
		vibrate,
		grab,
	};

	enum class ControllerHand
	{
		LEFT,
		RIGHT,
		COUNT
	};

	enum class InputActions
	{
		MoveAction,
		FlyAction,
		WalkAction,
		TriggerAction, 
		SqueezeAction, 
		GrabAction, 
		ThumbstickAction, 
		MenuAction, 
		SelectAction, 
		VibrationAction
	};

	struct ActionStateData
	{
		XrActionSet actionSet{ XR_NULL_HANDLE };
		XrAction	aimPoseAction{ XR_NULL_HANDLE };
		XrAction	gripPoseAction{ XR_NULL_HANDLE };
		XrAction	triggerValueAction{ XR_NULL_HANDLE };
		XrAction	triggerClickAction{ XR_NULL_HANDLE };

		XrAction squeezeValueAction{ XR_NULL_HANDLE };
		XrAction squeezeForceAction{ XR_NULL_HANDLE };
		XrAction squeezeClickAction{ XR_NULL_HANDLE };

		XrAction thumbstickAction{ XR_NULL_HANDLE };
		XrAction thumbstickClickAction{ XR_NULL_HANDLE };
		XrAction thumbstickXAction{ XR_NULL_HANDLE };
		XrAction thumbstickYAction{ XR_NULL_HANDLE };
		XrAction menuClickAction{ XR_NULL_HANDLE };
		XrAction selectClickAction{ XR_NULL_HANDLE };
		XrAction triggerAction{ XR_NULL_HANDLE };

		XrAction aClickAction{ XR_NULL_HANDLE };
		XrAction bClickAction{ XR_NULL_HANDLE };
		XrAction xClickAction{ XR_NULL_HANDLE };
		XrAction yClickAction{ XR_NULL_HANDLE };

		XrAction vibrateAction{ XR_NULL_HANDLE };

		XrAction grabAction{ XR_NULL_HANDLE };

		std::vector<XrPath>	 controllerPaths;
		std::vector<XrSpace> controllerReferenceSpaces_aim;
		std::vector<XrSpace> controllerReferenceSpaces_grip;
	};
}; // namespace Spectre

class Controllers final
{
public:
	struct ActionDefinition
	{
		ActionDefinition(std::string actionName, std::string displayName, XrActionType type, XrAction* action, Spectre::InputActions inputAction)
		{
			ActionName = actionName;
			DisplayName = displayName;
			Type = type;
			Action = action;
			InputAction = inputAction;
		}

		std::vector<Spectre::ControllerButtonName> Buttons;

		std::string			  ActionName;
		std::string			  DisplayName;
		XrActionType		  Type;
		XrAction*			  Action;
		Spectre::InputActions InputAction;
	};

	Controllers(){};
	Controllers(XrInstance instance, XrSession m_Session);
	~Controllers();

	bool Sync();

	glm::mat4 GetPose(size_t controllerIndex) const { return m_Poses.at(controllerIndex); }
	float	  GetFlySpeed(size_t controllerIndex) const { return m_FlySpeeds.at(controllerIndex); }

	std::vector<glm::mat4>& GetPoses() { return m_Poses; }
	std::vector<float>&		GetFlySpeeds() { return m_FlySpeeds; }

	const std::vector<XrPath>& GetPaths() { return m_ActionSetData.controllerPaths; };
	size_t					   GetNumberOfController();
	const XrSession&		   GetSession() { return m_Session; };
	std::vector<XrSpace>&	   GetSpaces() { return m_Spaces; };

	std::map<std::string, std::unique_ptr<ActionDefinition>>& GetActions() { return m_Actions; };

private:
	XrSession				 m_Session{ nullptr };
	std::vector<XrSpace>	 m_Spaces;
	std::vector<glm::mat4>	 m_Poses;
	std::vector<float>		 m_FlySpeeds;
	Spectre::ActionStateData m_ActionSetData{};

	std::map<std::string, std::unique_ptr<ActionDefinition>> m_Actions{};

	void AddAction(std::string actionName, std::string displayName, XrActionType type, XrAction* action, Spectre::InputActions inputAction);
	void BindActions(const XrInstance& instance);
	void RegisterInputs();
	void CreateActions();
};