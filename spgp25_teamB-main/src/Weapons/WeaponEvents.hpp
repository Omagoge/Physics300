/// @file WeaponEvents.hpp
/// @author Xein
/// @date 19 Feb 2026

#pragma once
#include <Toast/Event/Event.hpp>

struct IWeapon;

struct WeaponShoot : event::Event<WeaponShoot> {
	WeaponShoot(IWeapon* w) : weapon(w) {
		currentAmmo = w->GetCurrentAmmo();
		magazineSize = w->magazineSize;
		bulletsRemaining = w->GetAvailableAmmo();
	}

	const IWeapon* weapon;
	uint8_t currentAmmo;
	uint8_t magazineSize;
	uint8_t bulletsRemaining;
};

struct WeaponReload : event::Event<WeaponReload> {
	WeaponReload(IWeapon* w) : weapon(w) {
		currentAmmo = w->GetCurrentAmmo();
		magazineSize = w->magazineSize;
		bulletsRemaining = w->GetAvailableAmmo();
	}

	const IWeapon* weapon;
	uint8_t currentAmmo;
	uint8_t magazineSize;
	uint8_t bulletsRemaining;
};

struct WeaponThrow : event::Event<WeaponThrow> {
	WeaponThrow(IWeapon* w) : weapon(w) { }

	const IWeapon* weapon;
};

struct WeaponGrab : event::Event<WeaponGrab> {
	WeaponGrab(IWeapon* w) : weapon(w) {
		currentAmmo = w->GetCurrentAmmo();
		magazineSize = w->magazineSize;
		bulletsRemaining = w->GetAvailableAmmo();
	}

	const IWeapon* weapon;
	uint8_t currentAmmo;
	uint8_t magazineSize;
	uint8_t bulletsRemaining;
};
