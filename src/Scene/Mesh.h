#pragma once
#include "VulkanBase/VulkanDevice.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// std include
#include <vector>
#include <unordered_map>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 m_Color;

	//static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
	//static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

	static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescription(1);
		bindingDescription[0].binding = 0;
		bindingDescription[0].stride = sizeof(Vertex);
		bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, m_Color);

		return attributeDescriptions;
	}

	bool operator==(const Vertex& other) const {
		return position == other.position && m_Color == other.m_Color /*&& texCoord == other.texCoord*/;
	}
};

struct Vertex2D
{
	glm::vec2 position;
	glm::vec3 m_Color;

	//static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
	//static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

	static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescription(1);
		bindingDescription[0].binding = 0;
		bindingDescription[0].stride = sizeof(Vertex2D);
		bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex2D, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex2D, m_Color);

		return attributeDescriptions;
	}

	bool operator==(const Vertex2D& other) const {
		return position == other.position && m_Color == other.m_Color /*&& texCoord == other.texCoord*/;
	}
};

namespace Spectre
{
	class Mesh
	{
	public:
		
		Mesh(VulkanDevice& device, const glm::vec3 offset);
		Mesh(VulkanDevice& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		Mesh(VulkanDevice& device, uint32_t height, uint32_t width, std::string modelPath, std::string texturePath);
		Mesh(VulkanDevice& device, const glm::vec2 offset);
		~Mesh();
		Mesh(const Mesh&) = delete;
		Mesh(Mesh&&) = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(Mesh&&) = delete;

		void BindBuffers(VkCommandBuffer commandBuffer);
		void Draw(VkCommandBuffer commandBuffer);
		void loadModel();
		
		void CreateCube(VulkanDevice& device, glm::vec3 offset);
		
		void CreateSquare(VulkanDevice& device, glm::vec2 offset);
		void CreateTriangle(VulkanDevice& device, glm::vec2 offset);
		void CreateRoundedRectangle(VulkanDevice& device, glm::vec2 offset, int numSegments = 40, float width = 1.f, float height = 1.f, float cornerRadius = 0.5f);
		void CreateOval(VulkanDevice& device, glm::vec2 offset, int numSegments = 40, float width = 1.f, float height = 0.5f);

	private:
		VulkanDevice& m_Device;
		bool m_Is3D{ false };


		std::vector<Vertex> m_Vertices;
		std::vector<Vertex2D> m_Vertices2D;
		std::vector<uint32_t> m_Indices;
		uint32_t m_VertexCount;
		uint32_t m_IndiceCount;

		VkBuffer m_VertrexBuffer;
		VkDeviceMemory m_VertexBufferMemory;

		VkBuffer m_IndexBuffer;
		VkDeviceMemory m_IndexBufferMemory;

		const uint32_t WIDTH = 800;
		const uint32_t HEIGHT = 600;
		const std::string MODEL_PATH = "Models/vikingroom.obj";
		const std::string TEXTURE_PATH = "Textures/viking_room.png";

		void CreateVertexBuffers(const std::vector<Vertex2D>& vertices);
		void CreateVertexBuffers(const std::vector<Vertex>& vertices);
		void CreateIndexBuffer(const std::vector<uint32_t>& indices);
	};
}

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.position) ^ (hash<glm::vec3>()(vertex.m_Color) << 1)) >> 1) /*^ (hash<glm::vec2>()(vertex.texCoord) << 1)*/;
		}
	};	
	
	template<> struct hash<Vertex2D> {
		size_t operator()(Vertex2D const& vertex) const {
			return ((hash<glm::vec2>()(vertex.position) ^ (hash<glm::vec3>()(vertex.m_Color) << 1)) >> 1) /*^ (hash<glm::vec2>()(vertex.texCoord) << 1)*/;
		}
	};
}
