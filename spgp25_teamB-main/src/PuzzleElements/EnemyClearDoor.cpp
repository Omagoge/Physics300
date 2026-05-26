// created by Akaansh on 21/03/26

#include "EnemyClearDoor.hpp"

#ifdef TOAST_EDITOR
#include <imgui.h>
#include <imgui_stdlib.h>
#endif

#include "../Enemies/ShootingEnemy/ShootingEnemy.hpp"
#include "ShootableObject.hpp"

#include <Toast/Objects/Scene.hpp>
#include <Toast/Time.hpp>
#include <algorithm>

namespace game {

void EnemyClearDoor::Init() {
	Actor::Init();

	
}

void EnemyClearDoor::Begin() {
	Actor::Begin();
	
	hud = children.AddRequired<HUDActor>("hud");
	hud->SetUrl("file:///assets/UI/enemy_door.html");
	hud->SetDimestion(100, 100);

	auto scale = transform()->scale();
	hud->transform()->scale({ 1 / scale.x * 3, 1 / scale.y * 3, 1 / scale.z * 3 });

	m_rb = children.Get<physics::Rigidbody>();
	m_collider = children.Get<physics::Collider>();
	if (!m_rb) {
		TOAST_WARN("EnemyClearDoor '{}': no rigidbody found in children!", name());
	}

	m_unlocked = false;
	m_movedSoFar = 0.f;
	m_enemies.clear();
	m_collider->enabled(true);

	if (m_doorTag.empty()) {
		TOAST_WARN("EnemyClearDoor '{}': no door tag set", name());
		return;
	}

	// scan scene for ShootingEnemies or Shootable Objects whose doorTag matches ours
	std::function<void(toast::Object*)> scan = [&](toast::Object* obj) {
		if (!obj) {
			return;
		}

		if (m_trueEnemyFalseObject) {
			if (auto* enemy = dynamic_cast<ShootingEnemy*>(obj)) {
				if (enemy->GetDoorTag() == m_doorTag) {
					m_enemies.push_back(enemy);
				}
			}
		} else {
			if (auto* object = dynamic_cast<ShootableObject*>(obj)) {
				if (object->GetDoorTag() == m_doorTag) {
					m_enemies.push_back(object);
				}
			}
		}

		for (auto& [_, child] : obj->children.GetAll()) {
			scan(child.get());
		}
	};

	scan(scene());

	TOAST_INFO("EnemyClearDoor '{}': linked {} enemies with tag '{}'", name(), m_enemies.size(), m_doorTag);
}

int EnemyClearDoor::GetAliveEnemiesCount() const {
	int alive_count = 0;
	for (IDamageable* e : m_enemies) {
		if (e && e->mHealth > 0.f) {
			alive_count++;
		}
	}
	return alive_count;
}

void EnemyClearDoor::Tick() {
	Actor::Tick();

	int total_enemies = static_cast<int>(m_enemies.size());
	int alive_enemies = GetAliveEnemiesCount();

	// 2. Dynamically update the HTML HUD layout (n = alive, d = total)
	if (!m_unlocked && hud && total_enemies > 0) {
		auto command = std::format("SetRatio({},{})", alive_enemies, total_enemies);
		hud->ExecuteJS(command);
	}

	if (!m_unlocked && AllEnemiesDead()) {
		Unlock();
	}

	// Move the door directly through the transform (bypass physics velocity)
	if (m_unlocked && m_movedSoFar < m_openDistance) {
		float dt = static_cast<float>(Time::delta());
		float step = m_openSpeed * dt;
		float remaining = m_openDistance - m_movedSoFar;
		float actual = std::min(step, remaining);

		// Update actor transform directly
		auto pos3 = transform()->worldPosition();
		if (m_trueYFalseX) {
			pos3.y += actual * static_cast<float>(m_yDir);
		} else {
			pos3.x += actual * static_cast<float>(m_xDir);
		}
		transform()->worldPosition(pos3);

		// Keep the physics rigidbody in sync with the transform but don't use it for movement
		m_movedSoFar += actual;
		if (m_rb) {
			m_rb->SetPosition(glm::dvec2(pos3.x, pos3.y));
			m_rb->SetVelocity(glm::dvec2(0.0, 0.0));
		}
	} else {
		// Ensure rigidbody velocity is zero when door is stopped
		if (m_rb) {
			m_rb->SetVelocity(glm::dvec2(0.0, 0.0));
		}
	}
}

void EnemyClearDoor::Destroy() {
	Actor::Destroy();
	m_enemies.clear();
	m_rb = nullptr;
}

bool EnemyClearDoor::AllEnemiesDead() const {
	if (m_enemies.empty()) {
		return false;    // add some enenmies brev, no enemies = stay locked
	}

	for (IDamageable* e : m_enemies) {
		if (e && e->mHealth > 0.f) {
			return false;
		}
	}
	return true;
}

void EnemyClearDoor::Unlock() {
	m_unlocked = true;
	if (m_collider) {
		m_collider->enabled(false);
	}
}

json_t EnemyClearDoor::Save() const {
	json_t j = Actor::Save();
	j["doorTag"] = m_doorTag;
	j["openDistance"] = m_openDistance;
	j["openSpeed"] = m_openSpeed;
	j["usingEnemies"] = m_trueEnemyFalseObject;
	j["verticalMovement"] = m_trueYFalseX;
	j["yDir"] = m_yDir;
	j["xDir"] = m_xDir;
	return j;
}

void EnemyClearDoor::Load(json_t j, bool force_create) {
	Actor::Load(j, force_create);
	if (j.contains("doorTag")) {
		m_doorTag = j["doorTag"].get<std::string>();
	}
	if (j.contains("openDistance")) {
		m_openDistance = j["openDistance"].get<float>();
	}
	if (j.contains("openSpeed")) {
		m_openSpeed = j["openSpeed"].get<float>();
	}
	if (j.contains("usingEnemies")) {
		m_trueEnemyFalseObject = j["usingEnemies"].get<bool>();
	}
	if (j.contains("verticalMovement")) {
		m_trueYFalseX = j["verticalMovement"].get<bool>();
	}
	if (j.contains("yDir")) {
		m_yDir = j["yDir"].get<int>();
	}
	if (j.contains("xDir")) {
		m_xDir = j["xDir"].get<int>();
	}
}

#ifdef TOAST_EDITOR
void EnemyClearDoor::Inspector() {
	Actor::Inspector();

	ImGui::SeparatorText("EnemyClearDoor");

	ImGui::InputText("Door Tag", &m_doorTag);
	ImGui::DragFloat("Open Distance", &m_openDistance, 0.1f, 0.f, 50.f);
	ImGui::DragFloat("Open Speed", &m_openSpeed, 0.1f, 0.f, 20.f);

	ImGui::Spacing();

	if (ImGui::CollapsingHeader("Experimental")) {
		ImGui::Spacing();
		ImGui::DragInt("Y Direction", &m_yDir, 1, -1, 1);
		ImGui::DragInt("X Direction", &m_xDir, 1, -1, 1);
		ImGui::Spacing();
		ImGui::Checkbox("Vertical Movement", &m_trueYFalseX);
		ImGui::Spacing();
	}

	ImGui::Spacing();

	ImGui::Checkbox("Using Enemies", &m_trueEnemyFalseObject);

	ImGui::Spacing();

	if (m_trueEnemyFalseObject) {
		ImGui::TextColored({ 1.f, 1.f, 1.f, 1.f }, "USING ENEMIES");
	} else {
		ImGui::TextColored({ 1.f, 1.f, 1.f, 1.f }, "USING SHOOTABLE OBJECTS");
	}

	ImGui::Spacing();

	if (m_unlocked) {
		ImGui::TextColored({ 0.2f, 1.f, 0.4f, 1.f }, "UNLOCKED");
	} else {
		ImGui::TextColored({ 1.f, 0.3f, 0.3f, 1.f }, "LOCKED");
	}

	if (!m_enemies.empty()) {
		ImGui::Spacing();
		ImGui::Text("Linked enemies (%zu):", m_enemies.size());
		for (IDamageable* e : m_enemies) {
			auto* asActor = dynamic_cast<toast::Actor*>(e);
			const std::string name = asActor ? asActor->name() : "???";
			bool dead = e->mHealth <= 0.f;
			if (dead) {
				ImGui::TextDisabled("  [dead]  %s", name.c_str());
			} else {
				ImGui::Text("  [alive] %s", name.c_str());
			}
		}
	} else {
		ImGui::Spacing();
		ImGui::TextDisabled("No enemies linked - (make sure da tag matches dumdum)");
	}
}
#endif

}    // namespace game
