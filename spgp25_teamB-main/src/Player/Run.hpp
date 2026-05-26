// TODO: JEADER

#pragma once
#include "Player.hpp"
#include "Toast/Time.hpp"

#include <Toast/Renderer/DebugDrawLayer.hpp>
#include <Toast/StateMachine.hpp>

namespace player_states {

struct Run : public toast::State<Player> {
	bool wasRunningBackwards = false;

	void OnBegin() override {
		wasRunningBackwards = parent->IsRunningBackwards();
		const char* anim = wasRunningBackwards ? "an_cat_backwards" : "an_cat_run";
		parent->GetSpineRenderer()->PlayAnimation(anim, true, 0);
		// parent->GetSpineRenderer()->NextCrossFadeToDefault(.05f, 0);
	}

	void OnTick() override {
		auto* rb = parent->GetRigidbody();
		auto* params = parent->GetParameters();
		glm::vec2 moveInput = parent->GetMoveInput();

		// Check if player is falling
		if (!params->grounded) {
			parent->GetStateMachine()->SetState("falling");
			return;
		}

		// Check for direction change using ground-tangent projection and input threshold
		glm::vec2 groundTangentNorm = params->groundTangent;
		if (glm::length(groundTangentNorm) < 1e-4f) {
			groundTangentNorm = glm::vec2(1.f, 0.f);
		} else {
			groundTangentNorm = glm::normalize(groundTangentNorm);
		}
		float projectedVelocity = glm::dot(glm::vec2(rb->velocity.x, rb->velocity.y), groundTangentNorm);
		float inputAlongTangent = parent->GetInputAlongTangent();

		// Use previous velocity to detect a direction change
		glm::vec2 prevVel = parent->GetPreviousVelocity();
		float projectedPrevVelocity = glm::dot(prevVel, groundTangentNorm);

		if (std::abs(projectedPrevVelocity) > params->directionChangeThreshold && (inputAlongTangent * projectedPrevVelocity) < 0.0f) {
			parent->GetStateMachine()->SetState("stopping");
			return;
		}

		// Swap animation in-place if backwards state changed
		bool nowBackwards = parent->IsRunningBackwards();
		if (nowBackwards != wasRunningBackwards) {
			wasRunningBackwards = nowBackwards;
			const char* anim = nowBackwards ? "an_cat_backwards" : "an_cat_run";
			parent->GetSpineRenderer()->PlayAnimation(anim, true, 0);
		}

		// Project input onto ground tangent and apply force
		glm::vec2 movementDirection = inputAlongTangent * groundTangentNorm;

		float delta = static_cast<float>(Time::delta());
		glm::vec2 force = movementDirection * params->moveSpeed * delta;
		// renderer::DrawDebugArrow(parent->transform()->worldPosition(), glm::vec3(force.x, force.y, 0) * 3.0f, 4.0f, { 1.f, 0.f, 0.f, 1.f });
		rb->AddForce(force);

		if (glm::length(moveInput) < 0.01f) {
			parent->GetStateMachine()->SetState("idle");
			return;
		}
	}

	void OnExit() override {
		parent->GetSpineRenderer()->NextCrossFadeToDefault(.5f, 0);
	}
};

}
