#include "App.hpp"

#include "Actions/Actions.hpp"
// #include <Toast/Input/InputComponent.hpp>
// #include <Toast/Input/InputSystem.hpp>
#include "../src/InputSystemTest.hpp"
#include "Objects/EditorScene.hpp"
#include "Toast/Input/InputListener.hpp"
#include "Tools/BaseTypeRegistry.hpp"
#include "Tools/PopupModal.hpp"

#include <Toast/Engine.hpp>
#include <Toast/Log.hpp>
#include <Toast/Objects/Object.hpp>
#include <Toast/SimulateWorldEvent.hpp>
#include <Toast/World.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

#ifdef _WIN32
#include <Windows.h>
#endif

// This is so I can act have it -ax
#include <Debug/InputTest.hpp>
#include <Toast/Audio/Audio.hpp>

namespace toast {
Engine* CreateApplication() {
	return new editor::ToastEditor();
}
}

namespace editor {

ToastEditor* ToastEditor::m_instance = nullptr;
unsigned int ToastEditor::EngineSceneId = -1;
bool ToastEditor::isSimulate = false;
PopupModal ToastEditor::modal;

ToastEditor* ToastEditor::Instance() {
	return m_instance;
}

ToastEditor::ToastEditor() {
	if (!std::filesystem::exists("./imgui.ini")) {
		std::filesystem::copy("./assets/imgui.ini", "./imgui.ini");
	}

	// m_inputComponent = std::make_unique<input::InputComponent>();
}

void ToastEditor::Begin() {
	Engine::Begin();
	PROFILE_ZONE;

	auto e = audio::load_bank("assets/AUDIO/Desktop/Music.bank");
	auto o = audio::load_bank("assets/AUDIO/Desktop/Master.strings.bank");
	auto s = audio::load_bank("assets/AUDIO/Desktop/Master.bank");
	if (!e.has_value()) {
		TOAST_ERROR("Error loading music bank");
	}

	audio::load_event("event:/City");
	audio::load_event("event:/Port");

	
	m_instance = this;

	m_listener.Subscribe<toast::SimulateWorldEvent>([this](const toast::SimulateWorldEvent* e) -> bool {
		isSimulate = e->value;
		if (not e->value) {
			input::SetLayout("editor");
			toast::Window::GetInstance()->SetShowMouseCursor(true);
		}
		return false;
	});

	// Create editor scene
	m_editorScene = std::make_unique<EditorScene>();
	toast::World::Instance()->SetEditorScene(m_editorScene.get());

	Registry::Init();
	m_theme.LoadDefaultDark();
	m_assetBrowser.Init();
	m_statusBar.Init();
	m_viewportWindow.Init();
	m_hierarchy.Init();

	ActionStack::Init();

	// input::SetActiveLayout("editor");
	event::Send(new toast::SimulateWorldEvent(false));

	TOAST_INFO("Editor Started");
}

void ToastEditor::EditorTick() {
	PROFILE_ZONE;
	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

	m_statusBar.Show();
	m_postProcessEditor.Show();
	m_hierarchy.Show();
	m_inspector.Show(m_hierarchy.SelectedObject());
	m_inspector.ShowChildren(m_hierarchy.SelectedObject());
	m_assetBrowser.Show();
	m_viewportWindow.Show();
	

	m_settingsPopup.Show();
	m_inputLayoutPopup.Show();
	modal.Show();
}

}
