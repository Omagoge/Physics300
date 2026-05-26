#include "Dog.hpp"

#include "Toast/Renderer/OclussionVolume.hpp"
#include "Toast/Time.hpp"
#include "Weapons/DogGun.hpp"

namespace game {
void Dog::Init() {
	// Base Init
	ShootingEnemy::Init();

	// Rigid
	m_rb->radius = 2.25f;    // Dogs are big and intimidating

	// Stupid spine bs
	m_spineJsonPath = "CHARS/ENEMY2_DOG/ANIMATIONS/dog puppet.json";
	m_spineAtlasPath = "CHARS/ENEMY2_DOG/ANIMATIONS/dog puppet.atlas";
	auto enemy_atlas = resource::LoadResource<SpineAtlas>(m_spineAtlasPath);
	auto enemy_skeleton = resource::LoadResource<SpineSkeletonData>(m_spineJsonPath, enemy_atlas);
	m_spine->SetSkeletonData(enemy_skeleton);
	m_spine->scale(glm::vec3(.5f, .5f, 1.f));
	m_spine->position(glm::vec3(0, -2, 0));

	// Stupid defaults
	DefaultShootParams();

	// Rigid body stuff
	m_rb->gravityScale = { 5.f, 5.f };

	// Weapons
	m_heldWeapon = new DogGun();
}

void Dog::Begin() {
	// Base begin
	ShootingEnemy::Begin();

	// Just in case
	m_spine->StopAnimation(0);
	m_spine->StopAnimation(1);
	m_spine->StopAnimation(2);

	// Pos based on default dir
	if (m_shootParams.spriteDirDefault > 0) {
		m_spine->position(glm::vec3(0.5f, -2.f, 0));
		m_shootParams.currentDir = glm::vec3(-1, 0, 0);
		m_shootParams.aimDirDefault = m_shootParams.currentDir;
		ManageSpriteDir();
		ManageGunAnims();
	} else if (m_shootParams.spriteDirDefault < 0) {
		m_spine->position(glm::vec3(-0.5f, -2.f, 0));
		m_shootParams.currentDir = glm::vec3(1, 0, 0);
		m_shootParams.aimDirDefault = m_shootParams.currentDir;
		ManageSpriteDir();
		ManageGunAnims();
	}

	// Set idle anim
	m_spine->PlayAnimation("an_dog_idle", true, 0);

	// So it works
	m_firstAim = true;
	m_idled = true;
}

void Dog::Tick() {
	// Base Tick
	ShootingEnemy::Tick();

	// Manual anims for now
	if (m_shootParams.idle && !m_idled && !m_shootParams.playerVisible) {
		ResetAnims();
		m_idled = true;
		m_firstAim = true;
	}

	// Damaged animation handling
	if (m_damaged) {
		m_damagedTimer += Time::delta();
		if (m_damagedTimer >= m_damagedTime) {
			m_spine->PlayAnimation("an_dog_idle", true, 0);
			m_spine->PlayAnimation("an_dog_aim", true, 1);
			m_damaged = false;
		}
	}

	// Reset again
	if (!m_shootParams.idle) {
		m_idled = false;
	}
}

#pragma region Animations

void Dog::ManageGunAnims() {
	// Controller
	const glm::vec2 target_world = glm::vec2(transform()->worldPosition()) + (m_shootParams.currentDir * m_shootParams.aimDistance);
	const glm::vec2 spine_pos = m_spine->WorldPositionToSpineLocal(target_world);
	m_spine->SetBoneLocalPosition("Aim", spine_pos);
}

void Dog::ManageSpriteDir() {
	// Type shit
	float threshold = 0.05f;
	if (m_shootParams.currentDir.x > threshold) {
		m_shootParams.spriteDir = -1;
		m_spine->position(glm::vec3(0.5f, -2.f, 0));
	} else if (m_shootParams.currentDir.x < -threshold) {
		m_shootParams.spriteDir = 1;
		m_spine->position(glm::vec3(-0.5f, -2.f, 0));
	}

	// Apply scale to sprite
	glm::vec3 currentScale = m_spine->scale();
	currentScale.x = std::abs(currentScale.x) * static_cast<float>(m_shootParams.spriteDir);
	m_spine->scale(currentScale);
}

void Dog::OnDamage(float damage) {
	m_damagedTimer = 0.f;
	m_damaged = true;
	m_spine->PlayAnimation("an_dog_taking_damage", false, 2);
}

void Dog::ResetAnims() {
	m_spine->StopAnimation(0);
	m_spine->StopAnimation(1);
	m_spine->StopAnimation(2);
	m_spine->StopAnimation(3);
	m_spine->PlayAnimation("an_dog_idle", true, 0);
}

void Dog::PlayShootIdleAnim() { }

void Dog::PlayAimAnim() {
	if (m_firstAim) {
		m_spine->PlayAnimation("an_dog_aim", true, 1);
		m_firstAim = false;
	}
}

void Dog::PlayFireAnim() {
	m_spine->PlayAnimation("an_dog_Shoot", false, 3);
}

void Dog::EndShootIdleAnim() { }

void Dog::EndAimAnim() { }

void Dog::EndFireAnim() { }

#pragma endregion
}
