#pragma once
#include "../Misc/Singleton.h"
#include "../VR/Headset.h"
#include "../VR/controllers.h"
#include "../VulkanBase/VulkanWindow.h"
#include "InputActions.h"
#include <glfw/glfw3.h>
#include <glm/common.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>

class InputHandler final : public Singleton<InputHandler>
{
public:
	struct KeyMappings
	{
		int moveLeft = GLFW_KEY_A;
		int moveRight = GLFW_KEY_D;
		int moveForward = GLFW_KEY_W;
		int moveBackward = GLFW_KEY_S;
		int moveUp = GLFW_KEY_SPACE;
		int moveDown = GLFW_KEY_LEFT_CONTROL;
		int lookLeft = GLFW_KEY_LEFT;
		int lookRight = GLFW_KEY_RIGHT;
		int lookUp = GLFW_KEY_UP;
		int lookDown = GLFW_KEY_DOWN;

		// int RotateLeft = GLFW_KEY_T;
		// int RotateRight = GLFW_KEY_Y;
		// int RotateHorizontalLeft = GLFW_KEY_U;
		// int RotateHorizontalRight = GLFW_KEY_I;
		// int RotateUp = GLFW_KEY_O;
		// int RotateDown = GLFW_KEY_P;
	};

	void Init(Controllers* controllers, Headset* headset);
	void HandleKeyboard(GLFWwindow* window, float deltaTime, Headset& headset);
	void HandleMouse(GLFWwindow* window, float deltaTime, Headset& headset);
	void Update(float deltaTime);

	void AddAction(InputAction* action)
	{
		if (action)
		{
			m_InputActions.push_back(action);
		}
	};

private:
	float m_MoveSpeed{ 3.f };
	float m_LookSpeed{ 1.5f };
	float m_MouseSensitivity = 0.01f;

	std::vector<InputAction*> m_InputActions;
	Controllers*			  m_Controllers{ nullptr };
	Headset*				  m_Headset{ nullptr };
	KeyMappings				  m_Keys{};
};
