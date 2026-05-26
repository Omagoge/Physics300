/// @file WeaponActor.hpp
/// @author dario
/// @date 19/02/2026.

#pragma once
#include "Toast/Components/MeshRendererComponent.hpp"
#include "Toast/Objects/Actor.hpp"
#include "Toast/Physics/Rigidbody.hpp"

struct IWeapon;

class WeaponActor : public toast::Actor {
public:
	REGISTER_TYPE(WeaponActor);

	void Init() override;
	void Begin() override;
	void Tick() override;
	void OnEnable() override;
	void Destroy() override;

	IWeapon* weaponData = nullptr;

	void Load(json_t j, bool force_create) override;
	json_t Save() const override;

	void ThrowWeapon(glm::vec2 pos, glm::vec2 force);

	void GrabWeapon();
#ifdef TOAST_EDITOR
	void Inspector() override;
#endif

	void ChangeWeaponType(int type);

	toast::Scene* currentScene = nullptr;
	toast::MeshRendererComponent* MeshComponent = nullptr;

private:
	void ChooseWeaponType(unsigned w);

	int weaponType = 0;

	bool showInspector = true;

	physics::Rigidbody* Rigidbody = nullptr;
};
