#pragma once

#include "Player/Player.hpp"
#include "Toast/Components/SpineRendererComponent.hpp"
#include "Toast/StateMachine.hpp"
#include "Weapons/BulletTrailEffect.hpp"
#include "Weapons/EnemyShootWarningEffect.hpp"
#include "Weapons/IDamageable.hpp"
#include "Weapons/IWeapon.hpp"

#include <Toast/Objects/Actor.hpp>
#include <Toast/Pool.hpp>

namespace game {

struct ShootingEnemyParameters {
	// Aim parameters
	float interpFactor = 2.5f;    // Interpolation factor between current aim dir and desired aim dir when far away
	float constSpeed = 7.f;
	float aimDistance = 5.f;
	float muzzleDistance = 0.25f;
	float occlusionDistance = 0.45f;    // Beyond this, does nothing
	int spriteDir = 1;
	int spriteDirDefault = 1;

	// Timers
	float aimTime = 1.f;
	float aimTimer = 0.f;
	float cooldownTime = 1.75f;
	float cooldownTimer = 0.f;
	float shootTime = 2.f;
	float shootTimer = 0.f;
	float idleTime = 3.f;
	float idleTimer = 0.f;

	// Aim color and directions
	glm::vec4 aimColor = { 1.f, 1.f, 1.f, 1.f };
	glm::vec2 currentDir = { 0.f, 0.f };
	glm::vec2 aimDir = { 0.f, 0.f };
	glm::vec2 aimDirDefault = { 0.f, -1.f };
	glm::vec2 lastPlayerPos = { 0.f, 0.f };
	glm::vec2 muzzlePos = { 0.f, 0.f };

	// Feature Bools
	bool doDamage = true;
	bool shootPlayer = true;
	bool shootChild = false;

	// State Control Bools
	bool playerVisible = false;
	bool initialAim = true;
	bool coolingDown = false;
	bool oldPosStored = false;

	// Condition Bools
	bool firing = false;
	bool idle = false;
	bool warned = false;
};

class ShootingEnemy : public toast::Actor,
                      public IDamageable {
public:
	void Init() override;

	void Begin() override;

	void Tick() override;

#ifdef TOAST_EDITOR
	void Inspector() override;
#endif

	// Weapon Helpers
	virtual void Shoot();
	virtual void Warn();
	virtual void ManageCooldowns();

	// Directional Helpers
	virtual void ManageAimingDirection(const glm::vec2& own_pos, const glm::vec2& dir);
	virtual void ManageAimingCriteria(const glm::vec2& target_pos);
	virtual void SelectTarget(const glm::vec2& own_pos, glm::vec2& target_pos, glm::vec2& target_dir);
	virtual void ManageGunAnims();
	virtual void ManageSpriteDir();

	ShootingEnemyParameters* GetShootParameters() {
		return &m_shootParams;
	}

	toast::StateMachine<ShootingEnemy>* GetShootStateMachine() {
		return &m_shootSM;
	}

	IWeapon* GetWeapon() {
		return m_heldWeapon;
	}

	MuzzleEffect* GetMuzzleEffect() {
		return m_muzzleEffect;
	}

	// TODO: this is shit, but I have 2 state machines and inheritance so idk how otherwise rn -ax
	virtual void PlayShootIdleAnim() = 0;
	virtual void PlayAimAnim() = 0;
	virtual void PlayFireAnim() = 0;
	virtual void EndShootIdleAnim() = 0;
	virtual void EndAimAnim() = 0;
	virtual void EndFireAnim() = 0;
	virtual void ResetAnims() = 0;

	[[nodiscard]]
	json_t Save() const override;
	void Load(json_t j, bool force_create = true) override;

	SpineRendererComponent* GetSpineRenderer() {
		return m_spine;
	}

	virtual void DefaultShootParams() {
		const int default_dir = m_shootParams.spriteDirDefault;
		const bool do_damage = m_shootParams.doDamage;
		const bool shoot_player = m_shootParams.shootPlayer;
		const bool shoot_child = m_shootParams.shootChild;
		m_shootParams = {};
		m_shootParams.doDamage = do_damage;
		m_shootParams.shootPlayer = shoot_player;
		m_shootParams.shootChild = shoot_child;
		m_shootParams.spriteDirDefault = default_dir;
	}

	// getter for Enemy Clear Door
	const std::string& GetDoorTag() const {
		return m_doorTag;
	}

	virtual void ResetOnBegin() {
		// Reset to default state
		m_shootSM.SetState("Idle");

		// Reset bools
		m_shootParams.oldPosStored = false;
		m_shootParams.idle = true;

		// Reset Timers
		m_shootParams.idleTimer = m_shootParams.idleTime;
		m_shootParams.aimTimer = 0.f;
		m_shootParams.cooldownTimer = 0.f;
		m_shootParams.shootTimer = 0.f;

		// Set aim dir to default
		m_shootParams.aimDir = m_shootParams.aimDirDefault;
		m_shootParams.currentDir = m_shootParams.aimDirDefault;

		// Modify gun anims and dir accordingly
		ManageGunAnims();
		ManageSpriteDir();
	}

	void AllowDeath(const bool value) {
		m_deathAllowed = value;
	}

protected:
	// State stuff
	bool m_deathAllowed = false;     // For death animations
	bool m_awaitingDeath = false;    // For disabling shooting during death anim
	bool m_firstAim = true;          // For first aim animations
	bool m_idled = true;             // For detecting idle

	// Components
	physics::Rigidbody* m_rb = nullptr;
	SpineRendererComponent* m_spine = nullptr;
	Actor* m_target = nullptr;

	// Parameters
	ShootingEnemyParameters m_shootParams;

	// State
	toast::StateMachine<ShootingEnemy> m_shootSM;

	// Player and current weapon
	Player* m_player = nullptr;
	IWeapon* m_heldWeapon = nullptr;
	MuzzleEffect* m_muzzleEffect = nullptr;
	Pool<WallBulletHitEffect, 5>* m_bulletHitEffects = nullptr;
	Pool<BulletTrailEffect, 5>* m_bulletTrailEffects = nullptr;
	Pool<EnemyShootWarningEffect, 2>* m_enemyWarningEffects = nullptr;

	// tag used by EnemyClearDoor to find enemy, (tried to do drag and drop but editor doesnt allow xDDD)
	std::string m_doorTag = "";
	std::string m_spineJsonPath = "";
	std::string m_spineAtlasPath = "";
};
}
