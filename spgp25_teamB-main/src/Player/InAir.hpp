// TODO: JEADER

#pragma once
#include "Player.hpp"
#include "Toast/Time.hpp"

#include <Toast/StateMachine.hpp>

namespace player_states {

class InAir : public toast::State<Player> {
	void OnBegin() override {
		//@TODO:
		// SPINE ANIMATION START JUMPING/IN AIR

		auto* p_params = parent->GetParameters();
		parent->GetRigidbody()->drag = { .05f, .05f };
		p_params->groundTangent = { 1.0f, 0.0f };
		p_params->gravity = glm::vec2(0.f, -p_params->gravForce);

		parent->GetSpineRenderer()->PlayAnimation("an_cat_inair", true, 0);
	}

	void OnTick() override {
		auto* rb = this->parent->GetRigidbody();
		auto* params = this->parent->GetParameters();
		glm::vec2 moveInput = this->parent->GetMoveInput();

		// Apply air control
		if (glm::length(moveInput) > 0.01f) {
			glm::vec2 airForce = glm::vec2(moveInput.x, std::clamp(moveInput.y, 0.0f, 1.0f)) * params->airMoveSpeed * static_cast<float>(Time::delta());
			rb->AddForce(airForce);
		}

		glm::vec2 gravNorm = params->gravity;
		if (glm::length(gravNorm) > 1e-4f) {
			gravNorm = glm::normalize(gravNorm);
		} else {
			gravNorm = glm::vec2(0.f, -1.f);
		}
		float velAlongGrav = glm::dot(glm::vec2(rb->velocity), gravNorm);
		if (velAlongGrav > 0.0f) {    // positive dot with gravity = moving in gravity direction = falling
			this->parent->GetStateMachine()->SetState("falling");
		}

		// Check if player landed early
		if (params->grounded) {
			this->parent->GetStateMachine()->SetState("landing");
		}

		if (!parent->GetParameters()->isUsingGrav) {
			if (glm::length(parent->GetPreviousVelocity()) > 40.0f) {
				parent->GetOnAirParticles()->Play();

			} else {
				parent->GetOnAirParticles()->Pause();
			}
		}
	}

	void OnExit() override {
		parent->GetRigidbody()->drag = { .5f, .5f };

		parent->GetSpineRenderer()->CrossFadeToDefault(0.2f, 0);
		parent->GetOnAirParticles()->Pause();
	}
};

}
