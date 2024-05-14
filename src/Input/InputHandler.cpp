#include "InputHandler.h"
#include <limits>

void Spectre::InputHandler::HandleKeyboard(GLFWwindow* window, float deltaTime, GameObject& gameObject)
{

	glm::vec3 rotate{ 0 };
	if(glfwGetKey(window, keys.lookRight) == GLFW_PRESS)
	{
		rotate.y += 1.f;
	}
	if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS)
	{
		rotate.y -= 1.f;
	}
	if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS)
	{
		rotate.x += 1.f;
	}
	if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS)
	{
		rotate.x -= 1.f;
	}

	if(glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
	{
		gameObject.m_Transform.rotation += lookSpeed * deltaTime * glm::normalize(rotate);
	}

	gameObject.m_Transform.rotation.x = glm::clamp(gameObject.m_Transform.rotation.x, -1.5f, 1.5f);
	gameObject.m_Transform.rotation.y = glm::mod(gameObject.m_Transform.rotation.y, glm::two_pi<float>());

	float yaw = gameObject.m_Transform.rotation.y;
	const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
	const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
	const glm::vec3 upDir{ 0.f, -1.f, 0.f };

	glm::vec3 moveDir{ 0.f };

	if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS)
	{
		moveDir += forwardDir;
	}
	if(glfwGetKey(window, keys.moveBackward) == GLFW_PRESS)
	{
		moveDir -= forwardDir;
	}
	if(glfwGetKey(window, keys.moveRight) == GLFW_PRESS)
	{
		moveDir += rightDir;
	}
	if(glfwGetKey(window, keys.moveLeft) == GLFW_PRESS)
	{
		moveDir -= rightDir;
	}
	if(glfwGetKey(window, keys.moveUp) == GLFW_PRESS)
	{
		moveDir += upDir;
	}
	if(glfwGetKey(window, keys.moveDown) == GLFW_PRESS)
	{
		moveDir -= upDir;
	}

	if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
	{
		gameObject.m_Transform.translation += moveSpeed * deltaTime * glm::normalize(moveDir);
	}
}

void Spectre::InputHandler::HandleMouse(GLFWwindow* window, float deltaTime, GameObject& gameObject)
{
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		static double lastX = xpos;
		static double lastY = ypos;
		double deltaX = xpos - lastX;
		double deltaY = ypos - lastY;
		lastX = xpos;
		lastY = ypos;

		gameObject.m_Transform.rotation.y += static_cast<float>(deltaX) * mouseSensitivity;
		gameObject.m_Transform.rotation.x += static_cast<float>(-deltaY) * mouseSensitivity;

		gameObject.m_Transform.rotation.x = glm::clamp(gameObject.m_Transform.rotation.x, -glm::pi<float>() / 2.0f, glm::pi<float>() / 2.0f);
	}
}
