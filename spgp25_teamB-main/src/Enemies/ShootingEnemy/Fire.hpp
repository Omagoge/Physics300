#pragma once

#include "ShootingEnemy.hpp"
#include "Toast/Physics/Raycast.hpp"
#include "Toast/Time.hpp"

#include <Toast/StateMachine.hpp>

namespace shootEnemy_states {

class Fire : public toast::State<game::ShootingEnemy> {
	void OnBegin() override {
		parent->GetShootParameters()->aimTimer = 0.f;
		parent->GetShootParameters()->shootTimer = 0.f;
		parent->GetShootParameters()->firing = true;
		parent->GetShootParameters()->warned = false;
	}

	void OnTick() override {
		// Variables for readability
		auto* params = parent->GetShootParameters();
		auto* sm = parent->GetShootStateMachine();

		// Sanity
		if (!params->shootPlayer && !params->shootChild) {
			sm->SetState("Idle");
			return;
		}

		// Increment timers
		params->aimTimer += Time::delta();
		params->shootTimer += Time::delta();

		// Change color based on time spent aiming
		if (params->aimTimer >= params->aimTime) {
			// Legit just shoot if beyond aim time
			parent->PlayFireAnim();
			parent->Shoot();
			// Otherwise, we warn player that we are about to shoot
		} else if (params->aimTimer >= (params->aimTime) / 4.f && !params->warned) {
			parent->Warn();
			params->warned = true;
		}
	}

	void OnExit() override {
		parent->GetShootParameters()->firing = false;
		parent->GetShootParameters()->aimTimer = 0.f;
		parent->GetShootParameters()->shootTimer = 0.f;
		parent->EndFireAnim();
	}
};
}
