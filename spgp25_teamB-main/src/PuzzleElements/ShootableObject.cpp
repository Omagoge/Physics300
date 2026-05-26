#include "ShootableObject.hpp"

#include "Toast/CoroutineHandler.hpp"
#include "Toast/WaitAsync.hpp"
#ifdef TOAST_EDITOR
#include "imgui.h"

#include <imgui_stdlib.h>    // for ImGui::InputText
#endif

void ShootableObject::Init() {
	Actor::Init();

	// Components
	m_sprite = children.AddRequired<toast::MeshRendererComponent>();
	m_rb = children.AddRequired<physics::Rigidbody>();

	// Component Defaults
	m_rb->hasGravity = false;
	m_rb->ignorePlayer = true;
	m_rb->flags = ColliderFlags::Enemy;
	m_rb->mass = 1000.f;    // These should not move...
}

void ShootableObject::Begin() {
	Actor::Begin();

	// Defaults
	ResetOnBegin();
}

void ShootableObject::ResetOnBegin() {
	// Defaults
	enabled(true);
	m_rb->enabled(true);
	m_sprite->enabled(true);
	mHealth = 1;
	mDeathAllowed = false;
}

#ifdef TOAST_EDITOR
void ShootableObject::Inspector() {
	Actor::Inspector();

	ImGui::PushID(this);

	ImGui::SeparatorText("");
	ImGui::DragFloat("Health", &mHealth, 0.01f);
	ImGui::DragFloat("Death Time", &mDeathTime, 0.01f);
	ImGui::Checkbox("Death Allowed", &mDeathAllowed);
	ImGui::Checkbox("Invincible", &mInvincible);
	ImGui::Spacing();
	ImGui::InputText("Door Tag", &mDoorTag);
	ImGui::Spacing();

	ImGui::PopID();
}
#endif

void ShootableObject::OnDeath() {
	if (mDeathAllowed) {
		DeathParticles();
		enabled(false);
		return;
	}

	// TODO: for now I just disable components ig?
	m_sprite->enabled(false);
	m_rb->enabled(false);

	// Coroutine for waiting for animation to end...
	[death_time = mDeathTime](ShootableObject* obj) -> toast::CoroutineTask {
		co_await toast::WaitSeconds(death_time);
		obj->AllowDeath(true);
		obj->OnDeath();
	}(this);
}

void ShootableObject::DeathParticles() {
	// TODO: make and play a particle here gang. Maybe switch for preset colors.
}

[[nodiscard]]
json_t ShootableObject::Save() const {
	json_t j = Actor::Save();

	j["Invincible"] = mInvincible;
	j["doorTag"] = mDoorTag;

	return j;
}

void ShootableObject::Load(json_t j, bool force_create) {
	Actor::Load(j, force_create);

	if (j.contains("Invincible")) {
		mInvincible = j["Invincible"].get<bool>();
	}
	if (j.contains("doorTag")) {
		mDoorTag = j["doorTag"].get<std::string>();
	}
}
