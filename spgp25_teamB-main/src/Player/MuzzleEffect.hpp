/// @file MuzzleEffect.hpp
/// @author dario
/// @date 23/02/2026.

#pragma once
#include "Toast/Components/SpineRendererComponent.hpp"
#include "Toast/Objects/Actor.hpp"
#include "Toast/Objects/ParticleSystem.hpp"

class MuzzleEffect : public toast::Actor {
public:
	REGISTER_ABSTRACT(MuzzleEffect);

	void Init() override;

	void PlayEffect(glm::vec2 pos, float rot, std::string_view anim_name, int number = 3) const;

private:
	SpineRendererComponent* m_spineRenderer = nullptr;
	toast::ParticleSystem* m_particleSystem = nullptr;
};
