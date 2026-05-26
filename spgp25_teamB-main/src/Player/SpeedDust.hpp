/// @file SpeedDust.hpp
/// @author dario
/// @date 28/03/2026.

#pragma once
#include "Toast/Objects/Actor.hpp"
#include "Toast/Objects/ParticleSystem.hpp"

// VFX/PARTICLES/DustFeetSpeed.lua

class SpeedDust : public toast::Actor {
	REGISTER_ABSTRACT(SpeedDust);
	void Init() override;

	toast::ParticleSystem* m_particleSystem = nullptr;

public:
	toast::ParticleSystem* GetParticleSystem() {
		return m_particleSystem;
	}
};
