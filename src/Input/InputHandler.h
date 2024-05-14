#pragma once
#include "Scene/GameObject.h"
#include "VulkanBase/VulkanWindow.h"

namespace Spectre
{
	class InputHandler
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

            //int RotateLeft = GLFW_KEY_T;
            //int RotateRight = GLFW_KEY_Y;
            //int RotateHorizontalLeft = GLFW_KEY_U;
            //int RotateHorizontalRight = GLFW_KEY_I;
            //int RotateUp = GLFW_KEY_O;
            //int RotateDown = GLFW_KEY_P;
		};

        void HandleKeyboard(GLFWwindow* window, float deltaTime, GameObject& gameObject);
        void HandleMouse(GLFWwindow* window, float deltaTime, GameObject& gameObject);

    private:
        KeyMappings keys{};
        float moveSpeed{ 3.f };
        float lookSpeed{ 1.5f };
        float mouseSensitivity = 0.01f;

	};
}

