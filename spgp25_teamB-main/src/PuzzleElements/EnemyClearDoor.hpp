// created by Akaansh on 21/03/26
// Door that slides open when all enemies with a matching doorTag are ded :)

#pragma once

#include "Toast/Renderer/HUD/HUDActor.hpp"
#include "Weapons/IDamageable.hpp"

#include <Toast/Objects/Actor.hpp>
#include <Toast/Physics/Collider.hpp>
#include <Toast/Physics/Rigidbody.hpp>
#include <string>
#include <vector>

namespace physics {
class Collider;
}

namespace game {

class EnemyClearDoor : public toast::Actor {
public:
	REGISTER_TYPE(EnemyClearDoor)

	void Init() override;
	void Begin() override;
	void Tick() override;
	void Destroy() override;

#ifdef TOAST_EDITOR
	void Inspector() override;
#endif

	[[nodiscard]]
	json_t Save() const override;
	void Load(json_t j, bool force_create = true) override;

private:
	void Unlock();
	bool AllEnemiesDead() const;
	int GetAliveEnemiesCount() const;

	std::string m_doorTag = "";             // MUSTT match m_doorTag on ShootingEnemy
	std::vector<IDamageable*> m_enemies;    // populated in Begin(), so dont need to serialize

	bool m_unlocked = false;
	bool m_trueEnemyFalseObject = true;
	float m_openDistance = 5.f;
	float m_openSpeed = 3.f;
	float m_movedSoFar = 0.f;

	int m_xDir = 1;
	int m_yDir = 1;
	bool m_trueYFalseX = true;

	physics::Rigidbody* m_rb = nullptr;
	physics::Collider* m_collider = nullptr;

	HUDActor* hud = nullptr;
};

}    // namespace game
