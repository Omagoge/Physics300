/// @file EnemyShootWarningEffect.cpp
/// @author Alexey
/// @date 12/03/2026.

#include "EnemyShootWarningEffect.hpp"

#include <Toast/Time.hpp>

void EnemyShootWarningEffect::Init() {
	m_ParticleSystem = children.AddRequired<toast::ParticleSystem>();

	// Tick must run so the hold-timer works; skip the others.
	SetRunEarlyTick(false);
	SetRunLateTick(false);

	m_ParticleSystem->LoadFromLua("VFX/PARTICLES/EnemyShootWarning.lua");
	m_ParticleSystem->Stop();
}

void EnemyShootWarningEffect::Tick() {
	if (m_holdTimer < 0.f) {
		return;
	}

	transform()->worldPosition(glm::vec3(m_enemyMuzzlePos->x, m_enemyMuzzlePos->y, 0.1f));

	m_holdTimer -= static_cast<float>(Time::delta());

	if (m_holdTimer <= 0.f) {
		m_holdTimer = -1.f;
		m_ParticleSystem->Pause();
		if (m_onHold) {
			m_onHold();
			m_onHold = nullptr;
		}
	}
}
