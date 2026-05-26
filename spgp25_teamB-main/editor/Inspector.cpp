#include "Inspector.hpp"

#include "Actions/Actions.hpp"
#include "Tools/MenuItems.hpp"
#include "Tools/VisibilityToggles.hpp"

#include <Toast/Objects/Object.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>

using toast::Object;

namespace editor {
void Inspector::Show(Object* selected_object) {
	PROFILE_ZONE;
	ImGui::Begin("Inspector");
	if (!selected_object) {
		ImGui::End();
		return;
	}

	std::string name_buffer = selected_object->name();

	bool enabled = selected_object->enabled();
	if (ImGui::Checkbox("##enabled", &enabled)) {
		selected_object->enabled(enabled);
	}
	ImGui::SameLine();
	if (ImGui::InputText("Name", &name_buffer)) {
		std::ranges::replace(name_buffer, ' ', '_');
		selected_object->name(name_buffer.data());
	}
	ImGui::Spacing();

	ImGui::Text("Object Type: %s", selected_object->type());
	ImGui::Text("Object ID: %d", selected_object->id());
	// ImGui::Text("Object Ptr: %ptr", selected_object);
	ImGui::NewLine();

	selected_object->Inspector();

	ImGui::End();
}

void Inspector::ShowChildren(Object* selected_object) {
	PROFILE_ZONE;
	ImGui::Begin("Children", nullptr, ImGuiWindowFlags_MenuBar);

	if (ImGui::BeginMenuBar()) {
		VisibilityToggles::Show(m_showScene, m_showActor, m_showComponent, m_showDisabled);
		ImGui::EndMenuBar();
	}

	if (!selected_object) {
		ImGui::End();
		return;
	}

	for (auto& [id, child] : selected_object->children.GetAll()) {
		if (child->base_type() == toast::ActorT && !m_showActor) {
			continue;
		}
		if (child->base_type() == toast::ComponentT && !m_showComponent) {
			continue;
		}
		if (child->base_type() == toast::SceneT && !m_showScene) {
			continue;
		}

		if (!m_showDisabled && !child->enabled()) {
			continue;
		}

		bool show_component = ImGui::CollapsingHeader(std::format("{0}: {1}", child->id(), child->name()).c_str(), ImGuiTreeNodeFlags_AllowOverlap);
		constexpr float BUTTON_WIDTH = 80.0f;
		ImGui::SameLine(ImGui::GetContentRegionAvail().x - BUTTON_WIDTH);
		if (ImGui::Button(std::format("Remove##{0}", id).c_str(), ImVec2(BUTTON_WIDTH, 0))) {
			ActionStack::DeleteObject(child.get());
		}

		if (show_component) {
			ImGui::Indent(20);
			child->Inspector();
			ImGui::Unindent(20);
		}
	}

	if (ImGui::InvisibleButton("ChildrenBackground", ImGui::GetContentRegionAvail())) { }
	if (ImGui::BeginPopupContextItem("##WindowContext", ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight)) {
		if (ImGui::BeginMenu("Create")) {
			if (selected_object->base_type() == toast::SceneT && ImGui::BeginMenu("Scene")) {
				CreateSceneMenuItem(selected_object);
				ImGui::EndMenu();    // Scene
			}
			if (selected_object->base_type() != toast::ComponentT && ImGui::BeginMenu("Actor")) {
				CreateActorMenuItem(selected_object);
				ImGui::EndMenu();    // Actor
			}
			if (selected_object->base_type() == toast::ActorT && ImGui::BeginMenu("Component")) {
				CreateComponentMenuItem(selected_object);
				ImGui::EndMenu();    // Component
			}
			ImGui::EndMenu();      // Create
		}
		ImGui::EndPopup();
	}
	ImGui::End();
}

}
