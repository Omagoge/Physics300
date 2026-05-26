/// @file Player.hpp
/// @author Xein & Dario
/// @date 16 Feb 2026

#pragma once
#include "DustEffect.hpp"
#include "DustParticles.hpp"
#include "FollowCamera.hpp"
#include "MuzzleEffect.hpp"
#include "SpeedDust.hpp"
#include "Toast/Components/HtmlView.hpp"
#include "Toast/Input/Action.hpp"
#include "Weapons/BulletTrailEffect.hpp"
#include "Weapons/DefaultGun.hpp"
#include "Weapons/IDamageable.hpp"
#include "Weapons/IWeapon.hpp"
#include "Weapons/LaserBeamEffect.hpp"
#include "Weapons/WeaponActor.hpp"

#include <Toast/Components/SpineRendererComponent.hpp>
#include <Toast/Input/InputListener.hpp>
#include <Toast/Objects/Actor.hpp>
#include <Toast/Physics/Rigidbody.hpp>
#include <Toast/Pool.hpp>
#include <Toast/StateMachine.hpp>

struct PlayerParameters {
	// Movement
	float moveSpeed = 67.f;
	float airMoveSpeed = 20.0f;

	// Ground check
	float groundCheckDistance = 1.5f;
	bool grounded = false;
	bool isUsingGrav = false;

	float gravForce = -6.f;
	float airGravForce = -5.f;
	float releaseForce = 2.f;
	glm::vec2 gravity = glm::vec2(0.f, -1.f);

	glm::vec2 groundTangent = glm::vec2(0.f, 0.f);

	float rotationLerpSpeed = 8.0f;

	// Direction change / stopping
	float directionChangeThreshold = 18.0f;       // Velocity threshold to trigger stop animation
	float directionChangeThresholdIdle = 5.0f;    // Lower threshold to consider player idle
	float stopAnimationDuration = 0.5f;           // How long to play stop animation before turning

	// Wall rebounce
	float minVelocityToTriggerRebounce = 0.0f;
	float minRebounceWallAngle = 75.f;
	float rebounceDistanceThreshold = 6.f;
	float rebounceDampening = -0.446f;
	float rebounceYMultiplier = -1.967;

	// Collision
	float collisionSpeedThresholdX = 40.0f;
	float collisionSpeedThresholdY = 50.0f;
	float collisionReductionThreshold = 30.0f;    // Speed drop amount to consider a collision
	float collisionStateDuration = .5f;           // Time to stay in collision state

	// Stomp on ground
	float stompVerticalVelocityThreshold = -10.0f;    // Vertical speed threshold to trigger stomp landing animation
	float stompTime = 0.1f;

	// Physics tweak when colliding
	float stoppingDragGrounded = 2.0f;    // extra drag when stopping

	// weapons
	std::array<WeaponActor*, 2> weapons = { nullptr, nullptr };
	char8_t currentWeaponIndex = 0;

	// weapon throw
	bool holdingThrow = false;
	float throwHoldTime = 0.0f;
	float maxThrowHoldTime = 1.0f;    // Max time to charge throw
	float maxThrowForce = 30.0f;

	// weapon grab
	float weaponGrabRange = 3.0f;    // Max distance to grab a weapon

	// Debug
	bool showDebugVectors = true;

	glm::vec2 aimingDirection = glm::vec2(0.0f, 0.0f);
	bool isAiming = false;    ///< true while the right-stick / aim input is active

	// Aim IK — world-space point → spine local
	float aimIKDistance = 2.f;                      ///< How far along the aim direction to project the target point (world units)
	glm::vec2 aimIKWorldOffset = glm::vec2(0.f);    ///< Additional world-space offset on top of the projected point

	/// Dot-product threshold below which the aim direction is considered
	float aimFlipThreshold = 0.f;
};

class Player : public toast::Actor,
               public IDamageable {
public:
	REGISTER_TYPE(Player);
	void Init() override;
	void Begin() override;
	void EarlyTick() override;
	void PhysTick() override;
#ifdef TOAST_EDITOR
	void EditorTick() override;
#endif
	void Destroy() override;

	void OnDisable() override;

	void OnDeath() override;
	void OnDamage(float damage) override;

	void Respawn(glm::vec3 spawnPos);
	void Respawn();

#ifdef TOAST_EDITOR
	void Inspector() override;
#endif

	glm::vec2 GetMoveInput() const {
		return m.moveInput;
	}

	/// Returns the movement input projected along the ground tangent.
	float GetInputAlongTangent() const {
		glm::vec2 tangent = m.parameters.groundTangent;
		if (glm::length(tangent) < 1e-4f) {
			tangent = glm::vec2(1.f, 0.f);
		} else {
			tangent = glm::normalize(tangent);
		}
		return glm::dot(m.moveInput, tangent);
	}

	physics::Rigidbody* GetRigidbody() const {
		return m.rigidbody;
	}

	SpineRendererComponent* GetSpineRenderer() const {
		return m.sprite;
	}

	PlayerParameters* GetParameters() {
		return &m.parameters;
	}

	toast::StateMachine<Player>* GetStateMachine() {
		return &m.stateMachine;
	}

	bool IsStopping() const {
		return m.isStopping;
	}

	void SetStopping(bool stopping) {
		m.isStopping = stopping;
	}

	bool IsRunningBackwards() const {
		return m.isRunningBackwards;
	}

	void SetRunningBackwards(bool backwards) {
		m.isRunningBackwards = backwards;
	}

	float* GetStopTimer() {
		return &m.stopTimer;
	}

	int GetSpriteFacingDirection() const {
		return m.spriteFacingDirection;
	}

	void SetSpriteFacingDirection(int dir) {
		m.spriteFacingDirection = dir;
	}

	// Collision accessors
	bool IsColliding() const {
		return m.isColliding;
	}

	void SetColliding(bool colliding) {
		m.isColliding = colliding;
	}

	float* GetCollisionTimer() {
		return &m.collisionTimer;
	}

	glm::vec2 GetPreviousVelocity() const {
		return m.previousVelocity;
	}

	void SetPreviousVelocity(const glm::vec2& v) {
		m.previousVelocity = v;
	}

	void SetPlayerRecivesInput(bool b) {
		m.playerFuckingRecivesInput = b;
	}

	void SetPlayerTicksStateMachine(bool b) {
		m.tickStateMachine = b;
	}

	float GetHealth() const {
		return mHealth;
	}

	void SetRespawnPos(glm::vec3 pos) {
		m.playerRespawnPos = pos;
	}

	glm::vec3 GetRespawnPos() const {
		return m.playerRespawnPos;
	}

	void SetInvincible(bool invincible) {
		mInvincible = invincible;
	}

	DustParticles* GetDustParticles() const {
		return m.dustParticles;
	}

	toast::ParticleSystem* GetOnAirParticles() const {
		return m.onAirParticles;
	}

	void Reload();

	inline IWeapon* GetWeaponData(int idx);
	inline IWeapon* GetCurrentWeaponData();

	void DoDustEffect(int type) const;
	void DoDustParticles(bool enable) const;

	void ReapplyAimingIfActive();

	double milliseconds = 0;
	int shotNumber = 0;
	double timeOnAir = 0;
	double timeOnGravity = 0;
	double maxSpeed = 0;
	int hitNumber = 0;

private:
	void RefreshJavascript();
	bool m_flashingActive = false;
	int m_flashRemaining = 0;       // remaining toggle steps
	int m_flashTotalToggles = 0;    // total toggles for the flash sequence
	float m_flashTimer = 0.0f;      // countdown until next toggle

	bool m_waitingForRespawn = false;
	float m_respawnDelayTimer = 0.0f;

	bool m_waitingForDeathActions = false;
	float m_deathDelayTimer = 0.0f;

	bool m_triggerArmed = true; // this is for PS5 Dualsense Controllers only and is used to only trigger once gone past actuation point.

	// Pending discard timers for delayed throw/check after shooting out of ammo
	std::vector<std::pair<IWeapon*, float>> m_pendingDiscardTimers;
	// Input callbacks
	void OnMove(const input::Action2D* a);
	void OnAim(const input::Action2D* a);
	void OnShoot(const input::Action1D* a);
	void OnGravGun(const input::Action1D* a);
	void OnSwitchWeapon(const input::Action0D* a);
	void OnThrow(const input::Action0D* a);

	char GetAvailableWeaponSlot() const;

	void Shoot() const;
	void Aim();
	void Throw();
	void Grab(WeaponActor* weapon);

	void GroundCheck();
	void UpdateSpriteRotation();
	void UpdateSpriteScale();
	void CollisionCheck();

	void UpdateCameraOffset();
	void UpdateDepthOfFieldFocusDistance();

	// Weapon switching
	void SwitchWeapon();

	bool m_reloadPending = false;
	float m_reloadDelayTimer = 0.0f;

	std::optional<WeaponActor*> IsWeaponInRange() const;

	static constexpr float INVINCIBILITY_DURATION = 1.5f;
	static constexpr float RESPAWN_HEALTH = 3.0f;
	static constexpr float MAX_HEALTH = 3.0f;
	static constexpr float HEALTH_RECHARGE_DURATION = 20.0f;    ///< seconds for a single missing health segment to recharge

	struct {
		PlayerParameters parameters;

		toast::StateMachine<Player> stateMachine;
		input::Listener input;

		FollowCamera* camera = nullptr;

		toast::HtmlView* fade = nullptr;

		// Respawn Shit
		glm::vec3 playerRespawnPos = glm::vec3(0.0f);
		bool playerFuckingRecivesInput = true;
		bool tickStateMachine = true;
		float invincibilityTimer = 0.0f;    ///< counts down; player is invincible while > 0

		// SPINERENDERER
		SpineRendererComponent* sprite = nullptr;

		physics::Rigidbody* rigidbody = nullptr;

		std::vector<toast::Object*> worldWeaponsList;

		// Movement
		glm::vec2 moveInput = glm::vec2(0.0f, 0.0f);

		// Effects
		MuzzleEffect* muzzleEffect = nullptr;
		DustEffect* dustEffect = nullptr;
		DustParticles* dustParticles = nullptr;
		DustParticles* dustParticlesWall = nullptr;
		SpeedDust* speedDust = nullptr;
		toast::ParticleSystem* onAirParticles = nullptr;
		Pool<WallBulletHitEffect, 6>* bulletHitEffects = nullptr;
		Pool<BulletTrailEffect, 12>* bulletTrailEffects = nullptr;

		LaserBeamEffect* laserBeamEffect = nullptr;

		// Rotation sprite stuff
		float currentRotationZ = 0.0f;
		float targetRotationZ = 0.0f;
		glm::vec3 currentPosition = glm::vec3(0.0f);

		// Sprite facing and stopping
		int spriteFacingDirection = 1;
		bool isStopping = false;
		float stopTimer = 0.0f;
		bool isRunningBackwards = false;    ///< true when velocity direction opposes the sprite facing (backwards run anim)

		glm::vec2 feetPosition = glm::vec2(0.0f);
		float feetRotation = 0.f;

		// Camera offset
		float defaultZoomOffset = 15.0f;
		glm::vec3 cameraSpeedModifier = { 1.5f, 1.5f, 0.25f };
		glm::vec2 cameraMaxOffset = { 40.0f, 20.0f };

		// Collision state tracking
		glm::vec2 previousVelocity = glm::vec2(0.f);    // velocity from previous physics tick
		bool isColliding = false;                       // currently in collision state
		float collisionTimer = 0.0f;                    // timer for collision state

		// Health recharge (authoritative)
		float healthRechargeTimer = 0.0f;    ///< accumulates time for active recharge
		bool rechargeActive = false;         ///< true when a recharge is in progress
		int rechargeTargetIndex = -1;        ///< 0-based index of the segment being recharged

		float blinkTimer = 0.0f;
		float blinkTime = 5.0f;

	} m;
};
