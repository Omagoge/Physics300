/// @file IWeapon.hpp
/// @author alexey, dario & xein
/// @date 27 Jan 2026

#pragma once
#include "Toast/Pool.hpp"
#include "WallBulletHitEffect.hpp"

#include <Toast/ISerializable.hpp>
#include <Toast/Physics/ColliderFlags.hpp>
#include <Toast/Physics/Raycast.hpp>
#include <chrono>
#include <glm/glm.hpp>
#include <optional>
#include <string>

struct ShootingResult {
	float distance;
	glm::vec2 knockbackForce;
	std::vector<physics::RayResult> collisions;
	std::vector<glm::vec2> missedDirections;
};

struct RecoilConfiguration {
	float velocityReduction = 0.8f;                 ///< Reduces player velocity in y-axis when shooting
	float forceReduction = 10.0f;                   ///< Velocity to multiply dealt force in x-axis by 0.8
	glm::vec2 forceFalloffVec = { 0.9f, 0.75f };    ///< How much the force reduces when shooting again

	float maxSpeedFactor = 3.f;                     // cap for speed contribution
	float oppositeBoostScale = 2.f;                 // additional multiplier
	float groundDamp = 0.6f;                        // reduce X force when grounded
	float yBoostScale = 1.f;                        // vertical boost scale relative to mag
	float yBoostCap = 10.f;                         // max vertical boost
	float xImpulseScale = 0.06f;                    // small X impulse when flipping direction
	float xImpulseCap = 3.0f;
};

enum class WeaponSpriteType : uint8_t {
	Guns = 0,
	Shotgun = 1,
	SMG = 2,
};

inline const char* to_string(WeaponSpriteType e) {
	switch (e) {
		case WeaponSpriteType::Guns: return "gun";
		case WeaponSpriteType::Shotgun: return "shotgun";
		case WeaponSpriteType::SMG: return "smg";
		default: return "unknown";
	}
}

struct IWeapon {
	IWeapon();
	virtual ~IWeapon() = default;

	virtual void DrawCrosshair(glm::vec2 dir, glm::vec2 pos) const = 0;
	virtual auto Shoot(glm::vec2 dir, glm::vec2 pos) -> std::optional<ShootingResult> = 0;
	void Reload();

	[[nodiscard]]
	virtual constexpr const char* path() const = 0;

	virtual void SaveToFile();
	virtual void LoadFromFile();

	std::string name;
	float damage = 1.0f;
	float cooldown = 0.0f;
	float maxForce = 0.0f;
	int maxBulletsPerShot = 1;
	float maxDispersionAngle = 20.0f;
	bool isAutomatic = false;
	ColliderFlags collisionFlag;

	RecoilConfiguration recoil;

	std::string uiIconPath = "UI/images/Gun_UI.png";

	std::string aimingAnimation = "an_cat_aim_guns";
	std::string shootingAnimation = "an_cat_shoot_guns";
	std::string effectSocket = "cat_gun_origin";

	std::string muzzleAnimName = "gun";

	int magazineSize = 1;
	int initialAmmo = 1;

	std::string droppedTexture = "WHITEBOXING/GUN.mat";

	[[nodiscard]]
	auto GetCurrentAmmo() const -> uint8_t;

	[[nodiscard]]
	auto GetAvailableAmmo() const -> uint8_t;

#ifdef TOAST_EDITOR
	virtual void Inspector();
#endif

protected:
	virtual void Save(json_t& j);
	virtual void Load(json_t& j);

	bool CheckIfInCooldown();
	void ResetCooldown();

	[[nodiscard]]
	bool HasAmmo() const;

	std::chrono::steady_clock::time_point lastUsedTime;
	int currentAmmo = 0;
	int availableAmmo = 0;

	int currentWeaponIndex = 0;    ///< Index of the weapon in the player's weapon array, used for saving/loading and HUD display

	glm::vec2 currentForce = {};
};
