/// @file DebugStats.cpp
/// @author dario
/// @date 20/10/2025.

#include "DebugStats.hpp"

#include "App.hpp"

#include <Toast/Engine.hpp>
#include <Toast/Resources/ResourceManager.hpp>
#include <Toast/Time.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

namespace editor {

void DebugStats::Init() {
	m_resourceManager = resource::ResourceManager::GetInstance();
	m_genericFileIcon = m_resourceManager->LoadResource<Texture>("EDITOR/icons/genericFile.png");
}

void DebugStats::Show() {
	if (!enabled) {
		return;
	}
	if (ImGui::Begin("Debug Stats", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		double d = Time::delta();
		ImGui::Text("Selected Object Id: %i", ToastEditor::hierarchy().SelectedObjectId());
		ImGui::Text("FPS: %.1f", 1.0f / d);
		ImGui::Text("Frame Time: %.3f ms", d * 1000.0f);


		if (ImGui::CollapsingHeader("Resource Manager")) {
			auto stats = m_resourceManager->GetCachedResources();

			ImGui::Columns(4, 0, false);

			unsigned int total_resources = 0;
			for (auto& [path, entry] : stats) {
				ImGui::PushID(total_resources);
				std::shared_ptr<Texture> texture = m_genericFileIcon;
				bool tex = false;
				if (auto t = dynamic_cast<Texture*>(entry.get())) {
					texture = std::dynamic_pointer_cast<Texture>(entry);
					tex = true;
				}
				ImGui::TextWrapped("%s", path.c_str());
				ImGui::TextWrapped("%s", typeid(*entry).name());
				ImGui::TextWrapped("Use count: %i", tex ? entry.use_count() - 3 : entry.use_count() - 2);
				ImGui::ImageButton("Asset", (ImTextureID)texture->id(), ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
				ImGui::NextColumn();
				ImGui::PopID();
				total_resources++;
			}
			ImGui::EndColumns();
			ImGui::Text("Total Resources: %d", total_resources);
			if (ImGui::Button("You should purge your resources... NOW!!!!")) {
				toast::Engine::ForcePurgeResources();
			}
		}
	}
	ImGui::End();
}
}
