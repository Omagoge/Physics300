// TODO: JEADER

#pragma once
#include "Player.hpp"

#include <Toast/StateMachine.hpp>

namespace player_states {

class Idle : public toast::State<Player> {
	float idleBreak_timer = 0.f;

	void OnBegin() override {
		//@TODO:
		// SPINE ANIMATION START IDLE

		parent->GetSpineRenderer()->PlayAnimation("an_cat_idle", true, 0);
		// parent->GetSpineRenderer()->NextCrossFadeToDefault(.05f, 0);
		idleBreak_timer = 0.f;

		parent->GetOnAirParticles()->Pause();
	}

	void OnTick() override {
		// Check if player is falling
		if (!this->parent->GetParameters()->grounded) {
			this->parent->GetStateMachine()->SetState("falling");
			return;
		}

		// do player stopping anim if we were moving fast and input indicates a turn
		glm::vec2 groundTangentNorm = parent->GetParameters()->groundTangent;
		if (glm::length(groundTangentNorm) < 1e-4f) {
			groundTangentNorm = glm::vec2(1.f, 0.f);
		} else {
			groundTangentNorm = glm::normalize(groundTangentNorm);
		}
		float projectedPrevVelocity = glm::dot(parent->GetPreviousVelocity(), groundTangentNorm);
		float inputAlongTangent = parent->GetInputAlongTangent();
		if (std::abs(projectedPrevVelocity) > parent->GetParameters()->directionChangeThresholdIdle && inputAlongTangent * projectedPrevVelocity < 0.f) {
			this->parent->GetStateMachine()->SetState("stopping");
			return;
		}

		if (idleBreak_timer > 15.f) {
			if (!parent->GetParameters()->isAiming) {
				parent->GetSpineRenderer()->StopAnimation(0);
				parent->GetSpineRenderer()->PlayAnimation("an_cat_idlebreak", false, 0);
				parent->GetSpineRenderer()->NextPlayAnimation("an_cat_idle", true, .75f);
			}
			idleBreak_timer = 0.f;
		} else {
			idleBreak_timer += Time::delta();
		}
	}

	void OnExit() override { }
};

}
