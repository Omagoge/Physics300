//
// Created by akaan on 19/11/2025.
//

#include "InterpMovementComponent.hpp"

#include <Toast/BadObjectException.hpp>
#include <Toast/Objects/Actor.hpp>
#include <Toast/Time.hpp>

#ifdef TOAST_EDITOR
#include "imgui.h"
#endif

void game::InterpMovementComponent::Begin() {
	Component::Begin();

	if (parent()->base_type() != toast::ActorT) {
		throw toast::BadObject(parent(), "InterpMovementComp can only be placed on an Actor");
	}

	auto* actor = static_cast<toast::Actor*>(parent());
	m_transform = actor->transform();

	if (!m_transform) {
		return;
	}

	m_origin = m_transform->worldPosition();
	m_t = 0.0f;
	m_direction = 1;
	m_waitTimer = 0.0f;
	m_isWaiting = false;
}

void game::InterpMovementComponent::Tick() {
	Component::Tick();

	if (!m_transform) {
		return;
	}
	if (m_offset == glm::vec3(0.0f)) {
		return;
	}

	float dt = static_cast<float>(Time::fixed_delta());

	if (m_isWaiting) {
		// we're at one end, count time
		m_waitTimer += dt;
		if (m_waitTimer >= m_waitTime) {
			m_waitTimer = 0.0f;
			m_isWaiting = false;
			m_direction *= -1;    // flip direction
		}
	} else {
		// move along the path
		m_t += m_direction * m_speed * dt;

		// hit the far end?
		if (m_t >= 1.0f) {
			m_t = 1.0f;
			if (m_waitTime > 0.0f) {
				m_isWaiting = true;
				m_waitTimer = 0.0f;
			} else {
				m_direction = -1;
			}
		}
		// hit the origin end?
		else if (m_t <= 0.0f) {
			m_t = 0.0f;
			if (m_waitTime > 0.0f) {
				m_isWaiting = true;
				m_waitTimer = 0.0f;
			} else {
				m_direction = 1;
			}
		}
	}

	glm::vec3 newPos = m_origin + m_offset * m_t;
	m_transform->worldPosition(newPos);
}

#ifdef TOAST_EDITOR
void game::InterpMovementComponent::Inspector() {
	Component::Inspector();

	ImGui::DragFloat3("Offset", &m_offset.x);
	ImGui::DragFloat("Speed", &m_speed);
	ImGui::DragFloat("Wait Time", &m_waitTime);
}
#endif

json_t game::InterpMovementComponent::Save() const {
	json_t j = Component::Save();

	j["offset_x"] = m_offset.x;
	j["offset_y"] = m_offset.y;
	j["speed"] = m_speed;
	j["wait_time"] = m_waitTime;

	return j;
}

void game::InterpMovementComponent::Load(json_t j, bool force_create) {
	Component::Load(j, force_create);

	if (j.contains("offset_x")) {
		m_offset.x = j.at("offset_x");
	}
	if (j.contains("offset_y")) {
		m_offset.y = j.at("offset_y");
	}
	if (j.contains("speed")) {
		m_speed = j.at("speed");
	}
	if (j.contains("wait_time")) {
		m_waitTime = j.at("wait_time");
	}
}
