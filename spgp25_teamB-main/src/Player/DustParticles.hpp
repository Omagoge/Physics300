/// @file DustParticles.hpp
/// @author dario
/// @date 17/03/2026.

#pragma once
#include "Toast/Objects/ParticleSystem.hpp"

class DustParticles : public toast::ParticleSystem {
public:
	REGISTER_ABSTRACT(DustParticles);

	void Init() override {
		SetSerialize(false);
		LoadFromLua("VFX/PARTICLES/DustFeetBraking.lua");
		Stop();
	}

	void EnableEffect(bool enable) {
		if (!enable) {
			this->Pause();
		} else {
			this->Play();
		}
	}
};
