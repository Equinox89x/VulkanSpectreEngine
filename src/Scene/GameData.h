#pragma once

#include "../VR/Headset.h"
#include "../VulkanBase/VulkanPipeline.h"
#include "../VulkanBase/VulkanRenderSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <string>

struct Model final
{
	size_t FirstIndex{ 0u };
	size_t IndexCount{ 0u };
};

struct Material
{
	VulkanRenderSystem::DynamicVertexUniformData dynamicUniformData{};
	std::string									 vertShaderName{ "shaders/Diffuse.vert.spv" };
	std::string									 fragShaderName{ "shaders/Diffuse.frag.spv" };
	Spectre::PipelineMaterialPayload			 pipelineData{};
	VulkanPipeline*								 pipeline{ nullptr };
};

struct ShadowMap
{
	VkImage		  image;
	VkImageView	  imageView;
	VkSampler	  sampler;
	VkFramebuffer framebuffer;
	VkExtent2D	  extent;
};

struct GameObject
{
	GameObject(Model* model, Material* material, std::string name, bool isVisible = true)
	{
		Name = name;
		Model = model;
		IsVisible = isVisible;
		Is2DShape = false;
		Material = material;
	}

	GameObject(Model* model, Material* material, std::string name, glm::vec2 screenPosition, float size = -12, bool isVisible = true)
	{
		Name = name;
		Model = model;
		IsVisible = isVisible;
		Is2DShape = true;
		Offset = { screenPosition, size };
		Material = material;
	}

	void Update(Headset& headset)
	{
		// Code is located here as the 2D shapes must act as VR UI, this uses the 2D pipeline with depthtesting disabled
		if (Is2DShape)
		{
			// Offsets the object from the headset center
			auto offsetMatrix{ glm::translate(headset.worldMatrix, Offset) };
			WorldMatrix = offsetMatrix;

			// Calculates the vector between origin and current location
			glm::vec3 centerOffset{ glm::vec3(WorldMatrix[3]) - (glm::vec3(WorldMatrix[3]) + Offset) };
			centerOffset = glm::normalize(centerOffset);

			// Calculates the rotation needed to make the object look towards the headset center
			float yaw{ glm::degrees(atan2(centerOffset.x, centerOffset.z)) };
			float pitch{ glm::degrees(asin(-centerOffset.y)) };

			glm::mat4 localRotationMatrix{ glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f)) };
			localRotationMatrix = glm::rotate(localRotationMatrix, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
			WorldMatrix *= localRotationMatrix;
		}
	}

	std::string Name{ "game object" };
	bool		IsVisible{ true };
	glm::mat4	WorldMatrix{ glm::mat4(1.0f) };
	Model*		Model{ nullptr };
	Material*	Material{ nullptr };
	bool		Is2DShape{ false };
	glm::vec3	Offset{ 0, 0, -12 };
};
