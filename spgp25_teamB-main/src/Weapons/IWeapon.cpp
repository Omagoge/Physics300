#include "IWeapon.hpp"

#include "Toast/Renderer/HUD/HUDLayer.hpp"
#include "WallBulletHitEffect.hpp"
#include "WeaponEvents.hpp"

#include <Toast/GlmJson.hpp>
#include <Toast/Log.hpp>
#include <Toast/Resources/ResourceManager.hpp>
#ifdef TOAST_EDITOR
#include <imgui.h>
#include <imgui_stdlib.h>
#endif
#include <iostream>

#define JSON_LOAD(name)                          \
	do {                                           \
		if (j.contains(#name))                       \
			(name) = j[#name];                         \
		else                                         \
			TOAST_WARN("Cannot find property " #name); \
	} while (0)

#define JSON_SAVE(name) \
	do {                  \
		j[#name] = name;    \
	} while (0)

void IWeapon::SaveToFile() {
	json_t j;
	Save(j);

	resource::SaveFile(path(), j.dump(2));
}

IWeapon::IWeapon() = default;

void IWeapon::LoadFromFile() {
	std::istringstream stream;
	json_t j;
	resource::ResourceManager::GetInstance()->OpenFile(path(), stream);
	stream >> j;
	Load(j);
}

void IWeapon::Save(json_t& j) {
	JSON_SAVE(name);
	JSON_SAVE(damage);
	JSON_SAVE(cooldown);
	JSON_SAVE(isAutomatic);
	JSON_SAVE(maxForce);
	JSON_SAVE(magazineSize);
	JSON_SAVE(initialAmmo);
	JSON_SAVE(droppedTexture);
	JSON_SAVE(uiIconPath);
	JSON_SAVE(aimingAnimation);
	JSON_SAVE(shootingAnimation);

	JSON_SAVE(recoil.forceFalloffVec);
	JSON_SAVE(recoil.velocityReduction);
	JSON_SAVE(recoil.forceReduction);
	JSON_SAVE(recoil.maxSpeedFactor);
	JSON_SAVE(recoil.oppositeBoostScale);
	JSON_SAVE(recoil.groundDamp);
	JSON_SAVE(recoil.yBoostScale);
	JSON_SAVE(recoil.yBoostCap);
	JSON_SAVE(recoil.xImpulseCap);
	JSON_SAVE(recoil.xImpulseScale);

	JSON_SAVE(effectSocket);
	JSON_SAVE(maxDispersionAngle);
	JSON_SAVE(maxBulletsPerShot);

	JSON_SAVE(muzzleAnimName);

	int collision_flag = int(collisionFlag);
	JSON_SAVE(collision_flag);
}

void IWeapon::Load(json_t& j) {
	JSON_LOAD(name);
	JSON_LOAD(damage);
	JSON_LOAD(cooldown);
	JSON_LOAD(isAutomatic);
	JSON_LOAD(maxForce);
	JSON_LOAD(magazineSize);
	JSON_LOAD(initialAmmo);
	JSON_LOAD(droppedTexture);
	JSON_LOAD(uiIconPath);
	JSON_LOAD(aimingAnimation);
	JSON_LOAD(shootingAnimation);

	JSON_LOAD(recoil.forceFalloffVec);
	JSON_LOAD(recoil.velocityReduction);
	JSON_LOAD(recoil.forceReduction);
	JSON_LOAD(recoil.maxSpeedFactor);
	JSON_LOAD(recoil.oppositeBoostScale);
	JSON_LOAD(recoil.groundDamp);
	JSON_LOAD(recoil.yBoostScale);
	JSON_LOAD(recoil.yBoostCap);
	JSON_LOAD(recoil.xImpulseCap);
	JSON_LOAD(recoil.xImpulseScale);

	JSON_LOAD(effectSocket);

	JSON_LOAD(muzzleAnimName);

	JSON_LOAD(maxDispersionAngle);
	JSON_LOAD(maxBulletsPerShot);

	int collision_flag = 0;
	JSON_LOAD(collision_flag);
	collisionFlag = ColliderFlags(collision_flag);

	availableAmmo = initialAmmo;
	Reload();
}

#ifdef TOAST_EDITOR
void IWeapon::Inspector() {
	ImGui::PushID(path());
	ImGui::Indent(20);

	ImGui::InputText("Name", &name);
	ImGui::DragFloat("Damage", &damage, 0.1f);
	ImGui::DragFloat("Cooldown", &cooldown, 0.01f);
	ImGui::Checkbox("Automatic Weapon?", &isAutomatic);

	ImGui::Spacing();    // ammo stuff
	ImGui::Text("Current ammo: %i", currentAmmo);
	ImGui::Text("Available ammo: %i", availableAmmo);
	ImGui::SameLine();
	if (ImGui::SmallButton("Force reload")) {
		Reload();
	}
	ImGui::DragInt("Initial Ammo", &initialAmmo);
	ImGui::SliderInt("Magazine Size", &magazineSize, 0, 32);

	ImGui::Spacing();    // world actor stuff
	ImGui::InputText("Dropped Material Path", &droppedTexture);
	ImGui::InputText("UI Icon Image Path", &uiIconPath);
	ImGui::InputText("Aiming Animation Name", &aimingAnimation);
	ImGui::InputText("Shooting Animation Name", &shootingAnimation);

	ImGui::Spacing();    // knockback stuff
	ImGui::SeparatorText("Knockback");
	ImGui::Text("Current force: (x)%f, (y)%f", currentForce.x, currentForce.y);
	ImGui::DragFloat("Force", &maxForce);
	ImGui::Separator();
	ImGui::SliderFloat2("Falloff", &recoil.forceFalloffVec.x, 0.01f, 1.0f);
	ImGui::SliderFloat("Velocity Reduction (y-axis)", &recoil.velocityReduction, 0.0f, 1.0f);
	ImGui::SliderFloat("Force Reduction (x-axis)", &recoil.forceReduction, 1.0f, 10.0f);
	ImGui::SliderFloat("Max Speed Factor", &recoil.maxSpeedFactor, 0.1f, 10.0f);
	ImGui::SliderFloat("Opposite Boost Scale", &recoil.oppositeBoostScale, 0.01f, 1.0f);
	ImGui::SliderFloat("Ground Dampening", &recoil.groundDamp, 0.0f, 1.0f);
	ImGui::SliderFloat("Y Boost Scale", &recoil.yBoostScale, 0.0f, 5.0f);
	ImGui::SliderFloat("Y Boost Cap", &recoil.yBoostCap, 0.0f, 20.0f);
	ImGui::SliderFloat("X Impulse Scale", &recoil.xImpulseScale, 0.0f, 5.0f);
	ImGui::SliderFloat("X Impulse Cap", &recoil.xImpulseCap, 0.0f, 20.0f);

	ImGui::Spacing();
	ImGui::SeparatorText("Dispersion");
	ImGui::SliderFloat("Max Dispersion Angle", &maxDispersionAngle, 0.0f, 180.0f);
	ImGui::SliderInt("Max Bullets per Shot", &maxBulletsPerShot, 1, 12);

	ImGui::Text("%s", path());
	if (ImGui::Button("Save JSON")) {
		SaveToFile();
	}
	ImGui::SameLine();
	if (ImGui::Button("Load JSON")) {
		LoadFromFile();
	}

	ImGui::Unindent();
	ImGui::PopID();
}
#endif

bool IWeapon::CheckIfInCooldown() {
	auto now = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUsedTime).count();

	if (elapsed < static_cast<int>(cooldown * 1000)) {
		return false;    // Still in cooldown
	}

	lastUsedTime = now;
	return true;    // Cooldown done
}

void IWeapon::ResetCooldown() {
	lastUsedTime = std::chrono::steady_clock::now() - std::chrono::milliseconds(int(cooldown) * 1000);
}

bool IWeapon::HasAmmo() const {
	return currentAmmo > 0;
}

void IWeapon::Reload() {
	if (availableAmmo <= 0) {
		// no bullets :(
		return;
	}

	// only substract necessary ammo, not the whole magazine
	int ammo_to_substract = magazineSize - currentAmmo;
	currentAmmo = magazineSize < availableAmmo ? magazineSize : availableAmmo;
	availableAmmo = std::max(availableAmmo - ammo_to_substract, 0);

	currentForce = { maxForce, maxForce };

	event::Send(new WeaponReload(this));
}

auto IWeapon::GetAvailableAmmo() const -> uint8_t {
	return availableAmmo;
}

auto IWeapon::GetCurrentAmmo() const -> uint8_t {
	return currentAmmo;
}
