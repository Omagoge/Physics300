/// @file ShootableObject.hpp
/// @author Alexey
/// @brief Object that dies when shot. That is literally it.
/// @date 23 March 2026

#pragma once

#include "Toast/Components/MeshRendererComponent.hpp"
#include "Toast/Objects/Actor.hpp"
#include "Toast/Physics/Rigidbody.hpp"
#include "Weapons/IDamageable.hpp"

class ShootableObject : public toast::Actor,
                        public IDamageable {
public:
	friend class ConditionalDoor;
	REGISTER_TYPE(ShootableObject);

	void Init() override;
	void Begin() override;

#ifdef TOAST_EDITOR
	void Inspector() override;
#endif

	const std::string& GetDoorTag() const {
		return mDoorTag;
	}

	void AllowDeath(const bool value) {
		mDeathAllowed = value;
	}

	void OnDamage(float damage) override {
		mShot = true;
	}

	void OnDeath() override;
	void DeathParticles();
	void ResetOnBegin();

	[[nodiscard]]
	json_t Save() const override;
	void Load(json_t j, bool force_create = true) override;

protected:
	// Components
	physics::Rigidbody* m_rb = nullptr;
	toast::MeshRendererComponent* m_sprite = nullptr;

	// Variables
	std::string mDoorTag;
	float mDeathTime = 0.9f;
	bool mDeathAllowed = false;
	bool mShot = false;
};
