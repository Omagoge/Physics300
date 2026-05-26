/// @file BulletTrailEffect.hpp
/// @brief Poolable bullet tracer.

#pragma once

#include "Toast/Objects/Actor.hpp"
#include "Toast/Renderer/IRenderable.hpp"
#include "Toast/Renderer/Shader.hpp"
#include "Toast/Resources/Mesh.hpp"

#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

class BulletTrailRenderer : public renderer::IRenderable {
public:
	REGISTER_ABSTRACT(BulletTrailRenderer);

	void Init() override;
	void Destroy() override;
	void LoadTextures() override;
	void OnRender(renderer::IRenderablePass pass, const glm::mat4& view_proj) noexcept override;

	void SetTrail(glm::vec2 start, glm::vec2 end, float t, float half_width, glm::vec4 color, float alpha);

	void Hide() {
		m_active = false;
	}

private:
	static uint32_t PackABGR(const glm::vec4& c, float alpha) {
		auto r = static_cast<uint8_t>(glm::clamp(c.r, 0.f, 1.f) * 255.f);
		auto g = static_cast<uint8_t>(glm::clamp(c.g, 0.f, 1.f) * 255.f);
		auto b = static_cast<uint8_t>(glm::clamp(c.b, 0.f, 1.f) * 255.f);
		auto a = static_cast<uint8_t>(glm::clamp(c.a * alpha, 0.f, 1.f) * 255.f);
		return (static_cast<uint32_t>(a) << 24) | (static_cast<uint32_t>(b) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(r);
	}

	renderer::Mesh m_mesh;
	std::shared_ptr<renderer::Shader> m_shader;
	std::vector<renderer::SpineVertex> m_verts;
	std::vector<uint16_t> m_indices;
	bool m_active = false;
};

class BulletTrailEffect : public toast::Actor {
public:
	REGISTER_ABSTRACT(BulletTrailEffect);

	void Init() override;
	void Tick() override;

	void EnableRenderer() const {
		m_renderer->enabled(true);
	}

	/// @brief Activate this trail.
	/// @param start        Muzzle world position.
	/// @param end          Target world position.
	/// @param holdCallback Called when the trail finishes to return this to its pool.
	void PlayEffect(glm::vec2 start, glm::vec2 end, std::function<void()> holdCallback) {
		// Defensive: ensure renderer was created by Init/Begin
		if (!m_renderer) {
			TOAST_WARN("BulletTrailEffect::PlayEffect called but renderer not initialised - returning to pool immediately");
			if (holdCallback) {
				holdCallback();
			}
			return;
		}
		m_start = start;
		m_end = end;
		_UpdateRenderer(0.f, 1.f);

		const float dist = glm::distance(start, end);
		m_travelDuration = dist / TRAIL_SPEED;
		m_elapsed = 0.f;
		m_phase = Phase::Travelling;

		transform()->position(glm::vec3(start.x, start.y, 1.f));
		m_onHold = std::move(holdCallback);
	}

	float trailHalfWidth = 0.05f;
	glm::vec4 trailColor = { 1.0f, 0.95f, 0.5f, 1.0f };

private:
	static constexpr float TRAIL_SPEED = 500.f;
	static constexpr float FADE_DURATION = 0.02f;

	enum class Phase : uint8_t {
		Idle,
		Travelling,
		Fading
	};

	void _UpdateRenderer(float t, float alpha) const;

	BulletTrailRenderer* m_renderer = nullptr;
	glm::vec2 m_start = {};
	glm::vec2 m_end = {};
	float m_elapsed = 0.f;
	float m_travelDuration = 0.f;
	Phase m_phase = Phase::Idle;
	std::function<void()> m_onHold;
};
