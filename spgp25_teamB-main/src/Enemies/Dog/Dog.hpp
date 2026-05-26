#pragma once
#include "../Pigeon/Pigeon.hpp"

namespace game {
class Dog : public ShootingEnemy {
public:
	REGISTER_TYPE(Dog)

	void Init() override;
	void Begin() override;
	void Tick() override;

	void PlayShootIdleAnim() override;
	void PlayAimAnim() override;
	void PlayFireAnim() override;

	void EndShootIdleAnim() override;
	void EndAimAnim() override;
	void EndFireAnim() override;

	void ResetAnims() override;

	void OnDamage(float damage) override;

	void OnDeath() override { }

	void DoDamage(float damage) override {
		OnDamage(damage);
	}    // The dog is invincible, no need to check anything...

	void ManageGunAnims() override;
	void ManageSpriteDir() override;

	void DefaultShootParams() override {
		const int default_dir = m_shootParams.spriteDirDefault;
		const bool do_damage = m_shootParams.doDamage;
		const bool shoot_player = m_shootParams.shootPlayer;
		const bool shoot_child = m_shootParams.shootChild;
		m_shootParams = {};
		m_shootParams.doDamage = do_damage;
		m_shootParams.shootPlayer = shoot_player;
		m_shootParams.shootChild = shoot_child;
		m_shootParams.spriteDirDefault = default_dir;
		m_shootParams.aimDistance = 7.f;
		m_shootParams.muzzleDistance = 0.125f;
		m_shootParams.shootTime = 0.1f;
		m_shootParams.cooldownTime = 1.f;
		m_shootParams.interpFactor = 4.f;
		m_spine->scale(glm::vec3(.5f, .5f, 1.f));
		m_spine->position(glm::vec3(-0.5f, -2.f, 0));
	}

private:
	float m_damagedTimer = 0.f;
	float m_damagedTime = 0.35f;
	bool m_damaged = false;
};
}
