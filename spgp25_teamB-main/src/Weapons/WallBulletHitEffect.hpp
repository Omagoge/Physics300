/// @file WallBulletHitEffect.hpp
/// @author dario
/// @date 27/02/2026.

#pragma once
#include "Toast/Objects/Actor.hpp"
#include "Toast/Objects/ParticleSystem.hpp"

#include <functional>

class WallBulletHitEffect : public toast::Actor {
public:
	REGISTER_ABSTRACT(WallBulletHitEffect);

	void Init() override;
	void Tick() override;

	void EnableParticles() const {
		m_ParticleSystem->enabled(true);
	}

	void PlayEffect(glm::vec2 pos, glm::vec2 normal, std::function<void()> holdCallback) {
		if (!m_ParticleSystem) {
			TOAST_WARN("WallBulletHitEffect::PlayEffect called but particle system not initialised - returning to pool immediately");
			if (holdCallback) {
				holdCallback();
			}
			return;
		}
		transform()->worldPosition(glm::vec3(pos.x, pos.y, 0.1f));
		const float rot = atan2f(normal.y, normal.x) - (glm::pi<float>() / 2.f);
		transform()->rotationRadians(glm::vec3(0.0f, 0.0f, rot));

		m_ParticleSystem->Stop();
		m_ParticleSystem->Play();

		m_holdTimer = 0.5f;
		m_onHold = std::move(holdCallback);
	}

	toast::ParticleSystem* m_ParticleSystem = nullptr;

private:
	float m_holdTimer = -1.f;
	std::function<void()> m_onHold;
};
