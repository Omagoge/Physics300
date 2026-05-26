/// @file EditorCamera.h
/// @author dario
/// @date 12/10/2025.

#pragma once
// #include <Toast/Input/Action.hpp>
#include "Toast/Input/InputListener.hpp"

#include <Toast/Renderer/Camera.hpp>
#include <Toast/SimulateWorldEvent.hpp>

namespace editor {

class EditorCamera : public toast::Camera {
public:
	REGISTER_TYPE(EditorCamera);

	void Init() override;
	void Begin() override;
	void Tick() override;

	static EditorCamera* GetInstance() {
		return m_instance;
	}

	void SetViewMode(bool mode) {
		m_mode = mode;
	}

	float scrollSpeed = 2.0f;

private:
	static EditorCamera* m_instance;

	input::Listener m_listener;
	// bool MoveCamera(input::Action* a);
	// bool ZoomCamera(input::Action* a);
	// bool RotateCamera(input::Action* a);

	void InitAnglesFromQuat(const glm::quat& q);

	glm::quat MakeOrientation(float yawDeg, float pitchDeg);

	glm::vec3 m_dir = glm::vec3(0.0f);

	bool m_mode = false;    // 0 = 2D, 1 = 3D
	bool m_button = false;

	bool m_dragButton = false;

	glm::vec2 m_lastMousePos = glm::vec2(0.0f, 0.0f);
	glm::vec2 m_mousePos = glm::vec2(0.0f, 0.0f);
	glm::vec2 m_mouseDelta = glm::vec2(0.0f, 0.0f);

	bool s_anglesInitialized = false;
	float s_yawDeg = 0.0f;      // yaw around world +Y
	float s_pitchDeg = 0.0f;    // pitch around camera local +X (right)
};
}
