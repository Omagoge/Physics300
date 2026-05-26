/// @file SpeedDust.cpp
/// @author dario
/// @date 28/03/2026.

#include "SpeedDust.hpp"

void SpeedDust::Init() {
	Actor::Init();

	SetSerialize(false);

	m_particleSystem = children.AddRequired<toast::ParticleSystem>();
	m_particleSystem->LoadFromLua("VFX/PARTICLES/DustFeetSpeed.lua");
	m_particleSystem->Stop();
}
