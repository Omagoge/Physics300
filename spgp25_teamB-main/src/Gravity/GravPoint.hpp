/**
 * @file GravPoint.hpp
 * @author Xein
 * @date 22/02/26
 *
 * @brief An object the player can be attracted to
 */

#pragma once
#include <Toast/Components/MeshRendererComponent.hpp>
#include <Toast/Objects/Actor.hpp>

class GravPoint : public toast::Actor {
public:
	REGISTER_TYPE(GravPoint);
	void Init() override;

	void Using();
	void Disable();
	void Enable();

private:
	toast::MeshRendererComponent* mesh = nullptr;
};
