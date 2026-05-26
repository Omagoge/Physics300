// created by Alexey on 31/03/26
// Door that opens once certain levels are unlocked

#pragma once

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

class HubDoor : public toast::Actor {
public:
	REGISTER_TYPE(HubDoor)

	void Init() override;
	void Begin() override;
	void Tick() override;
	void Destroy() override;
	void CheckUnlock(int world, int level);

#ifdef TOAST_EDITOR
	void Inspector() override;
#endif

	[[nodiscard]]
	json_t Save() const override;
	void Load(json_t j, bool force_create = true) override;

private:
	void Unlock();

	struct {
		int world = 0;
		int level = 0;

		bool unlocked = false;
		float openDistance = 5.f;
		float openSpeed = 3.f;
		float movedSoFar = 0.f;

		int xDir = 1;
		int yDir = 1;
		bool trueYFalseX = true;
	} m;

	physics::Rigidbody* m_rb = nullptr;
	physics::Collider* m_collider = nullptr;
};

}    // namespace game
