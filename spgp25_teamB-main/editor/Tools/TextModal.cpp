#include "TextModal.hpp"

#include <Toast/Log.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>

namespace editor {

void TextModal::Init(std::string_view name, std::string_view defaultText, const std::function<void(std::string&)>& callback) {
	m_enabled = true;
	m_menuName = name;
	m_callback = callback;
	m_textBuffer = defaultText;
}

void TextModal::Show() {
	if (m_enabled) {
		ImGui::OpenPopup(m_menuName.c_str());
	}

	if (ImGui::BeginPopupModal(m_menuName.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
		auto input_flags = ImGuiInputTextFlags_EnterReturnsTrue;
		if (ImGui::InputText("##TextMenu", &m_textBuffer, input_flags)) {
			m_callback(m_textBuffer);
			CLIENT_INFO("Text Menu: {} returned string: {}", m_menuName, m_textBuffer);
			m_enabled = false;
			ImGui::CloseCurrentPopup();
		}
		// if (!m_enabled) {
		//   ImGui::CloseCurrentPopup();
		// }
		ImGui::EndPopup();
	}
}
}
