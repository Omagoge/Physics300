/// @file PigeonGun.hpp
/// @author Alexey
/// @date 24 Feb 2026

#pragma once
#include "IDamageable.hpp"
#include "IWeapon.hpp"
#include "WeaponEvents.hpp"

#include <Toast/Renderer/DebugDrawLayer.hpp>

struct RatGun : IWeapon {
	using IWeapon::IWeapon;

	[[nodiscard]]
	constexpr const char* path() const override {
		return "WEAPONS/RatGun.json";
	}

	auto Shoot(glm::vec2 dir, glm::vec2 pos) -> std::optional<ShootingResult> override {
		if (not CheckIfInCooldown()) {
			return std::nullopt;
		}

		currentForce = { 0.f, 0.f };
		event::Send(new WeaponShoot(this));

		currentForce = { 0.f, 0.f };
		event::Send(new WeaponShoot(this));

		auto hit = physics::RayCast(pos, dir, collisionFlag | ColliderFlags::Ground);
		std::vector<physics::RayResult> collisions;
		float distance = 0;
		if (hit.has_value()) {
			collisions.push_back(*hit);
			distance = hit->distance;

			// damage check
			if (hit->other) {
				if (auto* damageable = dynamic_cast<IDamageable*>(hit->other)) {
					damageable->DoDamage(damage);
				}
			}
		}

		return ShootingResult { .distance = distance, .knockbackForce = currentForce, .collisions = collisions };
	}

	void DrawCrosshair(glm::vec2 dir, glm::vec2 pos) const override {
		// debug crosshair
		renderer::DebugLine(pos, pos + (dir * 10.0f), { 1, 0, 1, 1 });
	}
};
