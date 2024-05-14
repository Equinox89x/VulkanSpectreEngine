#include "Mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace Spectre
{
	Mesh::Mesh(VulkanDevice& device, const glm::vec2 offset) :
		m_Device{ device }
	{}

	Mesh::Mesh(VulkanDevice& device, const glm::vec3 offset) :
		m_Device{ device }
	{
		m_Is3D = true;
		CreateCube(m_Device, offset);
	}	

	Mesh::Mesh(VulkanDevice& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) :
		m_Device{ device }, m_Vertices{ vertices }, m_Indices{ indices }
	{
		CreateVertexBuffers(vertices);
		CreateIndexBuffer(indices);
	}	
	
	Mesh::Mesh(VulkanDevice& device, uint32_t height, uint32_t width, std::string modelPath, std::string texturePath) : 
		m_Device{ device }, WIDTH{ width }, HEIGHT{ height }, MODEL_PATH{ modelPath }, TEXTURE_PATH{ texturePath }
	{
		m_Is3D = true;
		loadModel();
	}


	Mesh::~Mesh()
	{
		vkDestroyBuffer(m_Device.GetDevice(), m_VertrexBuffer, nullptr);
		vkFreeMemory(m_Device.GetDevice(), m_VertexBufferMemory, nullptr);		

		vkDestroyBuffer(m_Device.GetDevice(), m_IndexBuffer, nullptr);
		vkFreeMemory(m_Device.GetDevice(), m_IndexBufferMemory, nullptr);
	}

	void Mesh::BindBuffers(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffers[] = { m_VertrexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

		vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

	}

	void Mesh::Draw(VkCommandBuffer commandBuffer)
	{
		vkCmdDrawIndexed(commandBuffer, m_IndiceCount, 1, 0, 0, 0);
	}

	void Mesh::loadModel()
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
			throw std::runtime_error(warn + err);
		}

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		for (const auto& shape : shapes) {
			for (const auto& index : shape.mesh.indices) {
				Vertex vertex{};

				vertex.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				/* vertex.texCoord = {
					 attrib.texcoords[2 * index.texcoord_index + 0],
					 1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				 };*/

				vertex.m_Color = { 1.0f, 1.0f, 1.0f };

				if (uniqueVertices.count(vertex) == 0) {
					uniqueVertices[vertex] = static_cast<uint32_t>(m_Vertices.size());
					m_Vertices.push_back(vertex);
				}

				m_Indices.push_back(uniqueVertices[vertex]);
			}
		}

		CreateVertexBuffers(m_Vertices);
		CreateIndexBuffer(m_Indices);
	}

	void Mesh::CreateVertexBuffers(const std::vector<Vertex>& vertices)
	{
		m_VertexCount = static_cast<uint32_t>(vertices.size());
		VkDeviceSize bufferSize{ sizeof(vertices[0]) * vertices.size() };

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		m_Device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_Device.GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_Device.GetDevice(), stagingBufferMemory);

		m_Device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertrexBuffer, m_VertexBufferMemory);

		m_Device.CopyBuffer(stagingBuffer, m_VertrexBuffer, bufferSize);

		vkDestroyBuffer(m_Device.GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(m_Device.GetDevice(), stagingBufferMemory, nullptr);
	}

	void Mesh::CreateVertexBuffers(const std::vector<Vertex2D>& vertices)
	{
		m_VertexCount = static_cast<uint32_t>(vertices.size());
		VkDeviceSize bufferSize{ sizeof(vertices[0]) * vertices.size() };

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		m_Device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_Device.GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_Device.GetDevice(), stagingBufferMemory);

		m_Device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertrexBuffer, m_VertexBufferMemory);

		m_Device.CopyBuffer(stagingBuffer, m_VertrexBuffer, bufferSize);

		vkDestroyBuffer(m_Device.GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(m_Device.GetDevice(), stagingBufferMemory, nullptr);
	}

	void Mesh::CreateIndexBuffer(const std::vector<uint32_t>& indices) {
		m_IndiceCount = static_cast<uint32_t>(indices.size());
		VkDeviceSize bufferSize{ sizeof(indices[0]) * m_IndiceCount };

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		m_Device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(m_Device.GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(m_Device.GetDevice(), stagingBufferMemory);

		m_Device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer, m_IndexBufferMemory);

		m_Device.CopyBuffer(stagingBuffer, m_IndexBuffer, bufferSize);

		vkDestroyBuffer(m_Device.GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(m_Device.GetDevice(), stagingBufferMemory, nullptr);
	}

	void Mesh::CreateTriangle(VulkanDevice& device, glm::vec2 offset) {
		m_Vertices2D = {
			{{0.25f, -0.5f}, {1.0f, 1.0f, 1.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		m_Indices = {
			0, 1, 2
		};

		for (auto& v : m_Vertices2D)
		{
			v.position += offset;
		}

		CreateVertexBuffers(m_Vertices2D);
		CreateIndexBuffer(m_Indices);
	}
	
	void Mesh::CreateSquare(VulkanDevice & device, glm::vec2 offset) {
		m_Vertices2D = {
			{{-0.8f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.8f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{0.8f, 0.5f}, {0.0f, 0.0f, 1.0f}},
			{{-0.8f, 0.5f}, {1.0f, 1.0f, 1.0f}}
		};

		m_Indices = {
			0, 1, 2, 2, 3, 0
		};

		for (auto& v : m_Vertices2D)
		{
			v.position += offset;
		}

		CreateVertexBuffers(m_Vertices2D);
		CreateIndexBuffer(m_Indices);
	}	

	void Mesh::CreateOval(VulkanDevice & device, glm::vec2 offset, int numSegments, float width, float height) {
		constexpr float M_PI = 3.14f;

		for (int i = 0; i <= numSegments; ++i) {
			float angle = static_cast<float>(i) / numSegments * 2.0f * M_PI;
			float x = width / 2.0f * std::cos(angle);
			float y = height / 2.0f * std::sin(angle);

			m_Vertices2D.push_back({ {x, y}, {0.8f, 0.0f, 0.0} });
		}

		for (uint16_t i = 0; i < numSegments; ++i) {
			m_Indices.push_back(i);
			m_Indices.push_back((i + 1) % numSegments);
			m_Indices.push_back(numSegments);
		}

		for (auto& v : m_Vertices2D)
		{
			v.position += offset;
		}

		CreateVertexBuffers(m_Vertices2D);
		CreateIndexBuffer(m_Indices);
	}
	
	void Mesh::CreateRoundedRectangle(VulkanDevice & device, glm::vec2 offset, int numSegments, float width, float height, float cornerRadius) {
		constexpr float M_PI = 3.14f;

		for (int i = 0; i < 4; ++i) {
			float xSign = (i == 1 || i == 2) ? 1.0f : -1.0f;
			float ySign = (i == 2 || i == 3) ? 1.0f : -1.0f;


			float xStart = (i == 1 || i == 2) ? -width / 2.0f + cornerRadius : width / 2.0f - cornerRadius;
			float yStart = (i == 2 || i == 3) ? -height / 2.0f + cornerRadius : height / 2.0f - cornerRadius;

			for (int j = 0; j <= numSegments; ++j) {
				float angle = static_cast<float>(j) / numSegments * M_PI / 2.0f;
				float x = xStart + xSign * cornerRadius * (1.0f + std::cos(angle));
				float y = yStart + ySign * cornerRadius * (1.0f + std::sin(angle));

				m_Vertices2D.push_back({ {x, y}, {1.0f, 1.0f, 1.0f} });
			}
		}

		m_Vertices2D.push_back({ {-width / 2.0f + cornerRadius, -height / 2.0f}, {1.0f, 1.0f, 1.0f} });
		m_Vertices2D.push_back({ {width / 2.0f - cornerRadius, -height / 2.0f}, {1.0f, 1.0f, 1.0f} });
		m_Vertices2D.push_back({ {width / 2.0f - cornerRadius, height / 2.0f}, {1.0f, 1.0f, 1.0f} });
		m_Vertices2D.push_back({ {-width / 2.0f + cornerRadius, height / 2.0f}, {1.0f, 1.0f, 1.0f} });

		for (int i = 0; i < 4; ++i) {
			int baseIndex = i * (numSegments + 1);
			for (int j = 0; j < numSegments; ++j) {
				m_Indices.push_back(baseIndex + j);
				m_Indices.push_back(baseIndex + numSegments + j + 2);
				m_Indices.push_back(baseIndex + j + 1);

				m_Indices.push_back(baseIndex + j);
				m_Indices.push_back(baseIndex + numSegments + j + 1);
				m_Indices.push_back(baseIndex + numSegments + j + 2);
			}
		}

		int baseIndex = 4 * (numSegments + 1);
		for (int i = 0; i < 4; ++i) {
			m_Indices.push_back(baseIndex + i);
			m_Indices.push_back(baseIndex + (i + 1) % 4 + 1);
			m_Indices.push_back(baseIndex + (i + 1) % 4);
		}

		for (auto& v : m_Vertices2D)
		{
			v.position += offset;
		}

		CreateVertexBuffers(m_Vertices2D);
		CreateIndexBuffer(m_Indices);
	}
	
	void Mesh::CreateCube(VulkanDevice& device, glm::vec3 offset) {
		m_Vertices = {

			// left (white)
			{{-.5f, -.5f, -.5f}, {1.f, 1.f, 1.f}},
			{{-.5f, .5f, .5f}, {1.f, 1.f, 1.f}},
			{{-.5f, -.5f, .5f}, {1.f, 1.f, 1.f}},
			{{-.5f, -.5f, -.5f}, {1.f, 1.f, 1.f}},
			{{-.5f, .5f, -.5f}, {1.f, 1.f, 1.f}},
			{{-.5f, .5f, .5f}, {1.f, 1.f, 1.f}},

			// right (red)
			{{.5f, -.5f, -.5f}, {1.f, 0.f, 0.f}},
			{{.5f, .5f, .5f}, {1.f, 0.f, 0.f}},
			{{.5f, -.5f, .5f}, {1.f, 0.f, 0.f}},
			{{.5f, -.5f, -.5f}, {1.f, 0.f, 0.f}},
			{{.5f, .5f, -.5f}, {1.f, 0.f, 0.f}},
			{{.5f, .5f, .5f}, {1.f, 0.f, 0.f}},

			// top (green)
			{{-.5f, -.5f, -.5f}, {0.f, 1.f, 0.f}},
			{{.5f, -.5f, .5f}, {0.f, 1.f, 0.f}},
			{{-.5f, -.5f, .5f}, {0.f, 1.f, 0.f}},
			{{-.5f, -.5f, -.5f}, {0.f, 1.f, 0.f}},
			{{.5f, -.5f, -.5f}, {0.f, 1.f, 0.f}},
			{{.5f, -.5f, .5f}, {0.f, 1.f, 0.f}},

			// bottom (blue)
			{{-.5f, .5f, -.5f}, {0.f, 0.f, 1.f}},
			{{.5f, .5f, .5f}, {0.f, 0.f, 1.f}},
			{{-.5f, .5f, .5f}, {0.f, 0.f, 1.f}},
			{{-.5f, .5f, -.5f}, {0.f, 0.f, 1.f}},
			{{.5f, .5f, -.5f}, {0.f, 0.f, 1.f}},
			{{.5f, .5f, .5f}, {0.f, 0.f, 1.f}},

			// front
			{{-.5f, -.5f, 0.5f}, {1.f, 1.f, 0.f}},
			{{.5f, .5f, 0.5f}, {1.f, 1.f, 0.f}},
			{{-.5f, .5f, 0.5f}, {1.f, 1.f, 0.f}},
			{{-.5f, -.5f, 0.5f}, {1.f, 1.f, 0.f}},
			{{.5f, -.5f, 0.5f}, {1.f, 1.f, 0.f}},
			{{.5f, .5f, 0.5f}, {1.f, 1.f, 0.f}},

			// back
			{{-.5f, -.5f, -0.5f}, {1.f, 0.f, 1.f}},
			{{.5f, .5f, -0.5f}, {1.f, 0.f, 1.f}},
			{{-.5f, .5f, -0.5f}, {1.f, 0.f, 1.f}},
			{{-.5f, -.5f, -0.5f}, {1.f, 0.f, 1.f}},
			{{.5f, -.5f, -0.5f}, {1.f, 0.f, 1.f}},
			{{.5f, .5f, -0.5f}, {1.f, 0.f, 1.f}},

		};

		m_Indices = {
			0, 1, 2,
			1, 3, 2,

			// Back face
			4, 6, 5,
			5, 6, 7,

			// Left face
			8, 9, 10,
			9, 11, 10,

			// Right face
			12, 14, 13,
			13, 14, 15,

			// Top face
			16, 18, 17,
			17, 18, 19,

			// Bottom face
			20, 22, 21,
			21, 22, 23
		};

		for (auto& v : m_Vertices)
		{
			v.position += offset;
		}

		CreateVertexBuffers(m_Vertices);
		CreateIndexBuffer(m_Indices);
	}
}
