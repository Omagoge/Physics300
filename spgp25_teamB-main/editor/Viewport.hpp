/**
 * @file Viewport.hpp
 * @author Dante / Dario
 * @date 28/10/25
 *
 * @brief Viewport for editor
 */

#pragma once

#include "Objects/EditorGizmo.hpp"

#include <Toast/Event/ListenerComponent.hpp>

namespace editor {

class ViewportWindow {
public:
	void Init();
	void Show();

	bool is_inFucking_Actual_Focus();

	bool isFocused = false;

private:
	void ShowViewportStatusBar();
	void SetViewMode(ViewMode view_mode);

	void ShowViewport();
	void ShowGame();

	unsigned int m_viewportTextureId = 0;

	event::ListenerComponent m_listener;

	EditorGizmo m_gizmos;
	ViewMode m_viewMode = VIEW2D;

	// Cached viewport rect in screen space for mouse hit-testing
	ImVec2 m_viewportMin { 0.0f, 0.0f };
	ImVec2 m_viewportMax { 0.0f, 0.0f };
};
}
