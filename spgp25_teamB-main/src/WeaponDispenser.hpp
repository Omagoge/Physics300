// Created by Akaansh on 23/03/2026.
// did uno that Japan is generating electricity from weapon dispensers!!!

#pragma once

#include "Weapons/WeaponActor.hpp"

#include <Toast/Objects/Actor.hpp>

class WeaponDispenser : public toast::Actor {
public:
	REGISTER_TYPE(WeaponDispenser)

protected:
	void Init() override;
	void Begin() override;
	void Tick() override;
	void Destroy() override;

public:
	[[nodiscard]]
	json_t Save() const override;
	void Load(json_t j, bool force_create) override;

#ifdef TOAST_EDITOR
	void Inspector() override;
#endif

private:
	void SpawnWeapon();

	// 1 = shotgun, 2 = smg — matches WeaponActor::ChooseWeaponType()
	// keeping it int so its easy to add new types later
	int m_weaponType = 1;

	float m_respawnDelay = 3.f;
	float m_respawnTimer = 0.f;
	bool m_waitingToSpawn = false;
	bool m_needsCleanup = false;

	WeaponActor* m_spawnedWeapon = nullptr;
};
