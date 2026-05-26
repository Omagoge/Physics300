/// @file EnemyShootWarningEffect.hpp
/// @author Alexey
/// @date 12/03/2026.

#pragma once
#include "Toast/Objects/Actor.hpp"
#include "Toast/Objects/ParticleSystem.hpp"

#include <functional>

class EnemyShootWarningEffect : public toast::Actor {
public:
	REGISTER_ABSTRACT(EnemyShootWarningEffect);

	void Init() override;
	void Tick() override;

	void EnableParticles() const {
		m_ParticleSystem->enabled(true);
	}

	void PlayEffect(glm::vec2* pos, std::function<void()> holdCallback) {
		if (!m_ParticleSystem) {
			TOAST_WARN("EnemyShootWarningEffect::PlayEffect called but particle system not initialised - returning to pool immediately");
			if (holdCallback) {
				holdCallback();
			}
			return;
		}
		transform()->worldPosition(glm::vec3(pos->x, pos->y, 0.1f));
		m_enemyMuzzlePos = pos;

		m_ParticleSystem->Stop();
		m_ParticleSystem->Play();

		m_holdTimer = 0.7f;
		m_onHold = std::move(holdCallback);
	}

	toast::ParticleSystem* m_ParticleSystem = nullptr;

private:
	float m_holdTimer = -1.f;
	std::function<void()> m_onHold;
	glm::vec2* m_enemyMuzzlePos = nullptr;
};
