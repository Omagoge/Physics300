#include "LaserBeamEffect.hpp"

#include "Toast/Renderer/IRendererBase.hpp"
#include "Toast/Resources/ResourceManager.hpp"

#include <glad/gl.h>

namespace {
constexpr int NUM_SEGMENTS = 24;
constexpr int VERTS_PER_SECTION = 6;
constexpr float Z_OFFSET = 0.0f;
constexpr float CORE_RATIO = 0.14f;
constexpr float GLOW_RATIO = 1.0f;
constexpr float EDGE_RATIO = 1.5f;

float WidthAt(float frac, float baseWidth) {
	const float leftTaper = 1.0f - 0.2f * glm::smoothstep(0.0f, 0.1f, frac);
	const float rightTaper = 1.0f - 0.2f * glm::smoothstep(0.9f, 1.0f, 1.0f - frac);
	return baseWidth * glm::min(leftTaper, rightTaper);
}

float AlphaAt(float frac, float baseAlpha) {
	const float distFromCenter = glm::abs(0.5f - frac) * 2.0f;
	const float a = 1.0f - 0.3f * distFromCenter;
	return a * baseAlpha;
}
}

void LaserBeamRenderer::Init() {
	m_verts.reserve((NUM_SEGMENTS + 1) * VERTS_PER_SECTION);
	m_indices.reserve(NUM_SEGMENTS * (VERTS_PER_SECTION - 1) * 6);
}

void LaserBeamRenderer::LoadTextures() {
	m_shader = resource::LoadResource<renderer::Shader>("SHADERS/laser_beam.shader");
	m_mesh.InitDynamicSpine();
	renderer::IRendererBase::GetInstance()->AddTransparent(this);
}

void LaserBeamRenderer::Destroy() {
	renderer::IRendererBase::GetInstance()->RemoveTransparent(this);
}

void LaserBeamRenderer::SetBeam(glm::vec2 start, glm::vec2 end, glm::vec4 color, float width, float alpha) {
	const float dist = glm::distance(start, end);
	if (dist < 0.001f) {
		m_active = false;
		return;
	}

	const glm::vec2 dir = glm::normalize(end - start);
	const glm::vec2 perp = glm::vec2(-dir.y, dir.x);

	const glm::vec4 core_color = glm::vec4(color.r, color.g, color.b, 1.0f);
	const glm::vec4 glow_color = glm::vec4(color.r, color.g, color.b, 0.35f);
	const glm::vec4 edge_color = glm::vec4(color.r, color.g, color.b, 0.0f);

	m_verts.clear();
	m_indices.clear();

	m_currentHalfWidth = width;

	// construct vertices per-segment
	for (int i = 0; i <= NUM_SEGMENTS; ++i) {
		const float frac = static_cast<float>(i) / static_cast<float>(NUM_SEGMENTS);
		const glm::vec2 pos = start + (end - start) * frac;
		const float w = WidthAt(frac, width);
		const float a = AlphaAt(frac, alpha);

		const float core_w = w * CORE_RATIO;
		const float glow_w = w * GLOW_RATIO;
		const float edge_w = w * EDGE_RATIO;

		const uint32_t col_core = PackABGR(core_color, a);
		const uint32_t col_glow = PackABGR(glow_color, a);
		const uint32_t col_edge = PackABGR(edge_color, 1.0f);

		float safe_w = glm::max(w, 1e-6f);
		const float norm_edge_outer = edge_w / safe_w;
		const float norm_edge_glow = glow_w / safe_w;
		const float norm_edge_core = core_w / safe_w;

		const float off_outer = glm::clamp(norm_edge_outer, 0.001f, 1.0f);
		const float off_glow = glm::clamp(norm_edge_glow, 0.001f, 1.0f);
		const float off_core = glm::clamp(norm_edge_core * 1.2f, 0.005f, 1.0f);

		m_verts.push_back({ glm::vec3(pos - perp * edge_w, Z_OFFSET), glm::vec2(-off_outer, frac), col_edge });
		m_verts.push_back({ glm::vec3(pos - perp * glow_w, Z_OFFSET), glm::vec2(-off_glow, frac), col_glow });
		m_verts.push_back({ glm::vec3(pos - perp * core_w, Z_OFFSET), glm::vec2(-off_core, frac), col_core });
		m_verts.push_back({ glm::vec3(pos + perp * core_w, Z_OFFSET), glm::vec2(off_core, frac), col_core });
		m_verts.push_back({ glm::vec3(pos + perp * glow_w, Z_OFFSET), glm::vec2(off_glow, frac), col_glow });
		m_verts.push_back({ glm::vec3(pos + perp * edge_w, Z_OFFSET), glm::vec2(off_outer, frac), col_edge });
	}

	// Build index buffer
	for (int i = 0; i < NUM_SEGMENTS; ++i) {
		const uint16_t base_curr = static_cast<uint16_t>(i * VERTS_PER_SECTION);
		const uint16_t base_next = static_cast<uint16_t>((i + 1) * VERTS_PER_SECTION);

		for (int j = 0; j < VERTS_PER_SECTION - 1; ++j) {
			const uint16_t tl = base_curr + static_cast<uint16_t>(j);
			const uint16_t tr = base_curr + static_cast<uint16_t>(j + 1);
			const uint16_t bl = base_next + static_cast<uint16_t>(j);
			const uint16_t br = base_next + static_cast<uint16_t>(j + 1);

			m_indices.push_back(tl);
			m_indices.push_back(bl);
			m_indices.push_back(br);

			m_indices.push_back(tl);
			m_indices.push_back(br);
			m_indices.push_back(tr);
		}
	}

	m_mesh.UpdateDynamicSpine(m_verts.data(), m_verts.size(), m_indices.data(), m_indices.size());
	m_active = true;
}

void LaserBeamRenderer::OnRender(renderer::IRenderablePass pass, const glm::mat4& view_proj) noexcept {
	if (!m_active || !m_shader || m_indices.empty()) {
		return;
	}

	if (pass != renderer::IRenderablePass::GEOMETRY) {
		return;
	}

	m_shader->Use();
	m_shader->Set("u_ViewProj", view_proj);
	m_shader->Set("u_GlowIntensity", glowIntensity);

	GLboolean prevDepthMask = GL_TRUE;
	glGetBooleanv(GL_DEPTH_WRITEMASK, &prevDepthMask);
	GLboolean wasBlendEnabled = glIsEnabled(GL_BLEND);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);

	m_mesh.DrawDynamicSpine(m_indices.size());

	glDepthMask(prevDepthMask == GL_TRUE ? GL_TRUE : GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (!wasBlendEnabled) {
		glDisable(GL_BLEND);
	}
}

void LaserBeamEffect::Init() {
	Actor::Init();
	SetRunEarlyTick(false);
	SetRunLateTick(false);

	m_renderer = children.AddRequired<LaserBeamRenderer>();
}

void LaserBeamEffect::_UpdateRenderer() const {
	m_renderer->SetBeam(m_start, m_end, beamColor, beamWidth, beamAlpha);
}
