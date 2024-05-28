#include "LightSystem.h"
#include "../Misc/Timer.h"
#include "../Scene/GameData.h"

void LightSystem::Update(GameObject* sun) {

	RotateSun(sun);

	m_LightDirection = -glm::normalize(glm::vec3(sun->WorldMatrix[3]) - glm::vec3(0, 0, 0));

	UpdateAngle();
}

void LightSystem::UpdateAngle()
{
	m_Angle += Timer::GetInstance().GetDeltaTime() * m_Speed;
	if (m_Angle >= 360)
	{
		m_Angle = 0;
	}
}

void LightSystem::RotateSun(GameObject* sun)
{
	glm::vec3 offset{ 0, 5, 50 };
	glm::mat4 rotationMatrix{ glm::rotate(glm::mat4(1.0f), glm::radians(m_Angle), glm::vec3(0, 1, 0)) };

	glm::vec4 sunPosition{ glm::vec4(offset, 1.0f) };
	sunPosition = rotationMatrix * sunPosition;

	sun->WorldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(sunPosition));
}