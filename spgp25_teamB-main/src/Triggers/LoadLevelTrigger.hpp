//
// Created by akaan on 28/01/2026.
//

#pragma once
#include "Toast/Physics/Trigger.hpp"

namespace toast {
class Object;
}

namespace game {
class [[deprecated("Use Gameflow Trigger Instead")]] LoadLevelTrigger : public physics::Trigger {
public:
	REGISTER_TYPE(LoadLevelTrigger);

	void OnEnter(toast::Object* obj) override;
	void OnExit(toast::Object*) override;
	void NextLevel() const;

#ifdef TOAST_EDITOR
	void Inspector() override;
#endif

	[[nodiscard]]
	json_t Save() const override;
	void Load(json_t j, bool force_create = true) override;

private:
	int m_triggerMode = 0;    // 0 = onEnter, 1 = onExit

	int m_worldIndex = -1;
	int m_levelIndex = -1;
};
}
