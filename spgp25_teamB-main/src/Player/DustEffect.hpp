/// @file DustEffect.hpp
/// @author dario
/// @date 10/03/2026.

#pragma once
#include "Toast/Components/SpineRendererComponent.hpp"
#include "Toast/Objects/Actor.hpp"

class DustEffect : public toast::Actor {
public:
	REGISTER_ABSTRACT(DustEffect);

	void Init() override;

	void PlayEffect(glm::vec2 pos, float rot, int type);

private:
	SpineRendererComponent* m_spine = nullptr;
};
