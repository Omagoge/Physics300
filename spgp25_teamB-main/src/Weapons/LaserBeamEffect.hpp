#pragma once

#include "Toast/Objects/Actor.hpp"
#include "Toast/Renderer/IRenderable.hpp"
#include "Toast/Renderer/Shader.hpp"
#include "Toast/Resources/Mesh.hpp"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class LaserBeamRenderer : public renderer::IRenderable {
public:
	REGISTER_ABSTRACT(LaserBeamRenderer);

	void Init() override;
	void Destroy() override;
	void LoadTextures() override;
	void OnRender(renderer::IRenderablePass pass, const glm::mat4& view_proj) noexcept override;

	void SetBeam(glm::vec2 start, glm::vec2 end, glm::vec4 color, float width, float alpha);

	float glowIntensity = 1.5f;

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
	float m_currentHalfWidth = 0.5f;
};

class LaserBeamEffect : public toast::Actor {
public:
	REGISTER_ABSTRACT(LaserBeamEffect);

	void Init() override;

	void SetBeam(glm::vec2 start, glm::vec2 end) {
		if (!m_renderer) {
			return;
		}
		m_start = start;
		m_end = end;
		_UpdateRenderer();
	}

	void Hide() {
		if (m_renderer) {
			m_renderer->Hide();
		}
	}

	void Show() const {
		if (m_renderer) {
			m_renderer->enabled(true);
		}
	}

	float beamWidth = 0.2f;
	glm::vec4 beamColor = { 1.0f, 0.0f, 0.0f, 1.0f };
	float beamAlpha = 1.0f;

private:
	LaserBeamRenderer* m_renderer = nullptr;
	glm::vec2 m_start = {};
	glm::vec2 m_end = {};

	void _UpdateRenderer() const;
};
