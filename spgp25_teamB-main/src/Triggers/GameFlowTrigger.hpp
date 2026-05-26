///@author Dante Harper

#pragma once
#include "Toast/Components/HtmlView.hpp"
#include "Toast/Event/ListenerComponent.hpp"
#include "Toast/Input/InputListener.hpp"
#include "Toast/Physics/Trigger.hpp"
#include "Toast/RTTIMacros.h"

namespace toast {
class Object;
}

enum GameFlowType : int {
	Null,
	NextLevel,
	NextWorld,
	Reset,
	LoadFromPath,
};

namespace game {
class GameFlowTrigger : public physics::Trigger {
	struct {
		GameFlowType type = NextLevel;
		GameFlowType pending = (GameFlowType)-1;
		bool enabled = true;
		input::Listener input;
		toast::HtmlView* view = nullptr;

		int movedir = 0;
		float moveheldtime = 0.0f;
		float nextrepeattime = 0.0f;

	} m;

	static constexpr float kinitialdelay = 0.35f;
	static constexpr float krepeatrate = 0.12f;
	static constexpr float kdeadzone = 0.4f;

	void OnMove(const input::Action2D* a, toast::HtmlView& v);
	void OnSelect(const input::Action0D* a, toast::HtmlView& v);

public:
	bool usePath = false;
	std::string path;

	REGISTER_TYPE(GameFlowTrigger);

	void Begin() override {
		physics::Trigger::Begin();
		m.enabled = true;
	}

	void OnEnter(toast::Object* obj) override;
	void Tick() override;

#ifdef TOAST_EDITOR
	void Inspector() override;
#endif

	[[nodiscard]]
	json_t Save() const override;
	void Load(json_t j, bool force_create = true) override;
};
}
