/// @file EditorCamera.cpp
/// @author dario
/// @date 14/10/2025.

#include "EditorCamera.hpp"

#include "App.hpp"
#include "Toast/Log.hpp"
#include "imgui.h"

#include <SDL3/SDL_mouse.h>

#include <Toast/Input/Action.hpp>
#include <Toast/Input/Bind.hpp>
#include <Toast/Time.hpp>
#include <Toast/Window/Window.hpp>
#include <Toast/Window/WindowEvents.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <Toast/SimulateWorldEvent.hpp>
#include <glm/gtx/quaternion.hpp>

namespace editor {

EditorCamera* EditorCamera::m_instance = nullptr;

constexpr float kPitchLimit = 89.9f;    // clamp to avoid singularity
constexpr float kMouseSensitivity = 0.15f;

void EditorCamera::Init() {
	Camera::Init();
	if (m_instance == nullptr) {
		m_instance = this;
	}
	// Initialize from current transform once; keep across RMB presses
	InitAnglesFromQuat(transform()->rotationQuat());

	listener()->Subscribe<event::WindowMouseScroll>(
	    [this](const event::WindowMouseScroll* e) -> bool {
		    if (!ToastEditor::viewport_window().isFocused) {
			    return false;
		    }
		    if (ToastEditor::isSimulate) {
			    return false;
		    }
		    if (e->y > 0) {
			    if (!m_mode) {    // 2D mode
				    // transform()->position(transform()->position() + glm::vec3(0.0f, 0.0f, -.5f));
				    scrollSpeed -= 0.2f;
				    scrollSpeed = std::clamp(scrollSpeed, 0.2f, 4.0f);
			    } else {    // 3D mode
				    transform()->position(transform()->position() + transform()->GetFrontVector() * .5f);
				    scrollSpeed -= 0.2f;
				    scrollSpeed = std::clamp(scrollSpeed, 0.2f, 4.0f);
			    }
		    } else if (e->y < 0) {
			    if (!m_mode) {    // 2D mode
				    // transform()->position(transform()->position() + glm::vec3(0.0f, 0.0f, .5f));
				    scrollSpeed += 0.2f;
				    scrollSpeed = std::clamp(scrollSpeed, 0.2f, 4.0f);
			    } else {    // 3D mode
				    transform()->position(transform()->position() - transform()->GetFrontVector() * .5f);
				    scrollSpeed += 0.2f;
				    scrollSpeed = std::clamp(scrollSpeed, 0.2f, 4.0f);
			    }
		    }
		    return false;
	    },
	    2
	);

	listener()->Subscribe<event::WindowMouseButton>([this](const event::WindowMouseButton* e) -> bool {
		if (!ToastEditor::viewport_window().isFocused) {
			return false;
		}
		if (ToastEditor::isSimulate) {
			return false;
		}

		// drag move with middle button
		if (e->button == SDL_BUTTON_MIDDLE) {
			if (e->action == event::WINDOW_INPUT_PRESSED) {
				m_dragButton = true;
				m_button = true;
			} else if (e->action == event::WINDOW_INPUT_RELEASED) {
				m_dragButton = false;
				m_button = false;
			}
		}
		return false;
	});

	listener()->Subscribe<event::WindowMousePosition>([this](const event::WindowMousePosition* e) -> bool {
		if (!ToastEditor::viewport_window().isFocused) {
			return false;
		}
		if (ToastEditor::isSimulate) {
			return false;
		}
		if (!m_button && !m_dragButton) {
			m_lastMousePos = glm::vec2(e->x, e->y);
			return false;
		}
		m_mousePos = glm::vec2(e->x, e->y);
		if (m_mousePos - m_lastMousePos == glm::vec2(0, 0)) {
			TOAST_INFO("{}, {}", m_mouseDelta.x, m_mouseDelta.y);
			return false;
		}
		m_mouseDelta = m_mousePos - m_lastMousePos;
		m_lastMousePos = m_mousePos;
		return false;
	});

	m_listener.Subscribe2D("move_camera", [this](const input::Action2D* a) {
		m_dir = { 0, 0, m_dir.z };
		if (!ToastEditor::viewport_window().isFocused) {
			return;
		}
		if (ToastEditor::isSimulate) {
			return;
		}
		auto dir = a->value;
		m_dir = glm::vec3(dir.x, dir.y, m_dir.z);
	});

	m_listener.Subscribe1D("zoom_camera", [this](const input::Action1D* a) {
		m_dir = { m_dir.x, m_dir.y, 0 };
		if (!ToastEditor::viewport_window().isFocused) {
			return;
		}
		if (ToastEditor::isSimulate) {
			return;
		}
		auto dir = a->value;
		m_dir = glm::vec3(m_dir.x, m_dir.y, dir);
	});

	m_listener.Subscribe0D("rotate_camera", [this](const input::Action0D* a) {
		if (a->state != input::Action0D::State::Ongoing) {
			m_button = false;
			return;
		}
		m_button = true;
		// m_mouseDelta = glm::vec2(0.0f);
	});
}

void EditorCamera::Begin() {
	toast::Camera::Begin();
	// Initialize once from the actual starting orientation
	InitAnglesFromQuat(transform()->rotationQuat());
}

void EditorCamera::Tick() {
	Camera::Tick();
	scrollSpeed = abs(scrollSpeed);
	if (!m_mode)    // 2D mode
	{
		transform()->rotation(glm::vec3(0.0f));
		transform()->position(transform()->position() + m_dir * scrollSpeed * transform()->worldPosition().z * static_cast<float>(Time::delta()));

		if (m_dragButton) {
			transform()->worldPosition(
			    transform()->worldPosition() + glm::vec3(
			                                       -m_mouseDelta.x * kMouseSensitivity * 0.01f * transform()->worldPosition().z,
			                                       m_mouseDelta.y * kMouseSensitivity * 0.01f * transform()->worldPosition().z,
			                                       0.0f
			                                   )
			);
			// Reset delta after consuming
			m_mouseDelta = glm::vec2(0.0f);
		}
	} else {    // 3D mode
		glm::vec3 local_dir = glm::vec3(m_dir.x, m_dir.z, m_dir.y);
		transform()->position(
		    transform()->position() + transform()->GetRightVector() * local_dir.x * (scrollSpeed * 10) * static_cast<float>(Time::delta())
		);
		transform()->position(
		    transform()->position() + transform()->GetUpVector() * local_dir.y * (scrollSpeed * 10) * static_cast<float>(Time::delta())
		);
		transform()->position(
		    transform()->position() + transform()->GetFrontVector() * local_dir.z * (scrollSpeed * 10) * static_cast<float>(Time::delta())
		);

		// Handle middle mouse drag in 3D mode
		if (m_dragButton) {
			transform()->position(transform()->position() - transform()->GetRightVector() * m_mouseDelta.x * kMouseSensitivity);
			transform()->position(transform()->position() + transform()->GetUpVector() * m_mouseDelta.y * kMouseSensitivity);
			// Reset delta after consuming
			m_mouseDelta = glm::vec2(0.0f);
		}

		if (m_button && !m_dragButton) {
			// Update yaw/pitch by mouse deltas
			s_yawDeg -= m_mouseDelta.x * kMouseSensitivity;
			s_pitchDeg -= m_mouseDelta.y * kMouseSensitivity;

			// Clamp pitch
			s_pitchDeg = glm::clamp(s_pitchDeg, -kPitchLimit, kPitchLimit);

			// Build orientation and apply
			const glm::quat orientation = MakeOrientation(s_yawDeg, s_pitchDeg);
			transform()->rotationQuat(orientation);

			// Reset delta after consuming
			m_mouseDelta = glm::vec2(0.0f);
		}
	}
}

void EditorCamera::InitAnglesFromQuat(const glm::quat& q) {
	const glm::vec3 f = glm::normalize(q * glm::vec3(0.0f, 0.0f, -1.0f));
	s_yawDeg = glm::degrees(std::atan2(f.x, -f.z));
	const float horiz = glm::max(1e-6f, glm::length(glm::vec2(f.x, f.z)));
	s_pitchDeg = glm::degrees(std::atan2(f.y, horiz));
	s_pitchDeg = glm::clamp(s_pitchDeg, -kPitchLimit, kPitchLimit);
	s_anglesInitialized = true;
}

glm::quat EditorCamera::MakeOrientation(float yawDeg, float pitchDeg) {
	const float yaw_rad = glm::radians(yawDeg);
	const float pitch_rad = glm::radians(pitchDeg);

	const glm::quat q_yaw = glm::angleAxis(yaw_rad, glm::vec3(0.0f, 1.0f, 0.0f));
	const glm::vec3 right = glm::normalize(q_yaw * glm::vec3(1.0f, 0.0f, 0.0f));
	const glm::quat q_pitch = glm::angleAxis(pitch_rad, right);

	// Apply yaw first, then pitch about local right: orientation = qPitch * qYaw
	return glm::normalize(q_pitch * q_yaw);
}
}
