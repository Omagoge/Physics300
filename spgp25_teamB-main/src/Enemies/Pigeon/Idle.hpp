#pragma once

#include "Pigeon.hpp"
#include "Toast/Time.hpp"
#include "glm/gtx/norm.hpp"

#include <Toast/StateMachine.hpp>

namespace moveEnemy_states {

class Idle : public toast::State<game::Pigeon> {
	void OnBegin() override {
		parent->GetSpineRenderer()->PlayAnimation("an_pidgeon_idle", true, 0);
	}

	void OnTick() override {
		// Variables for readability
		auto* params = parent->GetMoveParameters();
		auto* shoot_params = parent->GetShootParameters();
		constexpr float E = std::numeric_limits<float>::epsilon();

		// Sanity
		if (params->points.size() < 2) {
			return;
		}

		// If visible and shooting is enabled, hover and shoot!
		if (shoot_params->firing) {
			return;
		}

		// Get difference between next point and current position
		glm::vec2 diff = params->points[params->nextPoint] - glm::vec2(parent->transform()->position());
		glm::vec2 normalized_diff = diff;

		// Get Normalized direction if not 0,0
		if (glm::length2(normalized_diff) > E) {
			normalized_diff = normalize(normalized_diff);
		}

		// Get angle between next point and current direction to see if we are off track
		const glm::vec2 cross = glm::cross(glm::vec3(normalized_diff, 0.f), glm::vec3(params->nextPointDir, 0.f));
		const float cross_mag = std::sqrt(glm::length(cross));
		const float dot = glm::dot(normalized_diff, params->nextPointDir);
		const float angle = std::atan2(cross_mag, dot) * 180.f / glm::pi<float>();

		// Check for if on path
		params->onPath = abs(angle) <= E;
		if (!params->onPath) {
			parent->GetMoveStateMachine()->SetState("Returning");
		}

		// If difference is within radius, advance point and set own position to the point
		if ((diff.x * diff.x) + (diff.y * diff.y) < params->pointRad * params->pointRad) {
			// Advance Point
			params->currentPoint = params->nextPoint;

			// Set next point based on whether we are at end or not
			params->nextPoint = ++params->nextPoint >= params->points.size() ? 0 : params->nextPoint;

			// Update transform to be on the current point exactly to avoid floating errors
			parent->transform()->position(glm::vec3(params->points[params->currentPoint], parent->transform()->position().z));

			// Set next point's direction anew
			params->nextPointDir = params->points[params->nextPoint] - params->points[params->currentPoint];

			// Normalize direction if not 0,0
			if (glm::length2(params->nextPointDir) > E) {
				params->nextPointDir = normalize(params->nextPointDir);
			}
		}

		// Temporary transform which serves as a lerp basically --> it lerps from current to next
		glm::vec3 temp_transform = { params->nextPointDir.x * params->speed, params->nextPointDir.y * params->speed, 0.f };

		// Here it lerps by adding temp transform and delta time
		parent->transform()->position(parent->transform()->position() + (temp_transform *= Time::delta()));
	}
};
}
