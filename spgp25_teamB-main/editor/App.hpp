/**
 * @file App.hpp
 * @author Dante Harper
 * @date 06/10/25
 *
 * @brief Editor Executable for the engine
 */

#pragma once

#include "AssetBrowser.hpp"
#include "Hierarchy.hpp"
#include "Inspector.hpp"
#include "Objects/EditorScene.hpp"
#include "StatusBar.hpp"
#include "Theme.hpp"
#include "Tools/PopupModal.hpp"
#include "Viewport.hpp"
#include "Windows/InputLayoutEditor.hpp"
#include "Windows/SettingsEditor.hpp"

#include <Toast/Event/ListenerComponent.hpp>
// #include <Toast/Input/InputComponent.hpp>
#include "Windows/PostProcessingEditor.hpp"

#include <Toast/Engine.hpp>

namespace toast {
class Object;
}

namespace editor {
class ToastEditor final : public toast::Engine {
public:
	ToastEditor();

	[[nodiscard]]
	static ToastEditor* Instance();

	static unsigned int EngineSceneId;

	EditorTheme& theme() {
		return m_theme;
	}

	// clang-format off
  [[nodiscard]]
	static StatusBar& status_bar() { return Instance()->m_statusBar; }
  [[nodiscard]]
	static Inspector& inspector() { return Instance()->m_inspector; }
  [[nodiscard]]
	static Hierarchy& hierarchy() { return Instance()->m_hierarchy; }
  [[nodiscard]]
	static AssetBrowser& asset_browser() { return Instance()->m_assetBrowser; }
  [[nodiscard]]
	static ViewportWindow& viewport_window() { return Instance()->m_viewportWindow; }
  [[nodiscard]]
	static SettingsPopup& settings_popup() { return Instance()->m_settingsPopup; }
  [[nodiscard]]
	static InputLayoutPopup& input_layout_popup() { return Instance()->m_inputLayoutPopup; }

  // [[nodiscard]]
	// static input::InputComponent& input_component() { return *Instance()->m_inputComponent; }

	// clang-format on

	static bool isSimulate;
	static PopupModal modal;

private:
	static ToastEditor* m_instance;

	void Begin() override;
	void EditorTick() override;
	void EnableScene(toast::Scene* scene);

	EditorTheme m_theme;

	StatusBar m_statusBar;
	Inspector m_inspector;
	Hierarchy m_hierarchy;
	AssetBrowser m_assetBrowser;
	ViewportWindow m_viewportWindow;
	
	PostProcessingEditor m_postProcessEditor;

	SettingsPopup m_settingsPopup;
	InputLayoutPopup m_inputLayoutPopup;

	event::ListenerComponent m_listener;
	// std::unique_ptr<input::InputComponent> m_inputComponent;
	std::unique_ptr<EditorScene> m_editorScene;
};
}
