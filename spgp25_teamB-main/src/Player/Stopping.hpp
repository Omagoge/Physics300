// Stopping state
// @author Dario
// @date 18 Feb 2026

#pragma once
#include "Player.hpp"
#include "Toast/Time.hpp"

#include <Toast/StateMachine.hpp>

namespace player_states {

class Stopping : public toast::State<Player> {
	void OnBegin() override {
		// Flip sprite to face the stopping force direction
		float velocityX = parent->GetRigidbody()->velocity.x;
		if (std::abs(velocityX) > 0.1f) {
			parent->SetSpriteFacingDirection(velocityX > 0.f ? 1 : -1);
		}

		// Play stop animation on track 0 (movement). Clear aim track (1) so aiming visuals don't interfere.
		parent->GetSpineRenderer()->CrossFadeToDefault(0.15f, 0);
		if (parent->GetParameters()->currentWeaponIndex == 0) {
			parent->GetSpineRenderer()->CrossFadeToDefault(0.15f, 1);
		}
		parent->GetSpineRenderer()->PlayAnimation("an_cat_stop", false, 0);
		parent->GetSpineRenderer()->NextCrossFadeToDefault(0.3f, 0);

		// Reset stop timer
		*parent->GetStopTimer() = 0.0f;
		parent->SetStopping(true);

		parent->GetRigidbody()->drag = { parent->GetParameters()->stoppingDragGrounded, parent->GetParameters()->stoppingDragGrounded };

		parent->DoDustParticles(true);

		parent->GetOnAirParticles()->Pause();
	}

	void OnTick() override {
		// If player is not grounded, transition to in_air immediately
		if (!parent->GetParameters()->grounded) {
			parent->GetStateMachine()->SetState("in_air");
			return;
		}

		if (glm::length(parent->GetRigidbody()->velocity) <= 1.0f) {
			// If velocity is already very low, skip stop animation and transition to idle
			parent->SetStopping(false);
			glm::vec2 moveInput = parent->GetMoveInput();
			if (glm::length(moveInput) > 0.01f) {
				parent->GetStateMachine()->SetState("run");
			} else {
				parent->GetStateMachine()->SetState("idle");
			}
			return;
		}

		float deltaTime = static_cast<float>(Time::delta());
		float* stopTimer = parent->GetStopTimer();
		auto* params = parent->GetParameters();

		*stopTimer += deltaTime;

		// transition back to run or idle
		if (*stopTimer >= params->stopAnimationDuration) {
			parent->SetStopping(false);

			glm::vec2 moveInput = parent->GetMoveInput();
			if (glm::length(moveInput) > 0.01f) {
				parent->GetStateMachine()->SetState("run");
			} else {
				parent->GetStateMachine()->SetState("idle");
			}
		}
	}

	void OnExit() override {
		parent->SetStopping(false);
		parent->GetSpineRenderer()->CrossFadeToDefault(0.1f, 0);

		// Reset drag
		parent->GetRigidbody()->drag = { 0.5f, 0.5f };

		// Restore aiming animation on track 1 if the player is still aiming.
		if (parent->GetParameters()->isAiming && parent->GetCurrentWeaponData()) {
			parent->GetSpineRenderer()->PlayAnimation(parent->GetCurrentWeaponData()->aimingAnimation, true, 1);
		}

		parent->DoDustParticles(false);
	}
};

}
