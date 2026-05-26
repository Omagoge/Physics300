#pragma once

#include "Pigeon.hpp"
#include "Toast/Time.hpp"
#include "glm/gtx/norm.hpp"

#include <Toast/StateMachine.hpp>

namespace moveEnemy_states {

class Returning : public toast::State<game::Pigeon> {
	void OnBegin() override {
		// Points and initial position
		const auto& points = parent->GetMoveParameters()->points;
		const glm::vec2 parent_pos = parent->transform()->position();
		m_closestPoint = points[0];

		// Initial diff and distance
		const glm::vec2 first_diff = m_closestPoint - parent_pos;
		float dist = glm::length2(first_diff);

		// Find nearest point to lerp to
		for (int i = 1; i < points.size(); i++) {
			// Get point and check dist
			glm::vec2 diff = points[i] - parent_pos;
			float point_dist = glm::length2(diff);

			// Get closest point and index based on dist
			if (point_dist < dist) {
				dist = point_dist;
				m_closestPoint = points[i];
				m_closestIndex = i;
			}
		}
	}

	void OnTick() override {
		// Parameters
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

		// Get diff between points
		glm::vec2 diff = m_closestPoint - glm::vec2(parent->transform()->position());
		const float diff_length = glm::length(diff);

		// If difference is within radius, advance point and set own position to the point + return to move state
		if (diff_length < params->pointRad) {
			// If closest point is not current point, do point advancing logic
			if (m_closestIndex != params->currentPoint) {
				// Set point to be the closest point we found
				params->currentPoint = m_closestIndex;
				params->nextPoint = params->currentPoint;

				// Set next point based on whether we are at end or not
				params->nextPoint = ++params->nextPoint >= params->points.size() ? 0 : params->nextPoint;

				// Set next point's direction anew
				params->nextPointDir = params->points[params->nextPoint] - params->points[params->currentPoint];

				// Normalize direction if not 0,0
				if (abs(params->nextPointDir.x) > E || abs(params->nextPointDir.y) > E) {
					params->nextPointDir = normalize(params->nextPointDir);
				}
			}

			// Update transform to be on the current point exactly to avoid floating errors
			parent->transform()->position(glm::vec3(params->points[params->currentPoint], parent->transform()->position().z));

			// Return to move idle
			params->onPath = true;
			parent->GetMoveStateMachine()->SetState("Idle");
			return;
		}

		// Normalize diff
		if (glm::length2(diff) > E) {
			diff = glm::normalize(diff);
		}

		// Temporary transform which serves as a lerp basically --> it lerps from current to next
		glm::vec3 temp_transform = { diff * params->speed, 0.f };

		// Here it lerps by adding temp transform and delta time
		parent->transform()->position(parent->transform()->position() + (temp_transform *= Time::delta()));
	}

private:
	glm::vec2 m_closestPoint;
	unsigned m_closestIndex = 0;
};
}
