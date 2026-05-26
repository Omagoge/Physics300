/**
 * @file GameFlowDoor.hpp
 * @author Dante Harper
 * @date 23/03/26
 */

#pragma once

#include "Toast/Components/AtlasSpriteComponent.hpp"
#include "Toast/Objects/Actor.hpp"
#include "Toast/Physics/Trigger.hpp"
#include "Triggers/GameFlowTrigger.hpp"

#include <atomic>

namespace game {
class GameFlowDoor : public toast::Actor {
	struct {
		Actor* gameFlow;
		GameFlowTrigger* gLeft;
		GameFlowTrigger* gRight;
		physics::Trigger* tLeft;
		physics::Trigger* tMiddle;
		physics::Trigger* tRight;
		toast::AtlasSpriteComponent* sLeft;
		toast::AtlasSpriteComponent* sDoor;
		toast::AtlasSpriteComponent* sRight;

		std::atomic<bool> aLeft = false;
		std::atomic<bool> aRight = false;

		bool usePath = false;
		std::string path = "null";
	} m;

public:
	REGISTER_TYPE(GameFlowDoor);

	void Init() override;

	void Begin() override;

	[[nodiscard]]
	json_t Save() const override;
	void Load(json_t j, bool force_create = true) override;

#ifdef TOAST_EDITOR
	void Inspector() override;
#endif
};
}
