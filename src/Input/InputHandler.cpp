#include "InputHandler.h"
#include <limits>

#include "../Misc/Utils.h"
#include "../Misc/Timer.h"

void InputHandler::Init(Controllers* controllers, Headset* headset) {
	m_Controllers = controllers;
	m_Headset = headset;

	for (const auto& inputAction : m_InputActions)
	{
		inputAction->Init();
	}
}

void InputHandler::HandleKeyboard(GLFWwindow* window, Headset& gameObject)
{
	float deltaTime{ Timer::GetInstance().GetDeltaTime() };
	//glm::vec3 rotate{ 0 };
	//if(glfwGetKey(window, m_Keys.lookRight) == GLFW_PRESS)
	//{
	//	rotate.y += 1.f;
	//}
	//if (glfwGetKey(window, m_Keys.lookLeft) == GLFW_PRESS)
	//{
	//	rotate.y -= 1.f;
	//}
	//if (glfwGetKey(window, m_Keys.lookUp) == GLFW_PRESS)
	//{
	//	rotate.x += 1.f;
	//}
	//if (glfwGetKey(window, m_Keys.lookDown) == GLFW_PRESS)
	//{
	//	rotate.x -= 1.f;
	//}

	//if(glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
	//{
	//	gameObject.m_Transform.rotation += m_LookSpeed * deltaTime * glm::normalize(rotate);
	//}

	//gameObject.m_Transform.rotation.x = glm::clamp(gameObject.m_Transform.rotation.x, -1.5f, 1.5f);
	//gameObject.m_Transform.rotation.y = glm::mod(gameObject.m_Transform.rotation.y, glm::two_pi<float>());

	float yaw = gameObject.GetViewerPosition().y;
	const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
	const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
	const glm::vec3 upDir{ 0.f, -1.f, 0.f };

	glm::vec3 moveDir{ 0.f };

	if (glfwGetKey(window, m_Keys.moveForward) == GLFW_PRESS)
	{
		moveDir += forwardDir;
	}
	if(glfwGetKey(window, m_Keys.moveBackward) == GLFW_PRESS)
	{
		moveDir -= forwardDir;
	}
	if(glfwGetKey(window, m_Keys.moveRight) == GLFW_PRESS)
	{
		moveDir += rightDir;
	}
	if(glfwGetKey(window, m_Keys.moveLeft) == GLFW_PRESS)
	{
		moveDir -= rightDir;
	}
	if(glfwGetKey(window, m_Keys.moveUp) == GLFW_PRESS)
	{
		moveDir += upDir;
	}
	if(glfwGetKey(window, m_Keys.moveDown) == GLFW_PRESS)
	{
		moveDir -= upDir;
	}

	if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
	{
		//gameObject.Translate(m_MoveSpeed * deltaTime * glm::normalize(moveDir));
	}
}

void InputHandler::HandleMouse(GLFWwindow* window, Headset& headset)
{
	float deltaTime{ Timer::GetInstance().GetDeltaTime() };

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		static double lastX = xpos;
		static double lastY = ypos;
		double deltaX = xpos - lastX;
		double deltaY = ypos - lastY;
		lastX = xpos;
		lastY = ypos;

		//gameObject.m_Transform.rotation.y += static_cast<float>(deltaX) * m_MouseSensitivity;
		//gameObject.m_Transform.rotation.x += static_cast<float>(-deltaY) * m_MouseSensitivity;

		//gameObject.m_Transform.rotation.x = glm::clamp(gameObject.m_Transform.rotation.x, -glm::pi<float>() / 2.0f, glm::pi<float>() / 2.0f);
	}
}

void InputHandler::Update()
{
	// Update the actions
	const auto& paths{ m_Controllers->GetPaths() };
	for (size_t controllerIndex = 0u; controllerIndex < m_Controllers->GetNumberOfController(); ++controllerIndex)
	{
		const XrPath& path = paths.at(controllerIndex);

		for (auto inputAction : m_InputActions)
		{
			inputAction->Update(controllerIndex, m_Headset, path);
		}
	}
}