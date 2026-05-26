/// @file IDamageable.hpp
/// @author dario
/// @date 27/02/2026.

#pragma once

struct IDamageable {
	virtual void DoDamage(float damage) {
		if (mHealth <= 0 || mInvincible) {
			return;
		}
		mHealth -= damage;
		OnDamage(damage);
		CheckDeath();
	}

	float mHealth = 3.0f;

	bool mInvincible = false;

private:
	virtual void OnDeath() = 0;

	virtual void OnDamage(float damage) = 0;

	void CheckDeath() {
		if (mHealth <= 0) {
			mHealth = 0;
			OnDeath();
		}
	}
};
