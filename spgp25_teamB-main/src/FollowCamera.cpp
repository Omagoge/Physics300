/// @file FollowCamera.cpp
/// @author dario
/// @date 02/11/2025.

#include "FollowCamera.hpp"

#include "Toast/Renderer/DebugDrawLayer.hpp"

#include <Toast/Time.hpp>

#ifdef TOAST_EDITOR
#include "imgui.h"
#endif

void FollowCamera::LateTick() {
	Camera::LateTick();

	float dt = Time::delta();
	float alpha = 1.0f - std::exp(-m_zoomLerpSpeed * dt);
	m_zoomScale = glm::mix(m_zoomScale, m_targetZoomScale, alpha);

	if (!m_target) {
		return;
	}

	// apply zoom to existing offset
	glm::vec3 zoomedOffset = m_offset * m_zoomScale;

	auto targetPos = m_target->transform()->worldPosition() + zoomedOffset;
	targetPos.x = glm::clamp(targetPos.x, m_minBounds.x, m_maxBounds.x);    // X Cam Limits in Scene
	targetPos.y = glm::clamp(targetPos.y, m_minBounds.y, m_maxBounds.y);    // Y Cam Limits in Scene
	auto currentPos = transform()->worldPosition();

	// framerate-independent smoothing
	float followAlpha = 1.0f - std::exp(-m_followSpeed * dt);
	auto newPos = glm::mix(currentPos, targetPos, followAlpha);
	transform()->worldPosition(newPos);

	//@TODO: maybe get the velocity and change fov based on that?
	//@TODO: FOV camera controls lmao
}

#ifdef TOAST_EDITOR
void FollowCamera::Inspector() {
	Camera::Inspector();
	ImGui::SliderFloat("Follow Speed", &m_followSpeed, 0.0f, 10.0f);
	ImGui::SliderFloat3("Offset", &m_offset.x, -100.0f, 100.0f);
	ImGui::SliderFloat("Target Zoom Scale", &m_targetZoomScale, 0.1f, 5.0f);
	ImGui::SliderFloat("Zoom Lerp Speed", &m_zoomLerpSpeed, 0.1f, 10.0f);
	ImGui::SeparatorText("Camera Bounds");
	ImGui::DragFloat2("Minimum Scene Bounds", &m_minBounds.x, -1000.0f, 1000.0f);
	ImGui::DragFloat2("Maximum Scene Bounds", &m_maxBounds.x, -1000.0f, 1000.0f);
	ImGui::Checkbox("Draw Debug", &m_drawDebugBounds);

	if (!m_drawDebugBounds) {
		return;
	}

	// safetyyy
	glm::vec2 minB = glm::min(m_minBounds, m_maxBounds);
	glm::vec2 maxB = glm::max(m_minBounds, m_maxBounds);

	glm::vec2 bl(minB.x, minB.y);
	glm::vec2 br(maxB.x, minB.y);
	glm::vec2 tr(maxB.x, maxB.y);
	glm::vec2 tl(minB.x, maxB.y);

	// Drawing Cam Bounds while in Inspector
	renderer::DebugLine(bl, br, { 0.0f, 1.0f, 0.0f, 1.0f });
	renderer::DebugLine(br, tr, { 0.0f, 1.0f, 0.0f, 1.0f });
	renderer::DebugLine(tr, tl, { 0.0f, 1.0f, 0.0f, 1.0f });
	renderer::DebugLine(tl, bl, { 0.0f, 1.0f, 0.0f, 1.0f });
}
#endif
