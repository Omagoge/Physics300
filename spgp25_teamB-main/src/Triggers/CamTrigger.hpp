//
// Created by akaansh on 02/02/2026.
//
#pragma once

#include "Toast/Physics/Trigger.hpp"

namespace toast {
class Object;
}

namespace game {

class CamTrigger : public physics::Trigger {
public:
	REGISTER_TYPE(CamTrigger);

	void OnEnter(toast::Object* obj) override;

	void OnExit(toast::Object*) override { }

#ifdef TOAST_EDITOR
	void Inspector() override;
#endif

	[[nodiscard]]
	json_t Save() const override;
	void Load(json_t j, bool force_create = true) override;

private:
	float m_camZoom = 1.0f;
};

}
