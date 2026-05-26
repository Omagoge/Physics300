#pragma once

#include "ShootingEnemy.hpp"
#include "Toast/Time.hpp"

#include <Toast/StateMachine.hpp>

namespace shootEnemy_states {

class Idle : public toast::State<game::ShootingEnemy> {
	void OnBegin() override {
		parent->EndAimAnim();
		parent->PlayShootIdleAnim();
	}

	void OnTick() override {
		// Pointers for readability
		const auto* params = parent->GetShootParameters();
		auto* sm = parent->GetShootStateMachine();

		if (params->shootPlayer) {
			// If we are able to hit the player, we will switch to firing!
			auto ray_sult = physics::RayCast(parent->transform()->position(), params->currentDir, ColliderFlags::Player | ColliderFlags::Ground);
			if (ray_sult != std::nullopt) {
				if (Player* player = dynamic_cast<Player*>(ray_sult->other)) {
					// Switch to firing if ready!
					if (!params->coolingDown) {
						sm->SetState("Fire");
					}
				}
			}
		}

		// Otherwise, if shooting child, just send it dawg...
		else {
			if (!params->coolingDown) {
				sm->SetState("Fire");
			}
		}
	}

	void OnExit() override {
		parent->EndShootIdleAnim();
		parent->PlayAimAnim();    // Because aim is entered regularly from fire, I should change this -ax
	}
};
}
