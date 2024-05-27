#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <string>
#include <vector>
#include "GameData.h"


struct Vertex final
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
};

struct Vertex2D final
{
	glm::vec2 position;
	glm::vec3 color;
};


class MeshData final
{
public:
	enum class Color
	{
		White,
		FromNormals
	};

	bool LoadModel(const std::string& filename, Color color, std::vector<Model*>& models, size_t count);

	bool CreateTriangle(std::vector<Model*>& models, size_t count);
	bool CreateSquare(std::vector<Model*>& models, size_t count);
	bool CreateOval(int numSegments, float width, float height, std::vector<Model*>& models, size_t count);
	bool CreateRoundedRectangle(int numSegments, float width, float height, float cornerRadius, std::vector<Model*>& models, size_t count);

	size_t GetSize() const { return sizeof(m_Vertices.at(0u)) * m_Vertices.size() + sizeof(m_Indices.at(0u)) * m_Indices.size(); }
	size_t GetIndexOffset() const { return sizeof(m_Vertices.at(0u)) * m_Vertices.size(); }

	void WriteTo(char* destination) const;

private:
	std::vector<Vertex>	  m_Vertices;
	std::vector<uint32_t> m_Indices;

	int m_CurrentIndex{ 0 };

	void HandleModelData(std::vector<Model*>& models, size_t count, const size_t oldIndexCount, std::string fileName = "");
};