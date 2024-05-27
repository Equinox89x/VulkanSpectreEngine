#include "VulkanRenderer.h"

#include "../Buffers/DataBuffer.h"
#include "../Misc/Utils.h"
#include "../Scene/GameData.h"
#include "../Scene/MeshData.h"
#include "../VR/Headset.h"
#include "../VulkanBase/RenderTarget.h"
#include "VulkanDevice.h"

namespace Spectre
{
	constexpr size_t m_FramesInFlightCount = 2u;
} // namespace Spectre

VulkanRenderer::VulkanRenderer(const VulkanDevice* device, const Headset* headset, const MeshData* meshData, const std::vector<Material*>& materials, const std::vector<GameObject*>& gameObjects) : m_Device(device), m_Headset(headset), m_GameObjects(gameObjects), m_Materials(materials)
{
	const VkDevice vkDevice = device->GetVkDevice();

	// Create a command pool
	VkCommandPoolCreateInfo commandPoolCreateInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex = device->GetVkDrawQueueFamilyIndex();
	if (vkCreateCommandPool(vkDevice, &commandPoolCreateInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}

	// Create a descriptor pool
	std::array<VkDescriptorPoolSize, 2u> descriptorPoolSizes;

	descriptorPoolSizes.at(0u).type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	descriptorPoolSizes.at(0u).descriptorCount = static_cast<uint32_t>(Spectre::m_FramesInFlightCount);

	descriptorPoolSizes.at(1u).type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSizes.at(1u).descriptorCount = static_cast<uint32_t>(Spectre::m_FramesInFlightCount * 2u);

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
	descriptorPoolCreateInfo.maxSets = static_cast<uint32_t>(Spectre::m_FramesInFlightCount);
	if (vkCreateDescriptorPool(vkDevice, &descriptorPoolCreateInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}

	// Create a descriptor set layout
	std::array<VkDescriptorSetLayoutBinding, 3u> descriptorSetLayoutBindings;

	descriptorSetLayoutBindings.at(0u).binding = 0u;
	descriptorSetLayoutBindings.at(0u).descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	descriptorSetLayoutBindings.at(0u).descriptorCount = 1u;
	descriptorSetLayoutBindings.at(0u).stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	descriptorSetLayoutBindings.at(0u).pImmutableSamplers = nullptr;

	descriptorSetLayoutBindings.at(1u).binding = 1u;
	descriptorSetLayoutBindings.at(1u).descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetLayoutBindings.at(1u).descriptorCount = 1u;
	descriptorSetLayoutBindings.at(1u).stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	descriptorSetLayoutBindings.at(1u).pImmutableSamplers = nullptr;

	descriptorSetLayoutBindings.at(2u).binding = 2u;
	descriptorSetLayoutBindings.at(2u).descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetLayoutBindings.at(2u).descriptorCount = 1u;
	descriptorSetLayoutBindings.at(2u).stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	descriptorSetLayoutBindings.at(2u).pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();
	if (vkCreateDescriptorSetLayout(vkDevice, &descriptorSetLayoutCreateInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}

	// Create a pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutCreateInfo.pSetLayouts = &m_DescriptorSetLayout;
	pipelineLayoutCreateInfo.setLayoutCount = 1u;
	if (vkCreatePipelineLayout(vkDevice, &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS)
	{
		utils::ThrowError(EError::GenericVulkan);
	}

	// Create a render process for each frame in flight
	m_RenderProcesses.resize(Spectre::m_FramesInFlightCount);
	for (VulkanRenderSystem*& renderProcess : m_RenderProcesses)
	{
		renderProcess = new VulkanRenderSystem(device, m_CommandPool, m_DescriptorPool, m_DescriptorSetLayout, m_GameObjects.size());
	}

	// Description for 3D Pipeline
	VkVertexInputBindingDescription vertexInputBindingDescription{};
	vertexInputBindingDescription.binding = 0u;
	vertexInputBindingDescription.stride = sizeof(Vertex);
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription vertexInputAttributePosition{};
	vertexInputAttributePosition.binding = 0u;
	vertexInputAttributePosition.location = 0u;
	vertexInputAttributePosition.format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributePosition.offset = offsetof(Vertex, position);

	VkVertexInputAttributeDescription vertexInputAttributeNormal{};
	vertexInputAttributeNormal.binding = 0u;
	vertexInputAttributeNormal.location = 1u;
	vertexInputAttributeNormal.format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributeNormal.offset = offsetof(Vertex, normal);

	VkVertexInputAttributeDescription vertexInputAttributeColor{};
	vertexInputAttributeColor.binding = 0u;
	vertexInputAttributeColor.location = 2u;
	vertexInputAttributeColor.format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributeColor.offset = offsetof(Vertex, color);

	Spectre::PipelineMaterialPayload pipelineMaterialPayload = {};
	m_Pipelines.resize(3);
	//m_Pipelines[0] = new VulkanPipeline(m_Device, m_PipelineLayout, m_Headset->GetVkRenderPass(), "shaders/Grid.vert.spv", "shaders/Grid.frag.spv", { vertexInputBindingDescription }, { vertexInputAttributePosition, vertexInputAttributeColor }, pipelineMaterialPayload);
	//m_Pipelines[1] = new VulkanPipeline(m_Device, m_PipelineLayout, m_Headset->GetVkRenderPass(), "shaders/Diffuse.vert.spv", "shaders/Diffuse.frag.spv", { vertexInputBindingDescription }, { vertexInputAttributePosition, vertexInputAttributeNormal, vertexInputAttributeColor }, pipelineMaterialPayload);
	//m_Pipelines[2] = new VulkanPipeline(m_Device, m_PipelineLayout, m_Headset->GetVkRenderPass(), "shaders/Diffuse2D.vert.spv", "shaders/Diffuse2D.frag.spv", { vertexInputBindingDescription }, { vertexInputAttributePosition, vertexInputAttributeNormal, vertexInputAttributeColor }, pipelineMaterialPayload, false);

	for (size_t i = 0; i < materials.size(); i++)
	{
		//VulkanPipeline* existingPipeline = FindExistingPipeline(materials[i]->vertShaderName, materials[i]->fragShaderName, materials[i]->pipelineData);
		// std::printf("\n[Renderer][log] pipelining materials: index: {%d}, shader.name: {%s}, exists: {%d}", i, materials[i]->fragShaderName.c_str(), pipelineExistsAt );
		//  [tdbe] default grid pipeline
		//if (i == 0)
		//{
		//	materials[0]->pipeline = m_Pipelines[0];

		//} // [tdbe] default diffuse pipeline
		//else if (existingPipeline)
		//{
		//	materials[i]->pipeline = existingPipeline;
		//} // [tdbe] create a new pipeline from material shader name, with default parameters
		//else
		//{
			m_Pipelines.emplace_back(new VulkanPipeline(m_Device, m_PipelineLayout, m_Headset->GetVkRenderPass(), materials[i]->vertShaderName, materials[i]->fragShaderName, { vertexInputBindingDescription }, { vertexInputAttributePosition, vertexInputAttributeNormal, vertexInputAttributeColor }, materials[i]->pipelineData));
			materials[i]->pipeline = m_Pipelines[m_Pipelines.size() - 1];
		//}

		// if (!materials[i]->pipeline->isValid())
		//{
		//	valid = false;
		//	return;
		// }
	}

	// m_GridPipeline = new VulkanPipeline(m_Device, m_PipelineLayout, m_Headset->GetVkRenderPass(), "shaders/Grid.vert.spv", "shaders/Grid.frag.spv", { vertexInputBindingDescription }, { vertexInputAttributePosition, vertexInputAttributeColor });

	// m_DiffusePipeline = new VulkanPipeline(m_Device, m_PipelineLayout, m_Headset->GetVkRenderPass(), "shaders/Diffuse.vert.spv", "shaders/Diffuse.frag.spv", { vertexInputBindingDescription }, { vertexInputAttributePosition, vertexInputAttributeNormal, vertexInputAttributeColor });

	// m_2DPipeline = new VulkanPipeline(m_Device, m_PipelineLayout, m_Headset->GetVkRenderPass(), "shaders/Diffuse2D.vert.spv", "shaders/Diffuse2D.frag.spv", { vertexInputBindingDescription }, { vertexInputAttributePosition, vertexInputAttributeNormal, vertexInputAttributeColor }, false);

	// Create a vertex index buffer
	CreateVertexIndexBuffer(meshData, m_Device);

	m_IndexOffset = meshData->GetIndexOffset();
}

VulkanRenderer::~VulkanRenderer()
{
	delete m_VertexIndexBuffer;
	// delete m_DiffusePipeline;
	// delete m_GridPipeline;

	for (size_t i = 0; i < m_Pipelines.size(); i++)
	{
		// vkDestroyPipeline(device, materials[i]->pipeline, nullptr);
		delete m_Pipelines[i];
	}

	const VkDevice vkDevice{ m_Device->GetVkDevice() };
	if (vkDevice)
	{
		if (m_PipelineLayout)
		{
			vkDestroyPipelineLayout(vkDevice, m_PipelineLayout, nullptr);
		}

		if (m_DescriptorSetLayout)
		{
			vkDestroyDescriptorSetLayout(vkDevice, m_DescriptorSetLayout, nullptr);
		}

		if (m_DescriptorPool)
		{
			vkDestroyDescriptorPool(vkDevice, m_DescriptorPool, nullptr);
		}
	}

	for (const VulkanRenderSystem* renderProcess : m_RenderProcesses)
	{
		delete renderProcess;
	}

	if (vkDevice && m_CommandPool)
	{
		vkDestroyCommandPool(vkDevice, m_CommandPool, nullptr);
	}
}

void VulkanRenderer::CreateVertexIndexBuffer(const MeshData* meshData, const VulkanDevice* m_Device)
{
	const VkDeviceSize bufferSize = static_cast<VkDeviceSize>(meshData->GetSize());
	DataBuffer*		   stagingBuffer = new DataBuffer(m_Device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, bufferSize);

	char* bufferData = static_cast<char*>(stagingBuffer->MapData());

	meshData->WriteTo(bufferData);
	stagingBuffer->UnmapData();

	m_VertexIndexBuffer = new DataBuffer(m_Device, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferSize);

	stagingBuffer->CopyTo(*m_VertexIndexBuffer, m_RenderProcesses.at(0u)->GetCommandBuffer(), m_Device->GetVkDrawQueue());
	delete stagingBuffer;
}

void VulkanRenderer::Render(const glm::mat4& cameraMatrix, size_t swapchainImageIndex, float time)
{
	m_CurrentRenderProcessIndex = (m_CurrentRenderProcessIndex + 1u) % m_RenderProcesses.size();

	VulkanRenderSystem* renderProcess = m_RenderProcesses.at(m_CurrentRenderProcessIndex);

	const VkFence m_BusyFence = renderProcess->GetBusyFence();
	if (vkResetFences(m_Device->GetVkDevice(), 1u, &m_BusyFence) != VK_SUCCESS)
	{
		return;
	}

	const VkCommandBuffer commandBuffer = renderProcess->GetCommandBuffer();
	if (vkResetCommandBuffer(commandBuffer, 0u) != VK_SUCCESS)
	{
		return;
	}

	VkCommandBufferBeginInfo commandBufferBeginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	if (vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo) != VK_SUCCESS)
	{
		return;
	}

	UpdateUniformBuffers(renderProcess, cameraMatrix);

	renderProcess->staticFragmentUniformData.time = time;
	renderProcess->UpdateUniformBufferData();

	const std::array clearValues = { VkClearValue({ 0.01f, 0.01f, 0.01f, 1.0f }), VkClearValue({ 1.0f, 0u }) };

	VkRenderPassBeginInfo renderPassBeginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	renderPassBeginInfo.renderPass = m_Headset->GetVkRenderPass();
	renderPassBeginInfo.framebuffer = m_Headset->GetRenderTarget(swapchainImageIndex)->GetFramebuffer();
	renderPassBeginInfo.renderArea.offset = { 0, 0 };
	renderPassBeginInfo.renderArea.extent = m_Headset->GetEyeResolution(0u);
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport;
	viewport.x = static_cast<float>(renderPassBeginInfo.renderArea.offset.x);
	viewport.y = static_cast<float>(renderPassBeginInfo.renderArea.offset.y);
	viewport.width = static_cast<float>(renderPassBeginInfo.renderArea.extent.width);
	viewport.height = static_cast<float>(renderPassBeginInfo.renderArea.extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0u, 1u, &viewport);

	VkRect2D scissor;
	scissor.offset = renderPassBeginInfo.renderArea.offset;
	scissor.extent = renderPassBeginInfo.renderArea.extent;
	vkCmdSetScissor(commandBuffer, 0u, 1u, &scissor);

	VkDeviceSize   vertexOffset = 0u;
	const VkBuffer buffer = m_VertexIndexBuffer->getBuffer();
	vkCmdBindVertexBuffers(commandBuffer, 0u, 1u, &buffer, &vertexOffset);
	vkCmdBindIndexBuffer(commandBuffer, buffer, m_IndexOffset, VK_INDEX_TYPE_UINT32);

	DrawModels(renderProcess, commandBuffer);

	vkCmdEndRenderPass(commandBuffer);
}

void VulkanRenderer::DrawModels(VulkanRenderSystem* renderProcess, const VkCommandBuffer& commandBuffer)
{
	const VkDescriptorSet descriptorSet = renderProcess->GetDescriptorSet();
	for (size_t modelIndex = 0u; modelIndex < m_GameObjects.size(); ++modelIndex)
	{
		const GameObject* gameObject = m_GameObjects.at(modelIndex);
		const uint32_t	  uniformBufferOffset = static_cast<uint32_t>(utils::Align(static_cast<VkDeviceSize>(sizeof(VulkanRenderSystem::DynamicVertexUniformData)), m_Device->GetUniformBufferOffsetAlignment()) * static_cast<VkDeviceSize>(modelIndex));
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0u, 1u, &descriptorSet, 1u, &uniformBufferOffset);

		gameObject->Material->pipeline->Bind(commandBuffer);
		/*if (!gameObject->Is2DShape)
		{
			if (modelIndex == 0u)
			{
				m_GridPipeline->Bind(commandBuffer);
			}
			else if (modelIndex == 1u)
			{
				m_DiffusePipeline->Bind(commandBuffer);
			}
		}
		else
		{
			m_2DPipeline->Bind(commandBuffer);
		}*/

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(gameObject->Model->IndexCount), 1u, static_cast<uint32_t>(gameObject->Model->FirstIndex), 0u, 0u);
	}
}

void VulkanRenderer::UpdateUniformBuffers(VulkanRenderSystem* renderProcess, const glm::mat4& cameraMatrix)
{
	for (size_t modelIndex = 0u; modelIndex < m_GameObjects.size(); ++modelIndex)
	{
		renderProcess->dynamicVertexUniformData.at(modelIndex).worldMatrix = m_GameObjects.at(modelIndex)->WorldMatrix;
		renderProcess->dynamicVertexUniformData[modelIndex].colorMultiplier = m_GameObjects.at(modelIndex)->Material->dynamicUniformData.colorMultiplier;

	}

	for (size_t eyeIndex = 0u; eyeIndex < m_Headset->GetEyeCount(); ++eyeIndex)
	{
		renderProcess->staticVertexUniformData.viewProjectionMatrices.at(eyeIndex) = m_Headset->GetEyeProjectionMatrix(eyeIndex) * m_Headset->GetEyeViewMatrix(eyeIndex) * cameraMatrix;
	}
}

VulkanPipeline* VulkanRenderer::FindExistingPipeline(const std::string& vertShader, const std::string& fragShader, const Spectre::PipelineMaterialPayload& pipelineData)
{
	for (const auto& pipeline : m_Pipelines)
	{
		if (pipeline->GetVertShaderName() == vertShader && pipeline->GetFragShaderName() == fragShader)
		{
			if (pipelineData == pipeline->GetPipelineMaterialData())
			{
				return pipeline;
			}
		}
	}
	return nullptr;
}

void VulkanRenderer::submit(bool useSemaphores) const
{
	const VulkanRenderSystem* renderProcess = m_RenderProcesses.at(m_CurrentRenderProcessIndex);
	const VkCommandBuffer	  commandBuffer = renderProcess->GetCommandBuffer();
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		return;
	}

	constexpr VkPipelineStageFlags waitStages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	const VkSemaphore			   drawableSemaphore = renderProcess->GetDrawableSemaphore();
	const VkSemaphore			   presentableSemaphore = renderProcess->GetPresentableSemaphore();
	const VkFence				   busyFence = renderProcess->GetBusyFence();

	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.pWaitDstStageMask = &waitStages;
	submitInfo.commandBufferCount = 1u;
	submitInfo.pCommandBuffers = &commandBuffer;

	if (useSemaphores)
	{
		submitInfo.waitSemaphoreCount = 1u;
		submitInfo.pWaitSemaphores = &drawableSemaphore;
		submitInfo.signalSemaphoreCount = 1u;
		submitInfo.pSignalSemaphores = &presentableSemaphore;
	}

	if (vkQueueSubmit(m_Device->GetVkDrawQueue(), 1u, &submitInfo, busyFence) != VK_SUCCESS)
	{
		return;
	}
}