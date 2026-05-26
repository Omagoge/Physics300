#include "PopupModal.hpp"

#include <imgui.h>

namespace editor {

void PopupModal::Init(std::string_view name, const std::function<bool()>& logic) {
	m_menuName = name;
	m_logic = logic;
	m_enabled = true;
}

void PopupModal::Show() {
	if (m_enabled) {
		ImGui::OpenPopup(m_menuName.c_str());
	}
	if (ImGui::BeginPopupModal(m_menuName.c_str(), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize)) {
		if (m_logic()) {
			m_enabled = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

}
