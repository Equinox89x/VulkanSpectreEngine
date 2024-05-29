#pragma once
#include <glfw/glfw3.h>
#include <glm/common.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/mat4x4.hpp>
#include <openxr/openxr.h>
#include <vector>
#include "../VR/Controllers.h"
#include "../VR/Headset.h"

class Command
{
public:
	explicit Command() = default;
	virtual ~Command() = default;
	Command(const Command&) = delete;
	Command(Command&&) = delete;
	Command& operator=(const Command&) = delete;
	Command& operator=(Command&&) = delete;

	virtual void Update(size_t controllerIndex, Headset* headset, const XrPath& path/*, const XrActionStateFloat& state*/) = 0;
	virtual void Init() = 0;

	//virtual XrAction*	 GetAction() = 0;
};

class InputAction : Command
{
public:
	InputAction(Controllers* controller, XrAction* action) : m_Controller{ controller }, m_Action{ action } {};

	void	  Update(size_t controllerIndex, Headset* headset, const XrPath& path/*, const XrActionStateFloat& state*/) override = 0;
	void	  Init() override = 0;
	XrAction* GetAction() { return m_Action; }

protected:
	XrAction*	 m_Action{ nullptr };
	Controllers* m_Controller{ nullptr };
};

class MoveAction : public InputAction
{
public:
	MoveAction(Controllers* controller, XrAction* action) : InputAction(controller, action){};
	void	  Init() override;
	void	  Update(size_t controllerIndex, Headset* headset, const XrPath& path /*, const XrActionStateFloat& state*/) override;
//	XrAction* GetAction() override { return m_Action; }
//
//private:
//	XrAction*	 m_Action{ nullptr };
//	Controllers* m_Controller{ nullptr };
};

class FlyAction : public InputAction
{
public:
	FlyAction(Controllers* controller, XrAction* action) : InputAction(controller, action){};
	void	  Init() override {}
	void	  Update(size_t controllerIndex, Headset* headset, const XrPath& path /*, const XrActionStateFloat& state*/) override;
//	XrAction* GetAction() override { return m_Action; }
//
//private:
//	XrAction*	 m_Action{ nullptr };
//	Controllers* m_Controller{ nullptr };
};

class WalkAction : public InputAction
{
public:
	WalkAction(Controllers* controller, XrAction* action) : InputAction(controller, action){};
	void Init() override {}
	void Update(size_t controllerIndex, Headset* headset, const XrPath& path/*, const XrActionStateFloat& state*/) override;
//	XrAction* GetAction() override { return m_Action; }
//
//private:
//	XrAction*	 m_Action{ nullptr };
//	Controllers* m_Controller{ nullptr };
};