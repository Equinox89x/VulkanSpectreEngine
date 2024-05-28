#pragma once
#ifndef singleton
	#include "../Misc/Singleton.h"
	#define singleton
#endif
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/trigonometric.hpp>

struct GameObject;

class LightSystem : public Singleton<LightSystem>
{
public:
	void	  Update(GameObject* sun);
	glm::vec3 GetLightDirection() { return m_LightDirection; }

private:
	glm::vec3 m_LightOrigin{ 0, 5, 20 };
	glm::vec3 m_MapCenter{ 0, 0, 0 };
	glm::vec3 m_LightDirection{ 1.0f, -1.0f, -1.0f };

	float m_Angle{ 0 };
	float m_Speed{ 10 };

	void UpdateAngle();
	void RotateSun(GameObject* sun);
};
