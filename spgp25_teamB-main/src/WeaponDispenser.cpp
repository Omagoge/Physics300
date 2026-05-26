// Created by Akaansh on 23/03/2026.

#include "WeaponDispenser.hpp"

#include "Weapons/WeaponActorModified.hpp"

#ifdef TOAST_EDITOR
#include <imgui.h>
#endif

#include <Toast/Objects/Scene.hpp>
#include <Toast/Time.hpp>

void WeaponDispenser::Init() {
	Actor::Init();
}

void WeaponDispenser::Begin() {
	Actor::Begin();

	m_spawnedWeapon = nullptr;
	m_waitingToSpawn = false;
	m_respawnTimer = 0.f;
	m_needsCleanup   = true;

	// Check if weapon modifiesd
	listener()->Subscribe<WeaponActorModified>([this](WeaponActorModified* e) -> bool {
		if (!e || e->created) {
			return false;
		}

		if (e->weapon == m_spawnedWeapon) {
			TOAST_INFO("WeaponDispenser '{}': weapon gone, respawning in {}s", name(), m_respawnDelay);
			m_spawnedWeapon = nullptr;
			m_waitingToSpawn = true;
			m_respawnTimer = m_respawnDelay;
		}

		return false;
	});

}

void WeaponDispenser::Tick() {
	Actor::Tick();

	if (m_needsCleanup) {
		m_needsCleanup = false;

		std::vector<unsigned int> toRemove;

		std::function<void(toast::Object*)> scan = [&](toast::Object* obj) {
			if (!obj) return;
			if (obj->name().find("_spawned_weapon") != std::string::npos) {
				toRemove.push_back(obj->id());
			}
			for (auto& [_, child] : obj->children.GetAll()) {
				scan(child.get());
			}
		};

		scan(scene());

		for (unsigned int id : toRemove) {
			scene()->children.Remove(id);
		}

		SpawnWeapon();
	}

	if (!m_waitingToSpawn) {
		return;
	}

	m_respawnTimer -= static_cast<float>(Time::delta());
	if (m_respawnTimer <= 0.f) {
		m_waitingToSpawn = false;
		SpawnWeapon();
	}
}

void WeaponDispenser::Destroy() {
	// TODO: this doesnt destry the spawned weapon so it stacks on stop/play need to fix
	if (m_spawnedWeapon) {
		if (m_spawnedWeapon->enabled()) {
			m_spawnedWeapon->Destroy();
		}
		m_spawnedWeapon = nullptr;
	}

	Actor::Destroy();
}

void WeaponDispenser::SpawnWeapon() {
	auto* weapon = scene()->children.Add<WeaponActor>();
	if (!weapon) {
		TOAST_WARN("WeaponDispenser '{}': failed to spawn", name());
		return;
	}
	weapon->SetSerialize(false);
	weapon->name(name() + "_spawned_weapon");
	weapon->ChangeWeaponType(m_weaponType);
	weapon->enabled(true);
	weapon->ThrowWeapon(transform()->worldPosition(), { 0, 0 });

	// Runtime calls Object::_Begin(), which already loads textures in standalone.
	// Keep explicit load only for editor paths where auto-load differs.
#ifdef TOAST_EDITOR
	if (weapon->MeshComponent) {
		weapon->MeshComponent->LoadTextures();
	}
#endif

	m_spawnedWeapon = weapon;
	TOAST_INFO("WeaponDispenser '{}': spawned weapon type {}", name(), m_weaponType);
}

json_t WeaponDispenser::Save() const {
	json_t j = Actor::Save();
	j["weaponType"] = m_weaponType;
	j["respawnDelay"] = m_respawnDelay;
	return j;
}

void WeaponDispenser::Load(json_t j, bool force_create) {
	Actor::Load(j, force_create);
	if (j.contains("weaponType")) {
		m_weaponType = j["weaponType"].get<int>();
	}
	if (j.contains("respawnDelay")) {
		m_respawnDelay = j["respawnDelay"].get<float>();
	}
}

#ifdef TOAST_EDITOR
void WeaponDispenser::Inspector() {
	Actor::Inspector();

	ImGui::SeparatorText("WeaponDispenser");

	const char* weaponTypes[] = { "Shotgun", "SMG" };

	int comboIndex = 0;
	if (m_weaponType == 1) {
		comboIndex = 0;
	} else if (m_weaponType == 2) {
		comboIndex = 1;
	}

	if (ImGui::Combo("Weapon Type", &comboIndex, weaponTypes, IM_ARRAYSIZE(weaponTypes))) {
		m_weaponType = comboIndex + 1;
	}

	ImGui::DragFloat("Respawn Delay", &m_respawnDelay, 0.1f, 0.f, 30.f);

	ImGui::Spacing();

	if (m_spawnedWeapon) {
		ImGui::TextColored({ 0.2f, 1.f, 0.4f, 1.f }, "Weapon active");
	} else if (m_waitingToSpawn) {
		ImGui::TextColored({ 1.f, 0.8f, 0.f, 1.f }, "Respawning in %.1fs", m_respawnTimer);
	} else {
		ImGui::TextDisabled("No weapon spawned");
	}
}
#endif
