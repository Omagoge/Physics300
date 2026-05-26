// created by Alexey on 31/03/2026

#include "HubDoor.hpp"

#ifdef TOAST_EDITOR
#include <imgui.h>
#include <imgui_stdlib.h>
#endif

#include "Triggers/Portal.hpp"

#include <Toast/Objects/Scene.hpp>
#include <Toast/Time.hpp>
#include <algorithm>

namespace game {

void HubDoor::Init() {
	Actor::Init();
}

void HubDoor::Begin() {
	Actor::Begin();

	m_rb = children.Get<physics::Rigidbody>();
	m_collider = children.Get<physics::Collider>();
	m_collider->enabled(true);

	if (!m_rb) {
		TOAST_WARN("HubDoor '{}': no rigidbody found in children!", name());
	}

	m.unlocked = false;
	m.movedSoFar = 0.f;

	// scan scene for Portals with matching world and level
	std::function<void(toast::Object*)> scan = [&](toast::Object* obj) {
		if (!obj) {
			return;
		}

		if (auto* object = dynamic_cast<Portal*>(obj)) {
			if (object->WorldLevelMatch(m.world, m.level)) {
				Unlock();
			}
		}

		for (auto& [_, child] : obj->children.GetAll()) {
			scan(child.get());
		}
	};

	scan(scene());
}

void HubDoor::Tick() {
	Actor::Tick();

	// Move the door directly through the transform (bypass physics velocity)
	if (m.unlocked && m.movedSoFar < m.openDistance) {
		float dt = static_cast<float>(Time::delta());
		float step = m.openSpeed * dt;
		float remaining = m.openDistance - m.movedSoFar;
		float actual = std::min(step, remaining);

		// Update actor transform directly
		auto pos3 = transform()->worldPosition();
		if (m.trueYFalseX) {
			pos3.y += actual * static_cast<float>(m.yDir);
		} else {
			pos3.x += actual * static_cast<float>(m.xDir);
		}
		transform()->worldPosition(pos3);

		// Keep the physics rigidbody in sync with the transform but don't use it for movement
		m.movedSoFar += actual;
		if (m_rb) {
			m_rb->SetPosition(glm::dvec2(pos3.x, pos3.y));
			m_rb->SetVelocity(glm::dvec2(0.0, 0.0));
		} else {
			// Ensure rigidbody velocity is zero when door is stopped
			if (m_rb) {
				m_rb->SetVelocity(glm::dvec2(0.0, 0.0));
			}
		}
	}
}

void HubDoor::Destroy() {
	Actor::Destroy();
	m_rb = nullptr;
}

void HubDoor::CheckUnlock(int world, int level) {
	if (m.world == world && m.level == level) {
		Unlock();
	}
}

void HubDoor::Unlock() {
	m.unlocked = true;
	if (m_collider) {
		m_collider->enabled(false);
	}
}

json_t HubDoor::Save() const {
	json_t j = Actor::Save();
	j["world"] = m.world;
	j["level"] = m.level;
	j["openDistance"] = m.openDistance;
	j["openSpeed"] = m.openSpeed;
	j["verticalMovement"] = m.trueYFalseX;
	j["yDir"] = m.yDir;
	j["xDir"] = m.xDir;
	return j;
}

void HubDoor::Load(json_t j, bool force_create) {
	Actor::Load(j, force_create);
	if (j.contains("world")) {
		m.world = j["world"].get<int>();
	}
	if (j.contains("level")) {
		m.level = j["level"].get<int>();
	}
	if (j.contains("openDistance")) {
		m.openDistance = j["openDistance"].get<float>();
	}
	if (j.contains("openSpeed")) {
		m.openSpeed = j["openSpeed"].get<float>();
	}
	if (j.contains("verticalMovement")) {
		m.trueYFalseX = j["verticalMovement"].get<bool>();
	}
	if (j.contains("yDir")) {
		m.yDir = j["yDir"].get<int>();
	}
	if (j.contains("xDir")) {
		m.xDir = j["xDir"].get<int>();
	}
}

#ifdef TOAST_EDITOR
void HubDoor::Inspector() {
	Actor::Inspector();

	if (ImGui::CollapsingHeader("Hub Door Parameters")) {
		ImGui::DragFloat("Open Distance", &m.openDistance, 0.1f, 0.f, 50.f);
		ImGui::DragFloat("Open Speed", &m.openSpeed, 0.1f, 0.f, 20.f);
		ImGui::InputInt("World", &m.world);
		ImGui::InputInt("Level", &m.level);

		ImGui::Spacing();

		if (ImGui::Button("Unlock Door")) {
			CheckUnlock(m.world, m.level);
		}

		ImGui::Spacing();

		if (ImGui::Button("Test Portal Unlock")) {
			// scan scene for Portals with matching world and level
			std::function<void(toast::Object*)> scan = [&](toast::Object* obj) {
				if (!obj) {
					return;
				}

				if (auto* object = dynamic_cast<Portal*>(obj)) {
					if (object->WorldLevelMatch(m.world, m.level)) {
						Unlock();
					}
				}

				for (auto& [_, child] : obj->children.GetAll()) {
					scan(child.get());
				}
			};

			scan(scene());
		}
	}

	ImGui::Spacing();

	if (ImGui::CollapsingHeader("Directional Modifiers")) {
		ImGui::Spacing();
		ImGui::DragInt("Y Direction", &m.yDir, 1, -1, 1);
		ImGui::DragInt("X Direction", &m.xDir, 1, -1, 1);
		ImGui::Spacing();
		ImGui::Checkbox("Vertical Movement", &m.trueYFalseX);
		ImGui::Spacing();
	}

	ImGui::Spacing();

	if (m.unlocked) {
		ImGui::TextColored({ 0.2f, 1.f, 0.4f, 1.f }, "UNLOCKED");
	} else {
		ImGui::TextColored({ 1.f, 0.3f, 0.3f, 1.f }, "LOCKED");
	}
}
#endif

}
