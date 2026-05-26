#include "StatusBar.hpp"

#include "Actions/Actions.hpp"
#include "App.hpp"
#include "Objects/EditorCamera.hpp"
#include "Toast/Event/ListenerComponent.hpp"
#include "Tools/DebugStats.hpp"
#include "Tools/MenuItems.hpp"
#include "nlohmann/detail/value_t.hpp"

#include <Toast/GameEvents.hpp>
#include <Toast/Log.hpp>
#include <Toast/Objects/Scene.hpp>
#include <Toast/Profiler.hpp>
#include <Toast/Renderer/HUD/ShowHUDLayer.h>
#include <Toast/SimulateWorldEvent.hpp>
#include <Toast/Window/WindowEvents.hpp>
#include <Toast/World.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>

namespace editor {
void StatusBar::Init() {
	m_debugStats.Init();

	ImGui::GetStyle()._NextFrameFontSizeBase = 20;    // TODO: maybe not hardcode this
	auto dir = std::filesystem::recursive_directory_iterator("assets/FONTS");
	for (const auto& item : dir) {
		// Convert path to UTF-8 (so it works on both windows and linux)
		auto u8 = item.path().u8string();
		std::string utf8_path(u8.begin(), u8.end());

		if (item.is_directory()) {
			// filename() also needs conversion
			auto fname8 = item.path().filename().u8string();
			std::string utf8_name(fname8.begin(), fname8.end());
			m_fonts.emplace_back(utf8_name);
			continue;
		}
		if (item.path().extension() == ".ttf") {
			auto* font = ImGui::GetIO().Fonts->AddFontFromFileTTF(utf8_path.c_str(), 16.0f);
			m_imguiFonts.emplace_back(font);
		}
	}
	ChooseFont(0);
}

void StatusBar::Show() {
	PROFILE_ZONE;
	m_debugStats.Show();
	m_textMenu.Show();
	if (m_imguiDemo) {
		ImGui::ShowDemoWindow();
	}
	if (ImGui::BeginMainMenuBar()) {
		/// Status Bar Area
		FileMenu();
		WindowMenu();
		AppearanceMenu();

		if (ImGui::Button(m_playMode ? "Stop" : "Play")) {
			if (m_playMode) {
				event::Send(new toast::ResetGameFlow());
			}
			m_playMode = !m_playMode;
			event::Send(new toast::SimulateWorldEvent(m_playMode));
			event::Send(new ShowHUDLayerEvent(m_playMode));
		}

		if (ImGui::Button("GameFlow Play")) {
			event::Send(new toast::NextLevel());
			// if (not m_playMode) {
			// 	m_playMode = !m_playMode;
			// 	event::Send(new toast::ResetGameFlow());
			// 	event::Send(new toast::SimulateWorldEvent(m_playMode));
			// 	event::Send(new ShowHUDLayerEvent(m_playMode));
			// 	event::Send(new toast::LoadLevel(0, 0));
			//
			// 	if (not toast::World::Has("PlayerScene")) {
			// 		toast::World::LoadSceneSync("SCENES/PlayerScene.scene");
			// 	}
			// }
		}

		if (m_playMode && ImGui::Button("RestartLevel")) {
			event::Send(new toast::RestartLevel);
		}

		ImGui::Checkbox("Ruler", &ruler);

		ImGui::EndMainMenuBar();
	}
}

void StatusBar::FileMenu() {
	if (ImGui::BeginMenu("File")) {
		if (ImGui::MenuItem("New Scene")) {
			CLIENT_INFO("Loading New Scene");
			ActionStack::CreateObject(nullptr, toast::Scene::static_type());
		}
		if (ImGui::BeginMenu("Load Scene")) {
			LoadSceneMenuItem();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Save Scene")) {
			SaveSceneMenuItem();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Close Scene")) {
			unsigned int id = SaveSceneMenuItem();
			if (id != -1) {
				toast::World::UnloadScene(id);
			}
			ImGui::EndMenu();
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Project Settings")) {
			ToastEditor::settings_popup().enabled = true;
		}
		if (ImGui::MenuItem("Quit ToastEditor")) {
			event::Send(new event::WindowClose());
		}
		ImGui::EndMenu();
	}
}

void StatusBar::WindowMenu() {
	if (ImGui::BeginMenu("Window")) {
		if (ImGui::BeginMenu("Debug")) {
			if (ImGui::Checkbox("Debug Stats", &m_debugStats.enabled)) {
				TOAST_INFO("Toggled Debug Stats: {0}", m_debugStats.enabled);
			}
			if (ImGui::Checkbox("ImGui Demo", &m_imguiDemo)) {
				TOAST_INFO("Toggled ImGui Demo: {0}", m_imguiDemo);
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
}

void StatusBar::AppearanceMenu() {
	if (ImGui::BeginMenu("Appearance")) {
		ImGuiStyle& style = ImGui::GetStyle();
		if (ImGui::BeginMenu("Fonts")) {
			FontSelector();
			if (ImGui::DragFloat("FontSizeBase", &style.FontSizeBase, 0.20f, 5, 40, "%.0f")) {
				style._NextFrameFontSizeBase = style.FontSizeBase;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
}

void StatusBar::FontSelector() {
	if (ImGui::BeginCombo("Select Font", m_fonts[m_currentFont].c_str())) {
		for (int i = 0; i < m_fonts.size(); ++i) {
			if (ImGui::Selectable(m_fonts[i].c_str())) {
				m_currentFont = i;
				for (auto* font : m_imguiFonts) {
					std::string name(font->GetDebugName());
					if (name.find("Regular") != std::string::npos && name.find(m_fonts[i]) != std::string::npos) {
						ImGui::GetIO().FontDefault = font;
						CLIENT_INFO("{}", font->GetDebugName());
					}
				}
			}
		}
		ImGui::EndCombo();
	}
}

void StatusBar::ChooseFont(unsigned int idx) {
	m_currentFont = idx;
	for (auto* font : m_imguiFonts) {
		std::string name(font->GetDebugName());
		if (name.find("Regular") != std::string::npos && name.find(m_fonts[idx]) != std::string::npos) {
			ImGui::GetIO().FontDefault = font;
			CLIENT_INFO("Applied Font: {}", font->GetDebugName());
		}
	}
}

}
