#include "Camera.h"

// std includes
#include <cassert>
#include <limits>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Spectre
{
	void Camera::SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far) {
		m_ProjectionMatrix = glm::ortho(left, right, bottom, top, near, far);
	}

	void Camera::SetPerspectiveProjection(float fovY, float aspect, float near, float far) {
		m_ProjectionMatrix = glm::perspectiveLH(fovY, aspect, near, far);
	}

	void Camera::SetViewMatrix(glm::vec3& position, const glm::vec3& forward, const glm::vec3& up /*, const glm::vec3& right*/) {
		m_ViewMatrix = glm::lookAt(position, position + forward, up);
	}

	void Camera::SetViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up)
	{
		const glm::vec3 forward = glm::normalize(direction);
		const glm::vec3 upVector = glm::normalize(glm::cross(forward, up));
		//const glm::vec3 right = glm::cross(up, forward);
		SetViewMatrix(position, forward, upVector/*, right*/);
	}

	void Camera::SetViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up)
	{
		SetViewDirection(position, target - position, up);
	}

	void Camera::SetViewYXZ(glm::vec3 position, glm::vec3 rotation)
	{
		const glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0, 1, 0)) *
			glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1, 0, 0)) *
			glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0, 0, 1));

		glm::vec3 forward = glm::normalize(glm::vec3(rotationMatrix[2]));
		glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
		glm::vec3 up = glm::cross(right, forward);
		SetViewMatrix(position, -forward, up/*, right*/);
	}
}
