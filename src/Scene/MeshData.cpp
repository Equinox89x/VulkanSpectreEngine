#include "../Scene/MeshData.h"
#include "../Misc/Utils.h"
#include <cstring>
#include <iostream>
#include <tiny_obj_loader.h>

bool MeshData::LoadModel(const std::string& filename, Color color, std::vector<Model*>& models, size_t count)
{
	tinyobj::attrib_t			  attrib;
	std::vector<tinyobj::shape_t> shapes;
	if (!tinyobj::LoadObj(&attrib, &shapes, nullptr, nullptr, nullptr, filename.c_str()))
	{
		(EError::ModelLoadingFailure, filename);
		return false;
	}

	const size_t oldIndexCount{ m_Indices.size() };

	for (const auto& shape : shapes)
	{
		for (const auto& index : shape.mesh.indices)
		{

			Vertex vertex;
			vertex.position = { attrib.vertices[3 * index.vertex_index + 0], attrib.vertices[3 * index.vertex_index + 1], attrib.vertices[3 * index.vertex_index + 2] };

			if (index.normal_index >= 0)
			{
				vertex.normal = { attrib.normals[3 * index.normal_index + 0], attrib.normals[3 * index.normal_index + 1], attrib.normals[3 * index.normal_index + 2] };
			}
			else
			{
				vertex.normal = { 0.0f, 0.0f, 0.0f };
			}

			switch (color)
			{
			case Color::White:
				vertex.color = { 1.0f, 1.0f, 1.0f };
				break;
			case Color::FromNormals:
				vertex.color = vertex.normal;
				break;
			}

			m_Vertices.push_back(vertex);
			m_Indices.push_back(static_cast<uint32_t>(m_Indices.size()));
		}
	}

	HandleModelData(models, count, oldIndexCount, filename);

	return true;
}

void MeshData::WriteTo(char* destination) const
{
	const size_t verticesSize{ sizeof(m_Vertices.at(0u)) * m_Vertices.size() };
	const size_t indicesSize{ sizeof(m_Indices.at(0u)) * m_Indices.size() };
	memcpy(destination, m_Vertices.data(), verticesSize);			   // Vertex section first
	memcpy(destination + verticesSize, m_Indices.data(), indicesSize); // Index section next
}

bool MeshData::CreateTriangle(std::vector<Model*>& models, size_t count)
{
	const size_t oldIndexCount{ m_Indices.size() };

	m_Vertices = { Vertex{ { 0.25f, -0.5f, 0.1f }, { 0, 0, 0 }, { 1.0f, 1.0f, 1.0f } }, Vertex{ { 0.5f, 0.5f, 0.1f }, { 0, 0, 0 }, { 0.0f, 1.0f, 0.0f } }, Vertex{ { -0.5f, 0.5f, 0.1f }, { 0, 0, 0 }, { 0.0f, 0.0f, 1.0f } } };

	m_Indices = { 0, 1, 2 };

	HandleModelData(models, count, oldIndexCount);

	return true;
}

bool MeshData::CreateSquare(std::vector<Model*>& models, size_t count)
{
	const size_t oldIndexCount = m_Indices.size();

	m_Vertices.push_back({ { -1.0f, -1.0f, 0.0f }, { 0, 0, 0 }, { 1.0f, 0.0f, 0.0f } });
	m_Vertices.push_back({ { 1.0f, -1.0f, 0.0f }, { 0, 0, 0 }, { 0.0f, 1.0f, 0.0f } });
	m_Vertices.push_back({ { -1.0f, 1.0f, 0.0f }, { 0, 0, 0 }, { 0.0f, 0.0f, 1.0f } });
	m_Vertices.push_back({ { 1.0f, 1.0f, 0.0f }, { 0, 0, 0 }, { 1.0f, 1.0f, 1.0f } });

	m_Indices.push_back(0);
	m_Indices.push_back(1);
	m_Indices.push_back(2);
	m_Indices.push_back(0);
	m_Indices.push_back(2);
	m_Indices.push_back(3);

	HandleModelData(models, count, oldIndexCount, "customsquare");

	return true;
}

bool MeshData::CreateOval(int numSegments, float width, float height, std::vector<Model*>& models, size_t count)
{
	const size_t oldIndexCount{ m_Indices.size() };

	constexpr float M_PI{ 3.14f };

	for (int i = 0; i <= numSegments; ++i)
	{
		float angle = static_cast<float>(i) / numSegments * 2.0f * M_PI;
		float x = width / 2.0f * std::cos(angle);
		float y = height / 2.0f * std::sin(angle);

		m_Vertices.push_back({ { x, y, 0.1f }, { 0, 0, 0 }, { 0.8f, 0.0f, 0.0 } });
	}

	for (uint16_t i = 0; i < numSegments; ++i)
	{
		m_Indices.push_back(i);
		m_Indices.push_back((i + 1) % numSegments);
		m_Indices.push_back(numSegments);
	}

	HandleModelData(models, count, oldIndexCount);

	return true;
}

bool MeshData::CreateRoundedRectangle(int numSegments, float width, float height, float cornerRadius, std::vector<Model*>& models, size_t count)
{
	const size_t oldIndexCount{ m_Indices.size() };

	constexpr float M_PI{ 3.14f };

	for (int i = 0; i < 4; ++i)
	{
		float xSign = (i == 1 || i == 2) ? 1.0f : -1.0f;
		float ySign = (i == 2 || i == 3) ? 1.0f : -1.0f;

		float xStart = (i == 1 || i == 2) ? -width / 2.0f + cornerRadius : width / 2.0f - cornerRadius;
		float yStart = (i == 2 || i == 3) ? -height / 2.0f + cornerRadius : height / 2.0f - cornerRadius;

		for (int j = 0; j <= numSegments; ++j)
		{
			float angle = static_cast<float>(j) / numSegments * M_PI / 2.0f;
			float x = xStart + xSign * cornerRadius * (1.0f + std::cos(angle));
			float y = yStart + ySign * cornerRadius * (1.0f + std::sin(angle));

			m_Vertices.push_back({ { x, y, 0.1f }, { 0, 0, 0 }, { 1.0f, 1.0f, 1.0f } });
		}
	}

	m_Vertices.push_back({ { -width / 2.0f + cornerRadius, -height / 2.0f, 0.1f }, { 0, 0, 0 }, { 1.0f, 1.0f, 1.0f } });
	m_Vertices.push_back({ { width / 2.0f - cornerRadius, -height / 2.0f, 0.1f }, { 0, 0, 0 }, { 1.0f, 1.0f, 1.0f } });
	m_Vertices.push_back({ { width / 2.0f - cornerRadius, height / 2.0f, 0.1f }, { 0, 0, 0 }, { 1.0f, 1.0f, 1.0f } });
	m_Vertices.push_back({ { -width / 2.0f + cornerRadius, height / 2.0f, 0.1f }, { 0, 0, 0 }, { 1.0f, 1.0f, 1.0f } });

	for (int i = 0; i < 4; ++i)
	{
		int baseIndex = i * (numSegments + 1);
		for (int j = 0; j < numSegments; ++j)
		{
			m_Indices.push_back(baseIndex + j);
			m_Indices.push_back(baseIndex + numSegments + j + 2);
			m_Indices.push_back(baseIndex + j + 1);

			m_Indices.push_back(baseIndex + j);
			m_Indices.push_back(baseIndex + numSegments + j + 1);
			m_Indices.push_back(baseIndex + numSegments + j + 2);
		}
	}

	int baseIndex = 4 * (numSegments + 1);
	for (int i = 0; i < 4; ++i)
	{
		m_Indices.push_back(baseIndex + i);
		m_Indices.push_back(baseIndex + (i + 1) % 4 + 1);
		m_Indices.push_back(baseIndex + (i + 1) % 4);
	}

	HandleModelData(models, count, oldIndexCount);

	return true;
}

void MeshData::HandleModelData(std::vector<Model*>& models, size_t count, const size_t oldIndexCount, std::string fileName)
{
	for (size_t modelIndex = m_CurrentIndex; modelIndex < m_CurrentIndex + count; ++modelIndex)
	{
		std::cout << "Model: " << fileName << " with index: " << modelIndex << std::endl;
		Model* model = models.at(modelIndex);
		model->FirstIndex = oldIndexCount;
		model->IndexCount = m_Indices.size() - oldIndexCount;
	}

	m_CurrentIndex += count;
}
