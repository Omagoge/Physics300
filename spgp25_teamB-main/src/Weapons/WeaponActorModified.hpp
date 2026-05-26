/// @file WeaponActorSpawned.hpp
/// @author dario
/// @date 19/02/2026.

#pragma once

#include "WeaponActor.hpp"

#include <Toast/Event/Event.hpp>

struct WeaponActorModified : public event::Event<WeaponActorModified> {
	WeaponActor* weapon = nullptr;
	bool created = false;

	WeaponActorModified(WeaponActor* weapon, bool created) : weapon(weapon), created(created) { }
};
