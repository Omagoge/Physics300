//
// Created by inaki on 26/01/2026.
//
#pragma once
#include "Toast/Physics/Trigger.hpp"

namespace toast {
class Object;
}

namespace game {
class DeadTrigger : public physics::Trigger {
public:
	REGISTER_TYPE(DeadTrigger);

	void OnEnter(toast::Object* obj) override;

	void OnExit(toast::Object*) override { }
};

}
