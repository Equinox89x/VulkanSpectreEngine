#include "App.h"
#include "../Input/InputHandler.h"
#include "../Misc/Utils.h"
#include "../Scene/GameData.h"
#include "../Scene/MeshData.h"
#include "../VR/Controllers.h"
#include "../VulkanBase/VulkanDevice.h"
#include "../VulkanBase/VulkanRenderer.h"
#include "../Light/LightSystem.h"
// #include "../VulkanBase/VulkanWindow.h"
#include <chrono>
#include <iostream>

App::~App()
{
	// for (auto model : m_Models)
	//{
	//     delete model;
	// }
	// m_Models.clear();
}

int App::Run()
{
	VulkanDevice device;
	VulkanWindow window(&device);

	device.CreateXRDevice(window.GetSurface());

	Headset		headset(&device);
	Controllers controllers(device.GetXrInstance(), headset.GetXrSession());

	Model				gridModel, ruinsModel, carModelLeft, carModelRight, beetleModel, bikeModel, handModelLeft, handModelRight, planeModelLeft, planeModelRight, squareModel, sunModel;
	std::vector<Model*> models = { &gridModel, &ruinsModel, &carModelLeft, &carModelRight, &sunModel, & beetleModel, &bikeModel, &handModelLeft, &handModelRight, &planeModelLeft, &planeModelRight, &squareModel };

	Material gridMaterial, diffuseMaterial, transparentMaterial, material2D, sunMaterial = {};
	gridMaterial.vertShaderName = "shaders/Grid.vert.spv";
	gridMaterial.fragShaderName = "shaders/Grid.frag.spv";
	gridMaterial.dynamicUniformData.colorMultiplier = glm::vec4(1.0f);

	diffuseMaterial.vertShaderName = "shaders/Diffuse.vert.spv";
	diffuseMaterial.fragShaderName = "shaders/Diffuse.frag.spv";
	diffuseMaterial.dynamicUniformData.colorMultiplier = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

	sunMaterial.vertShaderName = "shaders/Illumination.vert.spv";
	sunMaterial.fragShaderName = "shaders/Illumination.frag.spv";
	sunMaterial.dynamicUniformData.colorMultiplier = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);

	transparentMaterial.vertShaderName = "shaders/DiffuseTransparent.vert.spv";
	transparentMaterial.fragShaderName = "shaders/DiffuseTransparent.frag.spv";
	transparentMaterial.dynamicUniformData.colorMultiplier = glm::vec4(0.0f, 0.8f, 0.f, 0.66f);
	transparentMaterial.pipelineData.cullMode = VkCullModeFlagBits::VK_CULL_MODE_NONE;

	material2D.vertShaderName = "shaders/Diffuse2D.vert.spv";
	material2D.fragShaderName = "shaders/Diffuse2D.frag.spv";
	material2D.dynamicUniformData.colorMultiplier = glm::vec4(1.0f, 0.0f, 0.1f, 0.66f);
	material2D.pipelineData.depthTestEnable = VK_FALSE;
	material2D.pipelineData.depthWriteEnable = VK_FALSE;
	std::vector<Material*> materials = { &gridMaterial, &diffuseMaterial, &transparentMaterial, &material2D, &sunMaterial };

	GameObject				 grid{ &gridModel, &gridMaterial, "grid" };
	GameObject				 ruins{ &ruinsModel, &diffuseMaterial, "ruins" };
	GameObject				 carLeft{ &carModelLeft, &diffuseMaterial, "carLeft" };
	GameObject				 carRight{ &carModelRight, &diffuseMaterial, "carRight" };
	GameObject				 sun{ &sunModel, &sunMaterial, "sun" };
	GameObject				 beetle{ &beetleModel, &diffuseMaterial, "beetle" };
	GameObject				 bike{ &bikeModel, &transparentMaterial, "bike" };
	GameObject				 handLeft{ &handModelLeft, &diffuseMaterial, "handLeft" };
	GameObject				 handRight{ &handModelRight, &diffuseMaterial, "handRight" };
	GameObject				 planeLeft{ &planeModelLeft, &material2D, "planeLeft", glm::vec2{ -4.0f, -4.0f } };
	GameObject				 planeRight{ &planeModelRight, &material2D, "planeRight", glm::vec2{ 4.0f, 4.0f } };
	GameObject				 square{ &squareModel, &material2D, "square", glm::vec2{ 4.0f, 4.0f } };
	std::vector<GameObject*> gameObjects = { &grid, &ruins, &carLeft, &carRight, &sun, &beetle, &bike, &handLeft, &handRight, &planeLeft, &planeRight, &square };

	carLeft.WorldMatrix = glm::rotate(glm::translate(glm::mat4(1.0f), { -3.5f, 0.0f, -7.0f }), glm::radians(75.0f), { 0.0f, 1.0f, 0.0f });
	carRight.WorldMatrix = glm::rotate(glm::translate(glm::mat4(1.0f), { 8.0f, 0.0f, -15.0f }), glm::radians(-15.0f), { 0.0f, 1.0f, 0.0f });
	beetle.WorldMatrix = glm::rotate(glm::translate(glm::mat4(1.0f), { -3.5f, 0.0f, -0.5f }), glm::radians(-125.0f), { 0.0f, 1.0f, 0.0f });

	MeshData* meshData = new MeshData;
	meshData->LoadModel("models/Grid.obj", MeshData::Color::FromNormals, models, 1u);
	meshData->LoadModel("models/Ruins.obj", MeshData::Color::White, models, 1u);
	meshData->LoadModel("models/Car.obj", MeshData::Color::White, models, 3u);
	meshData->LoadModel("models/Beetle.obj", MeshData::Color::White, models, 1u);
	meshData->LoadModel("models/Bike.obj", MeshData::Color::White, models, 1u);
	meshData->LoadModel("models/Hand.obj", MeshData::Color::White, models, 2u);
	meshData->LoadModel("models/Plane.obj", MeshData::Color::White, models, 2u);
	meshData->CreateSquare(models, 1u);

	VulkanRenderer renderer(&device, &headset, meshData, materials, gameObjects);
	delete meshData;

	window.Connect(&headset, &renderer);
	InputHandler::GetInstance().Init(&controllers, &headset);

	// Main loop
	Timer::GetInstance().Start();
	while (!headset.IsExitRequested() && !window.IsExitRequested())
	{
		Timer::GetInstance().Update();

		window.ProcessWindowEvents();

		uint32_t						swapchainImageIndex;
		const Headset::BeginFrameResult frameResult{ headset.BeginFrame(swapchainImageIndex) };
		if (frameResult == Headset::BeginFrameResult::ThrowError)
		{
			return EXIT_FAILURE;
		}
		else if (frameResult == Headset::BeginFrameResult::RenderFully)
		{
			// Sync
			if (!controllers.Sync())
			{
				return EXIT_FAILURE;
			}

			static float time{ 0.0f };
			time += Timer::GetInstance().GetDeltaTime();

			// Update
			LightSystem::GetInstance().Update(&sun);
			InputHandler::GetInstance().Update();
			UpdateControllers(headset, controllers, handRight, handLeft);
			UpdateGameObjects(gameObjects, headset);
			UpdateObjects(time, bike);

			// Render
			renderer.Render(headset.cameraMatrix, swapchainImageIndex, time, LightSystem::GetInstance().GetLightDirection());

			// Present
			if (!PresentImage(window, swapchainImageIndex, renderer))
			{
				return EXIT_FAILURE;
			}
		}

		if (frameResult == Headset::BeginFrameResult::RenderFully || frameResult == Headset::BeginFrameResult::SkipRender)
		{
			headset.EndFrame();
		}
	}

	// Sync before destroying so that resources are free
	device.Sync();
	Timer::GetInstance().Stop();
	return EXIT_SUCCESS;
}

int App::PresentImage(VulkanWindow& window, const uint32_t& swapchainImageIndex, VulkanRenderer& renderer)
{
	VulkanWindow::RenderResult windowResult{ window.Render(swapchainImageIndex) };
	if (windowResult == VulkanWindow::RenderResult::ThrowError)
	{
		return false;
	}

	// Present
	bool windowIsVisible = (windowResult == VulkanWindow::RenderResult::Visible);
	renderer.Submit(windowIsVisible);

	if (windowIsVisible)
	{
		window.Present();
	}
	return true;
}

void App::UpdateGameObjects(std::vector<GameObject*>& gameObjects, Headset& headset)
{
	for (auto& object : gameObjects)
	{
		object->Update(headset);
	}
}

void App::UpdateObjects(float time, GameObject& bikeModel) { bikeModel.WorldMatrix = glm::rotate(glm::translate(glm::mat4(1.0f), { 0.5f, 0.0f, -4.5f }), time * 0.2f, { 0.0f, 1.0f, 0.0f }); }

void App::UpdateControllers(Headset& headset, Controllers& controllers, GameObject& handModelRight, GameObject& handModelLeft)
{
	const glm::mat4 inverseCameraMatrix{ glm::inverse(headset.cameraMatrix) };
	handModelLeft.WorldMatrix = inverseCameraMatrix * controllers.GetPose(0u);
	handModelRight.WorldMatrix = inverseCameraMatrix * controllers.GetPose(1u);
	handModelRight.WorldMatrix = glm::scale(handModelRight.WorldMatrix, { -1.0f, 1.0f, 1.0f });
}
