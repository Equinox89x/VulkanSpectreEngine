#pragma once
#include "Mesh.h"

// libraries
#include <glm/gtc/matrix_transform.hpp>

//std includes
#include <memory>

namespace Spectre
{
	struct TransformComponent
	{
		glm::vec3 translation{};
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::vec3 rotation{};

		glm::mat4 Mat4()
		{
			const float c3{ glm::cos(rotation.z) };
			const float s3{ glm::sin(rotation.z) };
			const float c2{ glm::cos(rotation.x) };
			const float s2{ glm::sin(rotation.x) };
			const float c1{ glm::cos(rotation.y) };
			const float s1{ glm::sin(rotation.y) };
			return glm::mat4
			{
				{
					scale.x * (c1 * c3 + s1 * s2 * s3),
					scale.x * (c2 * s3),
					scale.x * (c1 * s2 * s3 - c3 * s1),
					0.0f,
				},
				{
					scale.y * (c3 * s1 * s2 - c1 * s3),
					scale.y * (c2 * c3),
					scale.y * (c1 * c3 * s2 + s1 * s3),
					0.0f,
				},
				{
					scale.z * (c2 * s1),
					scale.z * (-s2),
					scale.z * (c1 * c2),
					0.0f,
				},
				{translation.x, translation.y, translation.z, 1.0f}
			};
		}
	};

	class GameObject
	{
	public:
		GameObject(const GameObject&) = delete;
		GameObject(GameObject&&) = default;
		GameObject& operator=(const GameObject&) = delete;
		GameObject& operator=(GameObject&&) = default;
		static GameObject CreateGameObject()
		{
			static unsigned int currentId = 0;
			return GameObject(currentId++);
		}

		std::unique_ptr<Mesh> m_Mesh;
		glm::vec3 m_Color{};
		TransformComponent m_Transform{};

		unsigned int GetId() const { return m_ObjectId; }
	private:
		unsigned int m_ObjectId;
		explicit GameObject(unsigned int id) : m_ObjectId(id) {}
	};
}