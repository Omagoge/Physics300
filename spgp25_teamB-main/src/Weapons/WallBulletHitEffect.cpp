/// @file WallBulletHitEffect.cpp
/// @author dario
/// @date 27/02/2026.

#include "WallBulletHitEffect.hpp"

#include <Toast/Time.hpp>

void WallBulletHitEffect::Init() {
	m_ParticleSystem = children.AddRequired<toast::ParticleSystem>();

	// Tick must run so the hold-timer works; skip the others.
	SetRunEarlyTick(false);
	SetRunLateTick(false);

	m_ParticleSystem->LoadFromLua("VFX/PARTICLES/WallBulletHitEffect.lua");
	m_ParticleSystem->Stop();
}

void WallBulletHitEffect::Tick() {
	if (m_holdTimer < 0.f) {
		return;
	}

	m_holdTimer -= static_cast<float>(Time::delta());

	if (m_holdTimer <= 0.f) {
		m_holdTimer = -1.f;
		m_ParticleSystem->Stop();
		if (m_onHold) {
			m_onHold();
			m_onHold = nullptr;
		}
	}
}
