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

class InputAction
{
public:
	explicit InputAction() = default;
	virtual ~InputAction() = default;
	InputAction(const InputAction&) = delete;
	InputAction(InputAction&&) = delete;
	InputAction& operator=(const InputAction&) = delete;
	InputAction& operator=(InputAction&&) = delete;

	virtual void Update(size_t controllerIndex, Headset* headset, const XrPath& path) = 0;
	virtual void Init() = 0;
};
class MoveAction : public InputAction
{
public:
	MoveAction(Controllers* controller, XrAction* action) : m_Controller{ controller }, m_Action{ action } {};
	void Init();
	void Update(size_t controllerIndex, Headset* headset, const XrPath& path);

private:
	XrAction*	 m_Action{ nullptr };
	Controllers* m_Controller{ nullptr };
};

class FlyAction : public InputAction
{
public:
	FlyAction(Controllers* controller, XrAction* action) : m_Controller{ controller }, m_Action{ action } {};
	void Init() {}
	void Update(size_t controllerIndex, Headset* headset, const XrPath& path);

private:
	XrAction*	 m_Action{ nullptr };
	Controllers* m_Controller{ nullptr };
};

class WalkAction : public InputAction
{
public:
	WalkAction(Controllers* controller, XrAction* action) : m_Controller{ controller }, m_Action{ action } {};
	void Init() {}
	void Update(size_t controllerIndex, Headset* headset, const XrPath& path);

private:
	XrAction*	 m_Action{ nullptr };
	Controllers* m_Controller{ nullptr };
};