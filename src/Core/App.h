
#include "../VR/Headset.h"
#include "../VulkanBase/VulkanWindow.h"

class Controllers;
struct GameObject;

constexpr float flySpeedMultiplier = 2.5f;
class App		final
{
public:
	static constexpr int m_WIDTH{ 800 };
	static constexpr int m_HEIGHT{ 600 };

	App(){};
	~App();
	App(const App&) = delete;
	App(App&&) = delete;
	App& operator=(const App&) = delete;
	App& operator=(App&&) = delete;

	int Run();

private:
	void UpdateControllers(Headset& headset, Controllers& controllers, GameObject& handModelRight, GameObject& handModelLeft);
	void UpdateObjects(float time, GameObject& bikeModel);
	int	 PresentImage(VulkanWindow& window, const uint32_t& swapchainImageIndex, VulkanRenderer& renderer);
	void UpdateGameObjects(std::vector<GameObject*>& gameObjects, Headset& headset);
};
