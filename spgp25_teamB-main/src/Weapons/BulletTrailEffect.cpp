#include "BulletTrailEffect.hpp"

#include "Toast/Renderer/IRendererBase.hpp"
#include "Toast/Resources/ResourceManager.hpp"

#include <Toast/Time.hpp>
#include <glad/gl.h>

// ---- BulletTrailRenderer ---------------------------------------------------

void BulletTrailRenderer::Init() {
	m_verts.reserve(128);
	m_indices.reserve(256);
}

void BulletTrailRenderer::LoadTextures() {
	m_shader = resource::LoadResource<renderer::Shader>("SHADERS/bullet_trail.shader");
	m_mesh.InitDynamicSpine();
	renderer::IRendererBase::GetInstance()->AddTransparent(this);
}

void BulletTrailRenderer::Destroy() {
	renderer::IRendererBase::GetInstance()->RemoveTransparent(this);
}

void BulletTrailRenderer::SetTrail(glm::vec2 start, glm::vec2 end, float t, float half_width, glm::vec4 color, float alpha) {
	const glm::vec2 head = start + (end - start) * t;
	const float total_len = glm::distance(start, head);
	if (total_len < 0.001f) {
		m_active = false;
		return;
	}

	const glm::vec2 dir = glm::normalize(head - start);
	const glm::vec2 perp = glm::vec2(-dir.y, dir.x);

	constexpr int NUM_SEGMENTS = 12;
	constexpr float CORE_RATIO = 0.35f;
	constexpr float GLOW_RATIO = 1.0f;
	constexpr float OUTER_RATIO = 2.0f;
	constexpr float TIP_LENGTH = 0.06f;
	constexpr float MUZZLE_TAPER = 0.15f;
	constexpr float Z_CORE = 0.15f;
	constexpr float Z_GLOW = 0.14f;

	const glm::vec4 core_col = glm::vec4(glm::min(color.r * 1.4f, 1.f), glm::min(color.g * 1.3f, 1.f), glm::min(color.b * 0.7f, 1.f), 1.f);
	const glm::vec4 glow_col = glm::vec4(color.r * 0.8f, color.g * 0.7f, color.b * 0.4f, 0.6f);
	const glm::vec4 outer_col = glm::vec4(color.r * 0.5f, color.g * 0.4f, color.b * 0.2f, 0.0f);

	m_verts.clear();
	m_indices.clear();

	auto widthAt = [&](float frac) -> float {
		const float ramp = glm::smoothstep(0.0f, 0.3f, frac);
		const float taper = 1.0f - 0.15f * glm::smoothstep(0.7f, 1.0f, frac);
		const float muzzle = glm::mix(MUZZLE_TAPER, 1.0f, ramp);
		return muzzle * taper;
	};

	auto alphaAt = [&](float frac) -> float {
		return glm::mix(0.3f, 1.0f, glm::smoothstep(0.0f, 0.5f, frac));
	};

	constexpr int VERTS_PER_SECTION = 6;

	for (int i = 0; i <= NUM_SEGMENTS; ++i) {
		const float frac = static_cast<float>(i) / static_cast<float>(NUM_SEGMENTS);
		const glm::vec2 pos = start + (head - start) * frac;
		const float w = widthAt(frac) * half_width;
		const float a = alphaAt(frac) * alpha;
		const float core_w = w * CORE_RATIO;
		const float glow_w = w * GLOW_RATIO;
		const float outer_w = w * OUTER_RATIO;

		const uint32_t col_core = PackABGR(core_col, a);
		const uint32_t col_glow = PackABGR(glow_col, a);
		const uint32_t col_outer = PackABGR(outer_col, 1.f);

		m_verts.push_back({ glm::vec3(pos - perp * outer_w, Z_GLOW), {}, col_outer });
		m_verts.push_back({ glm::vec3(pos - perp * glow_w, Z_GLOW), {}, col_glow });
		m_verts.push_back({ glm::vec3(pos - perp * core_w, Z_CORE), {}, col_core });
		m_verts.push_back({ glm::vec3(pos + perp * core_w, Z_CORE), {}, col_core });
		m_verts.push_back({ glm::vec3(pos + perp * glow_w, Z_GLOW), {}, col_glow });
		m_verts.push_back({ glm::vec3(pos + perp * outer_w, Z_GLOW), {}, col_outer });
	}

	const glm::vec2 tip_pos = head + dir * (total_len * TIP_LENGTH);
	const uint32_t col_tip = PackABGR(glm::vec4(core_col.r, core_col.g, core_col.b, 0.f), 1.f);
	const auto tip_idx = static_cast<uint16_t>(m_verts.size());
	m_verts.push_back({ glm::vec3(tip_pos, Z_CORE), {}, col_tip });

	for (int i = 0; i < NUM_SEGMENTS; ++i) {
		const auto base_curr = static_cast<uint16_t>(i * VERTS_PER_SECTION);
		const auto base_next = static_cast<uint16_t>((i + 1) * VERTS_PER_SECTION);

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

	const auto last_base = static_cast<uint16_t>(NUM_SEGMENTS * VERTS_PER_SECTION);
	for (int j = 0; j < VERTS_PER_SECTION - 1; ++j) {
		m_indices.push_back(last_base + static_cast<uint16_t>(j));
		m_indices.push_back(tip_idx);
		m_indices.push_back(last_base + static_cast<uint16_t>(j + 1));
	}

	m_mesh.UpdateDynamicSpine(m_verts.data(), m_verts.size(), m_indices.data(), m_indices.size());
	m_active = true;
}

void BulletTrailRenderer::OnRender(renderer::IRenderablePass pass, const glm::mat4& view_proj) noexcept {
	if (!m_active || !m_shader || m_indices.empty()) {
		return;
	}
	if (pass == renderer::IRenderablePass::GEOMETRY) {
		m_shader->Use();
		m_shader->Set("u_ViewProj", view_proj);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_TRUE);

		m_mesh.DrawDynamicSpine(m_indices.size());

		glDepthMask(GL_TRUE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}

// ---- BulletTrailEffect -----------------------------------------------------

void BulletTrailEffect::Init() {
	Actor::Init();
	SetRunEarlyTick(false);
	SetRunLateTick(false);

	m_renderer = children.AddRequired<BulletTrailRenderer>();
}

void BulletTrailEffect::Tick() {
	if (m_phase == Phase::Idle) {
		return;
	}

	const float dt = static_cast<float>(Time::delta());
	m_elapsed += dt;

	if (m_phase == Phase::Travelling) {
		const float raw = glm::clamp(m_elapsed / m_travelDuration, 0.f, 1.f);
		const float t = 1.f - ((1.f - raw) * (1.f - raw));    // ease-out quad
		_UpdateRenderer(t, 1.f);

		if (m_elapsed >= m_travelDuration) {
			_UpdateRenderer(1.f, 1.f);
			m_elapsed = 0.f;
			m_phase = Phase::Fading;
		}
	} else if (m_phase == Phase::Fading) {
		const float fade = 1.f - (m_elapsed / FADE_DURATION);
		_UpdateRenderer(1.f, fade * fade);    // quadratic fade

		if (m_elapsed >= FADE_DURATION) {
			m_phase = Phase::Idle;
			m_renderer->Hide();
			if (m_onHold) {
				m_onHold();
				m_onHold = nullptr;
			}
		}
	}
}

void BulletTrailEffect::_UpdateRenderer(float t, float alpha) const {
	m_renderer->SetTrail(m_start, m_end, t, trailHalfWidth, trailColor, alpha);
}
