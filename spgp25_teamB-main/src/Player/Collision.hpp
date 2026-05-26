#pragma once

#include "Player.hpp"
#include "Toast/Time.hpp"

#include <Toast/StateMachine.hpp>

namespace player_states {

class Collision : public toast::State<Player> {
	void OnBegin() override {
		// Velocity at impact is now pointing away from the wall
		float velocityX = parent->GetRigidbody()->velocity.x;
		if (std::abs(velocityX) > 0.1f) {
			parent->SetSpriteFacingDirection(velocityX > 0.f ? -1 : 1);
		}

		// Play collision animation on track 0, clear aim track (1) and shoot track (2)
		parent->GetSpineRenderer()->CrossFadeToDefault(0.15f, 1);
		parent->GetSpineRenderer()->StopAnimation(2);
		// parent->GetSpineRenderer()->CrossFadeToDefault(0.15f, 0);
		parent->GetSpineRenderer()->PlayAnimation("an_cat_collision", false, 0);

		parent->SetColliding(true);
		*parent->GetCollisionTimer() = 0.0f;

		parent->GetRigidbody()->drag = { 20.f, 0.5f };

		parent->GetOnAirParticles()->Pause();

		// parent->Reload();
	}

	void OnTick() override {
		float deltaTime = static_cast<float>(Time::delta());
		auto* params = parent->GetParameters();
		// Increment timer via accessor
		*parent->GetCollisionTimer() += deltaTime;
		if (*parent->GetCollisionTimer() >= params->collisionStateDuration) {
			// exit collision state
			parent->SetColliding(false);
			// Transition to idle or run depending on input
			if (glm::length(parent->GetMoveInput()) > 0.01f) {
				parent->GetStateMachine()->SetState("run");
			} else {
				parent->GetStateMachine()->SetState("idle");
			}
		}
	}

	void OnExit() override {
		parent->SetColliding(false);
		parent->GetSpineRenderer()->CrossFadeToDefault(0.3f, 0);

		parent->GetRigidbody()->drag = { .5f, .5f };

		// Restore aiming animation on track 1 if the player is still aiming.
		if (parent->GetParameters()->isAiming && parent->GetCurrentWeaponData()) {
			parent->GetSpineRenderer()->PlayAnimation(parent->GetCurrentWeaponData()->aimingAnimation, true, 1);
		}
	}
};

}
