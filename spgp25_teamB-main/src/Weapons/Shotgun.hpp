/// @file Shotgun.hpp
/// @author dario
/// @date 25/02/2026.

#pragma once
#include "IWeapon.hpp"
#include "Toast/Renderer/DebugDrawLayer.hpp"
#include "WeaponEvents.hpp"
#include "glm/glm.hpp"

struct Shotgun : IWeapon {
	using IWeapon::IWeapon;

	[[nodiscard]]
	constexpr const char* path() const override {
		return "WEAPONS/Shotgun.json";
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

		// TODO: PLEASE TWEAK ANGLE
		std::uniform_real_distribution dist(-maxDispersionAngle, maxDispersionAngle);

		std::vector<physics::RayResult> collisions;
		std::vector<glm::vec2> missedDirs;

		float dis = 0;
		for (int i = 0; i < maxBulletsPerShot; ++i) {
			float randomAngle = glm::radians(dist(gen));

			glm::vec2 changedDir;
			changedDir.x = dir.x * cos(randomAngle) - dir.y * sin(randomAngle);
			changedDir.y = dir.x * sin(randomAngle) + dir.y * cos(randomAngle);

			changedDir = glm::normalize(changedDir);

			auto hit = physics::RayCast(pos, changedDir, collisionFlag | ColliderFlags::Ground);
			if (hit.has_value()) {
				collisions.push_back(*hit);
				dis = hit->distance;

				// damage check
				if (auto* damageable = dynamic_cast<IDamageable*>(hit->other)) {
					damageable->DoDamage(damage);
				}
			} else {
				missedDirs.push_back(changedDir);
			}
		}

		return ShootingResult { .distance = dis, .knockbackForce = knockback_force, .collisions = collisions, .missedDirections = missedDirs };
	}

	void DrawCrosshair(glm::vec2 dir, glm::vec2 pos) const override {
		// debug crosshair
		renderer::DebugLine(pos, pos + (dir * 10.0f), { 0, 1, 1, 1 });
	}

	std::mt19937 gen;
};
