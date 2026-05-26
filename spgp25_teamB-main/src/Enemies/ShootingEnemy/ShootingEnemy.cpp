#define GLM_ENABLE_EXPERIMENTAL

#include "ShootingEnemy.hpp"

#include "Fire.hpp"
#include "Idle.hpp"
#include "Toast/Physics/Raycast.hpp"

#include <Toast/Pool.hpp>
#ifdef TOAST_EDITOR
#include "imgui.h"
#endif
#include "Player/GameplayManager.hpp"
#include "Toast/Renderer/OclussionVolume.hpp"
#include "Toast/World.hpp"
#ifdef TOAST_EDITOR
#include <imgui_stdlib.h>    // for ImGui::InputText EnemyClearDoor
#endif

using namespace shootEnemy_states;

static float constexpr E = std::numeric_limits<float>::epsilon();

namespace game {
void ShootingEnemy::Init() {
	// Base Init
	Actor::Init();

	// Components
	m_spine = children.AddRequired<SpineRendererComponent>();
	m_rb = children.AddRequired<physics::Rigidbody>();
	m_target = children.AddRequired<Actor>("Enemy Target");

	// Component Defaults
	m_rb->hasGravity = true;
	m_rb->ignorePlayer = true;
	m_rb->flags = ColliderFlags::Enemy;
	m_rb->mass = 5000.f;    // Enemies are all fatasses like me -ax

	// States
	m_shootSM.SetParent(this);
	m_shootSM.AddState("Idle", std::make_unique<Idle>());
	m_shootSM.AddState("Fire", std::make_unique<Fire>());

	// Muzzle, Hits and Trails
	m_muzzleEffect = children.AddRequired<MuzzleEffect>();
	m_bulletHitEffects = children.Add<Pool<WallBulletHitEffect, 5>>();
	m_bulletTrailEffects = children.Add<Pool<BulletTrailEffect, 5>>();
	m_enemyWarningEffects = children.Add<Pool<EnemyShootWarningEffect, 2>>();
}

void ShootingEnemy::Begin() {
	// Begin logic
	Actor::Begin();

	// Defaults
	mHealth = 1;
	m_shootParams.playerVisible = false;
	m_shootParams.firing = false;
	m_deathAllowed = false;
	m_awaitingDeath = false;
	m_shootParams.initialAim = true;
	m_shootParams.spriteDir = m_shootParams.spriteDirDefault;
	ManageSpriteDir();

	// Enable this + components
	enabled(true);
	m_spine->enabled(true);
	m_rb->enabled(true);

	// Enable Effects (Does not fucking work for pools...)
	m_muzzleEffect->enabled(true);
	m_bulletHitEffects->enabled(true);
	m_bulletTrailEffects->enabled(true);
	m_enemyWarningEffects->enabled(true);

	// Get Player
	m_player = GameplayManager::GetPlayer();

	// Set Default State
	m_shootSM.SetState("Idle");

	// Load Weapon
	m_heldWeapon->LoadFromFile();
	m_heldWeapon->damage = m_shootParams.doDamage ? m_heldWeapon->damage : 0;

	// Set weapon collision flag just in case ig?
	m_heldWeapon->collisionFlag = ColliderFlags::Ground | ColliderFlags::Player;
}

void ShootingEnemy::Tick() {
	// Sanity
	if (!m_heldWeapon || !OclussionVolume::isSphereOnPlanes(transform()->position(), m_shootParams.occlusionDistance) || Time::delta() == 0.0f ||
	    m_awaitingDeath) {
		if (!m_idled) {
			ResetOnBegin();
		}
		m_idled = true;
		m_firstAim = true;
		return;
	};

	// Sanity Reset
	if (!m_shootParams.idle) {
		m_idled = false;
	}

	// Position and Direction vectors
	const glm::vec2 own_pos = transform()->position();
	glm::vec2 target_pos = own_pos;
	glm::vec2 target_dir = m_shootParams.aimDirDefault;

	// Select target
	SelectTarget(own_pos, target_pos, target_dir);

	// Manage aiming directions
	ManageAimingDirection(own_pos, target_dir);

	// Raycast to see if player is visible
	m_shootParams.playerVisible = false;
	auto ray_sult = physics::RayCast(own_pos, target_dir, ColliderFlags::Player | ColliderFlags::Ground);
	if (ray_sult != std::nullopt) {
		if (Player* player = dynamic_cast<Player*>(ray_sult->other)) {
			m_shootParams.playerVisible = true;
		}
	}

	// Draws a line to aim at player with using currentDir
	// renderer::DebugLine(own_pos, own_pos + m_shootParams.currentDir * glm::vec2(10, 10), m_shootParams.aimColor);

	// Get muzzle position
	m_shootParams.muzzlePos = m_spine->GetBoneWorldPosition(m_heldWeapon->effectSocket) + (m_shootParams.currentDir * m_shootParams.muzzleDistance);

	// State machine tick
	m_shootSM.Tick();

	// Manage Cooldowns and Aim Criteria
	ManageCooldowns();
	ManageAimingCriteria(target_pos);

	// Update gun aiming position and sprite axis
	ManageGunAnims();
	ManageSpriteDir();
}

#pragma region Helper Functions

void ShootingEnemy::SelectTarget(const glm::vec2& own_pos, glm::vec2& target_pos, glm::vec2& target_dir) {
	// If shooting child, child is target
	if (m_shootParams.shootChild && !m_shootParams.playerVisible) {
		// Choose target as target
		m_shootParams.shootChild = true;
		target_pos = m_target->transform()->position();
		target_pos += own_pos;
		target_dir = target_pos - own_pos;

		// Normalize target dir
		if (glm::dot(target_dir, target_dir) > E) {
			target_dir = glm::normalize(target_dir);
		}

		// Set current dir directly
		m_shootParams.aimDir = target_dir;
		m_shootParams.currentDir = target_dir;

		// Otherwise, player is target
	} else if (m_player) {
		// Choose player as target
		target_pos = m_player->transform()->position();
		target_dir = target_pos - own_pos;

		// Normalize player dir
		if (glm::dot(target_dir, target_dir) > E) {
			target_dir = glm::normalize(target_dir);
		}

		// Get diff between points
		glm::vec2 dir_diff = m_shootParams.aimDir - m_shootParams.currentDir;
		float diff_length = glm::length(dir_diff);

		// Factor based on distance
		float factor = std::max(m_shootParams.constSpeed, diff_length * m_shootParams.interpFactor);

		// Interpolate current point based on factor
		m_shootParams.currentDir += dir_diff * factor * static_cast<float>(Time::delta());

		// Normalize current dir
		if (glm::dot(m_shootParams.currentDir, m_shootParams.currentDir) > E) {
			m_shootParams.currentDir = glm::normalize(m_shootParams.currentDir);
		}
	}
}

void ShootingEnemy::ManageCooldowns() {
	// Cooldown between shots
	if (m_shootParams.coolingDown) {
		m_shootParams.cooldownTimer += Time::delta();
	}

	// Reset cooldown
	if (m_shootParams.cooldownTimer >= m_shootParams.cooldownTime) {
		m_shootParams.cooldownTimer = 0.f;
		m_shootParams.coolingDown = false;
	}
}

void ShootingEnemy::ManageAimingCriteria(const glm::vec2& target_pos) {
	// Old pos and idle timer logic
	if (m_shootParams.playerVisible) {
		m_shootParams.oldPosStored = false;
		m_shootParams.initialAim = false;
		m_shootParams.idleTimer = 0.f;
	} else {
		// Increment Idle timer
		m_shootParams.idleTimer += Time::delta();

		// If pos not stored, store it
		if (!m_shootParams.oldPosStored) {
			m_shootParams.lastPlayerPos = target_pos;
			m_shootParams.oldPosStored = true;
		}
	}
}

void ShootingEnemy::ManageAimingDirection(const glm::vec2& own_pos, const glm::vec2& dir) {
	// CONSTANTLY SHOOT AT CHILD IF YES
	if (m_shootParams.shootChild) {
		m_shootParams.aimDir = dir;
	}

	// INITIAL AIMING DIRECTION
	else if (m_shootParams.initialAim) {
		m_shootParams.aimDir = m_shootParams.aimDirDefault;
	}

	// OTHERWISE, ACTUAL LOGIC!
	else if (m_shootParams.oldPosStored) {
		// IDLE AIMING DIRECTION
		if (m_shootParams.idleTimer >= m_shootParams.idleTime) {
			// Mark idle for animations
			m_shootParams.idle = true;

			// Set dir to default
			m_shootParams.aimDir = m_shootParams.aimDirDefault;

			// OLD AIMING DIRECTION
		} else {
			// Set dir to last player pos
			m_shootParams.aimDir = m_shootParams.lastPlayerPos - own_pos;

			// Normalize dir
			if (glm::dot(m_shootParams.aimDir, m_shootParams.aimDir) > E) {
				m_shootParams.aimDir = glm::normalize(m_shootParams.aimDir);
			}
		}
		// CURRENT AIMING DIRECTION
	} else {
		m_shootParams.idle = false;
		m_shootParams.aimDir = dir;
	}
}

void ShootingEnemy::ManageSpriteDir() {
	// Type shit
	float threshold = 0.05f;
	if (m_shootParams.currentDir.x > threshold) {
		m_shootParams.spriteDir = 1;
	} else if (m_shootParams.currentDir.x < -threshold) {
		m_shootParams.spriteDir = -1;
	}

	// Apply scale to sprite
	glm::vec3 current_scale = m_spine->scale();
	current_scale.x = std::abs(current_scale.x) * static_cast<float>(m_shootParams.spriteDir);
	m_spine->scale(current_scale);
}

void ShootingEnemy::ManageGunAnims() {
	// Controller
	const glm::vec2 target_world = glm::vec2(transform()->worldPosition()) + (m_shootParams.currentDir * m_shootParams.aimDistance);
	const glm::vec2 spine_pos = m_spine->WorldPositionToSpineLocal(target_world);
	m_spine->SetBoneLocalPosition("gun_controler", spine_pos);
}

void ShootingEnemy::Warn() {
	// Play warning effect
	//  WHY IS THIS SHIT A POOL!?
	auto* warn_pool = m_enemyWarningEffects;
	auto* warn = warn_pool->Release();
	if (warn) {
		warn->EnableParticles();
		warn->PlayEffect(&m_shootParams.muzzlePos, [warn_pool, warn]() {
			warn_pool->Hold(warn);
		});
	}
}

void ShootingEnemy::Shoot() {
	// Get angle
	float angle = std::atan2(m_shootParams.currentDir.y, m_shootParams.currentDir.x) * 180.f / glm::pi<float>();

	// Fire
	const auto result = m_heldWeapon->Shoot(m_shootParams.currentDir, transform()->position());

	// Effect
	m_muzzleEffect->PlayEffect(m_shootParams.muzzlePos, angle, m_heldWeapon->muzzleAnimName);

	// bullet hit + trail effects
	if (result.has_value()) {
		auto* hit_pool = m_bulletHitEffects;
		auto* trail_pool = m_bulletTrailEffects;

		for (const auto& c : result->collisions) {
			// Trail to hit point
			auto* trail = trail_pool->Release();
			if (trail) {
				trail->EnableRenderer();
				trail->PlayEffect(m_shootParams.muzzlePos, c.point, [trail_pool, trail]() {
					trail_pool->Hold(trail);
				});
			}

			// Hit spark
			auto* effect = hit_pool->Release();
			if (effect) {
				effect->EnableParticles();
				effect->PlayEffect(c.point, c.normal, [hit_pool, effect]() {
					hit_pool->Hold(effect);
				});
			}
		}

		constexpr float MISS_RANGE = 50.0f;
		for (const auto& missDir : result->missedDirections) {
			auto* trail = trail_pool->Release();
			if (trail) {
				trail->EnableRenderer();
				const glm::vec2 missEnd = m_shootParams.muzzlePos + missDir * MISS_RANGE;
				trail->PlayEffect(m_shootParams.muzzlePos, missEnd, [trail_pool, trail]() {
					trail_pool->Hold(trail);
				});
			}
		}
	} else {
	}

	// Back to idle if we are finished shooting
	if (m_shootParams.shootTimer >= m_shootParams.shootTime) {
		m_shootParams.cooldownTimer = 0.f;
		m_shootParams.coolingDown = true;
		m_shootSM.SetState("Idle");
	}
}

#pragma endregion

#pragma region Inspector/Save/Load
#ifdef TOAST_EDITOR
void ShootingEnemy::Inspector() {
	Actor::Inspector();

	ImGui::PushID(this);

	ImGui::Spacing();

	if (ImGui::Button("Round Current Position")) {
		glm::vec3 pos = transform()->position();
		pos.x = std::round(pos.x);
		pos.y = std::round(pos.y);
		pos.z = std::round(pos.z);
		transform()->position(pos);
	}

	ImGui::Spacing();

	if (ImGui::CollapsingHeader("Enemy Variables:")) {
		ImGui::Spacing();
		ImGui::DragFloat("Health", &mHealth, 0.5f, 0.f);
		ImGui::DragInt("Default Sprite Dif", &m_shootParams.spriteDirDefault, 1, -1, 1);
		ImGui::Checkbox("Invincible", &mInvincible);
		ImGui::Checkbox("Enable Damage", &m_shootParams.doDamage);
		ImGui::Checkbox("Shoot Player", &m_shootParams.shootPlayer);
		ImGui::Checkbox("Shoot Child", &m_shootParams.shootChild);
		ImGui::Spacing();
		ImGui::InputText("Door Tag", &m_doorTag);
		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Aim Parameters")) {
		ImGui::Spacing();
		ImGui::DragFloat("Aim Time", &m_shootParams.aimTime, 0.05f);
		ImGui::DragFloat("Shoot Time", &m_shootParams.shootTime, 0.05f);
		ImGui::DragFloat("Cooldown Time", &m_shootParams.cooldownTime, 0.05f);
		ImGui::DragFloat("Interpolation Factor", &m_shootParams.interpFactor, 0.05f);
		ImGui::DragFloat("Close Aim Speed", &m_shootParams.constSpeed, 0.05f);
		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Held Weapon")) {
		ImGui::Spacing();
		m_heldWeapon->Inspector();
		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("General Debug")) {
		ImGui::Spacing();
		ImGui::DragFloat("Aim Distance", &m_shootParams.aimDistance, 0.05f);
		ImGui::DragFloat("Muzzle Distance", &m_shootParams.muzzleDistance, 0.05f);
		ImGui::DragFloat("Occlusion Distance", &m_shootParams.occlusionDistance, 0.05f);
		ImGui::Checkbox("Player Visible", &m_shootParams.playerVisible);
		ImGui::Checkbox("Firing", &m_shootParams.firing);
		ImGui::Spacing();

		if (ImGui::Button("Reset Aim Params")) {
			DefaultShootParams();
		}

		ImGui::Spacing();
	}

	ImGui::PopID();
}
#endif

[[nodiscard]]
json_t ShootingEnemy::Save() const {
	json_t j = Actor::Save();

	j["Default Sprite Dir"] = m_shootParams.spriteDirDefault;
	j["Enable Shooting"] = m_shootParams.shootPlayer;
	j["Do Damage"] = m_shootParams.doDamage;
	j["Invincible"] = mInvincible;
	j["Shoot Child"] = m_shootParams.shootChild;
	j["doorTag"] = m_doorTag;

	return j;
}

void ShootingEnemy::Load(json_t j, bool force_create) {
	Actor::Load(j, force_create);

	if (j.contains("Default Sprite Dir")) {
		m_shootParams.spriteDirDefault = j["Default Sprite Dir"].get<int>();
	}
	if (j.contains("Enable Shooting")) {
		m_shootParams.shootPlayer = j["Enable Shooting"].get<bool>();
	}
	if (j.contains("Do Damage")) {
		m_shootParams.doDamage = j["Do Damage"].get<bool>();
	}
	if (j.contains("Invincible")) {
		mInvincible = j["Invincible"].get<bool>();
	}
	if (j.contains("Shoot Child")) {
		m_shootParams.shootChild = j["Shoot Child"].get<bool>();
	}
	if (j.contains("doorTag")) {
		m_doorTag = j["doorTag"].get<std::string>();
	}

}

#pragma endregion
};
