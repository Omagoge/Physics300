// TODO: JEADER

#pragma once
#include "Player.hpp"
#include "Toast/Input/Haptics.hpp"
#include <Toast/StateMachine.hpp>

namespace player_states {

class Landing : public toast::State<Player> {
	float landTimer = 0.0f;

	void OnBegin() override {
		if (!parent) {
			return;
		}

		Player* p = parent;
		auto params = p->GetParameters();
		float prevVy = p->GetPreviousVelocity().y;

		if (params && prevVy <= params->stompVerticalVelocityThreshold) {
			auto* spine = p->GetSpineRenderer();
			if (spine) {
				spine->PlayAnimation("an_cat_land", false, 0);
			}
			// parent->GetSpineRenderer()->NextCrossFadeToDefault(0.05f, 0);
			landTimer = 0.0f;
			auto* rb = p->GetRigidbody();
			if (rb) {
				rb->drag = { 5, 5 };
			}
		} else {
			landTimer = params ? params->stompTime : 0.0f;
		}

		//TOAST_INFO("prevVy: {}", prevVy);

		if (prevVy <= -45.f) {
			p->DoDustEffect(2);
			haptics::Rumble(0.2f, 0.6f, 80);
		} else if (prevVy <= -30.f) {
			p->DoDustEffect(1);
			haptics::ImpactHeavy();
		} else if (prevVy <= -20.f) {
			p->DoDustEffect(0);
			haptics::ImpactLight();
		}

		p->Reload();

		parent->GetOnAirParticles()->Pause();
	}

	void OnTick() override {
		//@TODO

		glm::vec2 moveInput = this->parent->GetMoveInput();

		// tresold
		if (landTimer >= parent->GetParameters()->stompTime) {
			// Transition to idle or run depending on input
			if (glm::length(moveInput) > 0.01f) {
				this->parent->GetStateMachine()->SetState("run");
			} else {
				this->parent->GetStateMachine()->SetState("idle");
			}
		} else {
			landTimer += static_cast<float>(Time::delta());
		}
	}

	// dario is gay
	void OnExit() override {
		if (!parent) {
			return;
		}
		parent->GetRigidbody()->drag = { .5f, .5f };

		// Restore aiming animation on track 1 if the player is still aiming.
		if (parent->GetParameters()->isAiming && parent->GetCurrentWeaponData()) {
			parent->GetSpineRenderer()->PlayAnimation(parent->GetCurrentWeaponData()->aimingAnimation, true, 1);
		}
	}
};

}
