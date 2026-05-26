#include "Rat.hpp"

#include "Toast/CoroutineHandler.hpp"
#include "Toast/Renderer/OclussionVolume.hpp"
#include "Toast/WaitAsync.hpp"
#include "Weapons/RatGun.hpp"

namespace game {
void Rat::Init() {
	ShootingEnemy::Init();

	// Rigid
	m_rb->radius = 1.75f;    // Rats are bigger than cat

	// Stupid spine bs
	m_spineJsonPath = "CHARS/ENEMY1_RAT/ANIMATIONS/RAT_PUPPET.json";
	m_spineAtlasPath = "CHARS/ENEMY1_RAT/ANIMATIONS/RAT_PUPPET.atlas";
	auto enemy_atlas = resource::LoadResource<SpineAtlas>(m_spineAtlasPath);
	auto enemy_skeleton = resource::LoadResource<SpineSkeletonData>(m_spineJsonPath, enemy_atlas);
	m_spine->SetSkeletonData(enemy_skeleton);
	m_spine->scale(glm::vec3(.2f, .2f, 1.f));
	m_spine->position(glm::vec3(-0.25f, -1.75f, 0));

	// Stupid defaults
	DefaultShootParams();

	// Rigid body stuff
	m_rb->gravityScale = { 5.f, 5.f };

	// Weapons
	m_heldWeapon = new RatGun();
}

void Rat::Begin() {
	// :)
	ShootingEnemy::Begin();

	// Just in case
	m_spine->StopAnimation(0);
	m_spine->StopAnimation(1);

	// Pos based on default dir
	if (m_shootParams.spriteDirDefault < 0) {
		m_spine->position(glm::vec3(0.25f, -1.75f, 0));
		m_shootParams.currentDir = glm::vec3(-1, 0, 0);
		m_shootParams.aimDirDefault = m_shootParams.currentDir;
		ManageSpriteDir();
		ManageGunAnims();
	} else if (m_shootParams.spriteDirDefault > 0) {
		m_spine->position(glm::vec3(-0.25f, -1.75f, 0));
		m_shootParams.currentDir = glm::vec3(1, 0, 0);
		m_shootParams.aimDirDefault = m_shootParams.currentDir;
		ManageSpriteDir();
		ManageGunAnims();
	}

	// Set idle anim
	m_spine->PlayAnimation("an_rat_idle", true, 0);

	// So it works
	m_firstAim = true;
	m_idled = true;
}

void Rat::Tick() {
	// Almost forgot this, lmao
	ShootingEnemy::Tick();

	// Manual anims for now
	if (m_shootParams.idle && !m_idled && !m_shootParams.playerVisible) {
		ResetAnims();
		m_idled = true;
		m_firstAim = true;
	}

	// Reset once more
	if (!m_shootParams.idle) {
		m_idled = false;
	}
}

#pragma region Animations

void Rat::ResetAnims() {
	m_spine->StopAnimation(0);
	m_spine->StopAnimation(1);
	m_spine->StopAnimation(2);
	m_spine->StopAnimation(3);
	m_spine->PlayAnimation("an_rat_idle", true, 0);
}

void Rat::ManageGunAnims() {
	// Controller
	const glm::vec2 target_world = glm::vec2(transform()->worldPosition()) + (m_shootParams.currentDir * m_shootParams.aimDistance);
	const glm::vec2 spine_pos = m_spine->WorldPositionToSpineLocal(target_world);
	m_spine->SetBoneLocalPosition("Aim_controler", spine_pos);
}

void Rat::ManageSpriteDir() {
	// Type shit
	float threshold = 0.05f;
	if (m_shootParams.currentDir.x > threshold) {
		m_shootParams.spriteDir = 1;
		m_spine->position(glm::vec3(-0.25f, -1.75f, 0));
	} else if (m_shootParams.currentDir.x < -threshold) {
		m_shootParams.spriteDir = -1;
		m_spine->position(glm::vec3(0.25f, -1.75f, 0));
	}

	// Apply scale to sprite
	glm::vec3 currentScale = m_spine->scale();
	currentScale.x = std::abs(currentScale.x) * static_cast<float>(m_shootParams.spriteDir);
	m_spine->scale(currentScale);
}

void Rat::PlayShootIdleAnim() {
	// Managed manually for now, later...
}

void Rat::PlayAimAnim() {
	if (m_firstAim) {
		m_spine->StopAnimation(0);
		m_spine->PlayAnimation("an_rat_alert", false, 0);
		m_firstAim = false;
		// Coroutine for switching to proper aim animation...
		[](Rat* rat) -> toast::CoroutineTask {
			co_await toast::WaitSeconds(0.35f);
			if (!rat->m_awaitingDeath) {
				rat->GetSpineRenderer()->PlayAnimation("an_rat_aim", true, 0);
			}
		}(this);
	}
}

void Rat::PlayFireAnim() {
	m_spine->PlayAnimation("an_rat_shoot", false, 1);
}

void Rat::EndShootIdleAnim() {
	// Never happens
}

void Rat::EndAimAnim() {
	// This never happens
}

void Rat::EndFireAnim() {
	// It only fires once so unecessary
}

void Rat::OnDamage(float damage) {
	// TODO: hit animation?
}

void Rat::OnDeath() {
	// If we are ready to die, just die...
	if (m_deathAllowed) {
		enabled(false);
		return;
	}

	++scene()->enemy_kills;

	// Else, disable shooting and play death animation
	m_shootParams.shootPlayer = false;
	m_shootParams.shootChild = false;
	m_spine->StopAnimation(0);
	m_spine->StopAnimation(1);
	m_spine->ResetSkeletonToSetupPose();
	m_spine->PlayAnimation("an_rat_death", false, 0);
	m_awaitingDeath = true;

	// Coroutine for waiting for animation to end...
	[](Rat* rat) -> toast::CoroutineTask {
		co_await toast::WaitSeconds(0.9f);
		rat->AllowDeath(true);
		rat->OnDeath();
	}(this);
}

#pragma endregion
}
