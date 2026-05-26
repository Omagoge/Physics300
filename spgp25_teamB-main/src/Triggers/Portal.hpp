#include "Toast/Input/InputListener.hpp"
#include "Toast/Physics/Trigger.hpp"
#include "Toast/Renderer/HUD/HUDActor.hpp"

namespace game {

class Portal : public physics::Trigger {
	struct {
		// std::string name = "level";
		int world;
		int level;
		std::string path;
		std::string prevLevel;
		bool active = false;    // TODO: use evil Dante gameflow to set this based on stats
		bool alwaysActive = false;
		bool usePathInstead = false;
		bool playerIsColliding = false;
		bool locked = true;
		input::Listener input;
		HUDActor* hud = nullptr;
	} m;

public:
	REGISTER_TYPE(Portal);

	void Init() override;
	void Begin() override;

	void OnEnter(toast::Object* obj) override;

	void OnExit(toast::Object*) override;

#ifdef TOAST_EDITOR
	void Inspector() override;
#endif

	[[nodiscard]]
	json_t Save() const override;
	void Load(json_t j, bool force_create = true) override;

	bool WorldLevelMatch(const int world, const int level) const {
		return world == m.world && level == m.level && m.active;
	}
};
}
