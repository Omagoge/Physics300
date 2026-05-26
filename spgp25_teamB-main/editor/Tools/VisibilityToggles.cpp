#include "VisibilityToggles.hpp"

#include <imgui.h>
#include <ranges>

namespace editor {

void VisibilityToggles::Show(bool& show_scenes, bool& show_actors, bool& show_components, bool& show_disabled) {
	const std::vector<ToggleButton> toggles = {
		{ "S",     &show_scenes },
		{ "A",     &show_actors },
		{ "C", &show_components },
		{ "D",   &show_disabled },
	};

	ImGuiStyle& style = ImGui::GetStyle();
	const ImVec4 prev_button = style.Colors[ImGuiCol_Button];
	const ImVec4 prev_hovered = style.Colors[ImGuiCol_ButtonHovered];
	const ImVec4 prev_active = style.Colors[ImGuiCol_ButtonActive];

	for (auto i : std::views::iota(0u, toggles.size())) {
		if (*toggles[i].value) {
			style.Colors[ImGuiCol_Button] = ImVec4(0.2f, 0.7f, 0.2f, 1.0f);
			style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.8f, 0.3f, 1.0f);
			style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.1f, 0.6f, 0.1f, 1.0f);
		} else {
			style.Colors[ImGuiCol_Button] = prev_button;
			style.Colors[ImGuiCol_ButtonHovered] = prev_hovered;
			style.Colors[ImGuiCol_ButtonActive] = prev_active;
		}
		if (ImGui::Button(toggles[i].label)) {
			*toggles[i].value = !*toggles[i].value;
		}
		if (i < 2) {
			ImGui::SameLine();
		}
	}

	// Restore colors to default after buttons
	style.Colors[ImGuiCol_Button] = prev_button;
	style.Colors[ImGuiCol_ButtonHovered] = prev_hovered;
	style.Colors[ImGuiCol_ButtonActive] = prev_active;
}

}
