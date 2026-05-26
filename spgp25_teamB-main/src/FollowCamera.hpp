/// @file FollowCamera.hpp
/// @author dario
/// @date 02/11/2025.

#pragma once
#include <Toast/Renderer/Camera.hpp>

class FollowCamera : public toast::Camera {
public:
	REGISTER_TYPE(FollowCamera);

	void LateTick() override;

#ifdef TOAST_EDITOR
	void Inspector() override;
#endif

	void SetFollowCameraTarget(Actor* target) {
		m_target = target;
	}

	void SetOffset(const glm::vec3& offset) {
		m_offset = offset;
	}

	// zoom control
	void SetZoomScale(float s) {
		m_targetZoomScale = s;
	}

	float GetZoomScale() const {
		return m_zoomScale;
	}

	void SetCameraBounds(glm::vec2 minB, glm::vec2 maxB) {
		m_minBounds = minB;
		m_maxBounds = maxB;
	}

private:
	Actor* m_target = nullptr;

	glm::vec3 m_offset = glm::vec3(0.f, 5.f, 15.f);

	float m_followSpeed = 2.1f;

	// Zoom data
	float m_zoomScale = 1.0f;                            // current zoom multiplier
	float m_targetZoomScale = 1.0f;                      // where the zoom wants to go
	float m_zoomLerpSpeed = 3.0f;                        // how fast zoom blends

	glm::vec2 m_minBounds = glm::vec2(-50.f, -50.0f);    // min cam bounds per scene
	glm::vec2 m_maxBounds = glm::vec2(50.f, 50.0f);      // min cam bounds per scene

	bool m_drawDebugBounds = false;
};
