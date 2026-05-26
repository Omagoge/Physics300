/// @file WeaponActor.cpp
/// @author dario
/// @date 19/02/2026.

#include "WeaponActor.hpp"

#include "DefaultGun.hpp"
#include "IWeapon.hpp"
#include "SMG.hpp"
#include "Shotgun.hpp"
#include "Toast/Components/MeshRendererComponent.hpp"
#include "Toast/Physics/Rigidbody.hpp"
#include "Toast/Time.hpp"
#include "WeaponActorModified.hpp"

#ifdef TOAST_EDITOR
#include "imgui.h"
#endif

void WeaponActor::Init() {
	Actor::Init();

	SetRunEarlyTick(false);
	SetRunLateTick(false);
	SetRunTick(true);

	Rigidbody = children.AddRequired<physics::Rigidbody>();
	Rigidbody->ignorePlayer = true;
	MeshComponent = children.AddRequired<toast::MeshRendererComponent>();

	if (weaponData == nullptr) {
		ChooseWeaponType(weaponType);
	}

	weaponData->LoadFromFile();

	MeshComponent->SetMaterial(weaponData->droppedTexture);
	MeshComponent->scale(glm::vec3(.75f, .75f, 1.f));

	enabled(true);
}

void WeaponActor::Begin() {
	Actor::Begin();
	event::Send(new WeaponActorModified { this, true });
	currentScene = scene();

	MeshComponent->SetMaterial(weaponData->droppedTexture);
	MeshComponent->scale(glm::vec3(.75f, .75f, 1.f));
	// TOAST_INFO("WEAPON ACTOR BEGUN!!!");
}

void WeaponActor::Tick() {
	Actor::Tick();

	// rotate sprite if thrown
	float rotationSpeed = glm::length(Rigidbody->velocity) * 100.f * Time::delta();
	rotationSpeed += MeshComponent->rotation().z;
	MeshComponent->rotation(glm::vec3(0.f, 0.f, rotationSpeed));
}

void WeaponActor::OnEnable() {
	Actor::OnEnable();
	event::Send(new WeaponActorModified { this, true });
}

void WeaponActor::Destroy() {
	Actor::Destroy();

	delete weaponData;

	showInspector = false;

	event::Send(new WeaponActorModified { this, false });
}

void WeaponActor::Load(json_t j, bool force_create) {
	Actor::Load(j, force_create);

	if (j.contains("WeaponType")) {
		weaponType = j["WeaponType"];
	} else {
		weaponType = 0;
	}

	ChooseWeaponType(weaponType);

	weaponData->LoadFromFile();
}

json_t WeaponActor::Save() const {
	json_t j = Actor::Save();

	j["WeaponType"] = weaponType;
	return j;
}

void WeaponActor::ThrowWeapon(glm::vec2 pos, glm::vec2 force) {
	enabled(true);

	transform()->worldPosition(glm::vec3(pos.x, pos.y, .2f));

	MeshComponent->SetMaterial(weaponData->droppedTexture);
	MeshComponent->scale(glm::vec3(.75f, .75f, 1.f));

	// if no ammo letf, just do discard logic
	if (weaponData->GetAvailableAmmo() == 0 && weaponData->GetCurrentAmmo() == 0) {
		// FIXME: do anim

		scene()->children.Remove(id());
		return;
	}

	// MeshComponent->SetMaterial(weaponData->droppedTexture);
	Rigidbody->AddForce(force);
}

void WeaponActor::GrabWeapon() {
	enabled(false);
	transform()->worldPosition(glm::vec3(0.f));

	event::Send(new WeaponActorModified { this, false });
}

#ifdef TOAST_EDITOR
void WeaponActor::Inspector() {
	if (!showInspector) {
		return;
	}

	Actor::Inspector();

	ImGui::Separator();
	ImGui::SeparatorText("weapon Data");

	const char* items[] { "Gun", "Shotgun", "SMG" };
	if (ImGui::Combo("Weapon Type", &weaponType, items, IM_ARRAYSIZE(items))) {
		ChooseWeaponType(weaponType);
		weaponData->LoadFromFile();

		MeshComponent->SetMaterial(weaponData->droppedTexture);
		MeshComponent->scale(glm::vec3(.75f, .75f, 1.f));
	}

	if (!weaponData) {
		return;
	}

	ImGui::SeparatorText("Weapon Inspector");
	weaponData->Inspector();
}
#endif

void WeaponActor::ChangeWeaponType(int type) {
	weaponType = type;
	ChooseWeaponType(weaponType);
	weaponData->LoadFromFile();
	MeshComponent->SetMaterial(weaponData->droppedTexture);
	MeshComponent->scale(glm::vec3(.75f, .75f, 1.f));
}

void WeaponActor::ChooseWeaponType(unsigned w) {
	if (weaponData != nullptr) {
		delete weaponData;
	}

	switch (w) {
		case 0:    // GUNS
			weaponData = new DefaultGun;
			break;
		case 1: weaponData = new Shotgun; break;
		case 2: weaponData = new SMG; break;
		default: weaponData = new DefaultGun; break;
	}
}
