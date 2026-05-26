/// @file DefaultGun.hpp
/// @author Xein
/// @date 18 Feb 2026

#pragma once
#include "IDamageable.hpp"
#include "IWeapon.hpp"
#include "WeaponEvents.hpp"

#include <Toast/Renderer/DebugDrawLayer.hpp>

struct DefaultGun : IWeapon {
	using IWeapon::IWeapon;

	[[nodiscard]]
	constexpr const char* path() const override {
		return "WEAPONS/DefaultGun.json";
	}

	auto Shoot(glm::vec2 dir, glm::vec2 pos) -> std::optional<ShootingResult> override {
		if (not HasAmmo()) {
			return std::nullopt;
		}
		if (not CheckIfInCooldown()) {
			return std::nullopt;
		}

		glm::vec2 knockback_direction = glm::normalize(-dir);
		glm::vec2 knockback_force = knockback_direction * currentForce;
		currentAmmo -= 1;
		currentForce *= recoil.forceFalloffVec;

		event::Send(new WeaponShoot(this));

		auto hit = physics::RayCast(pos, dir, collisionFlag | ColliderFlags::Ground);
		std::vector<physics::RayResult> collisions;
		std::vector<glm::vec2> missedDirs;
		float distance = std::numeric_limits<float>().max();
		if (hit.has_value()) {
			collisions.push_back(*hit);
			distance = hit->distance;

			// damage check
			if (hit->other) {
				if (auto* damageable = dynamic_cast<IDamageable*>(hit->other)) {
					damageable->DoDamage(damage);
				}
			}
		} else {
			missedDirs.push_back(dir);
		}

		return ShootingResult { .distance = distance, .knockbackForce = knockback_force, .collisions = collisions, .missedDirections = missedDirs };
	}

	void DrawCrosshair(glm::vec2 dir, glm::vec2 pos) const override {
		// debug crosshair
		renderer::DebugLine(pos, pos + (dir * 10.0f), { 1, 0, 1, 1 });
	}
};
