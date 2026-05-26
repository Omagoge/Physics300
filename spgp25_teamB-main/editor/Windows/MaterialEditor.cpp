/// @file MaterialEditor.cpp
/// @author dario
/// @date 17/11/2025.

#include "MaterialEditor.hpp"

#include <Toast/Resources/ResourceManager.hpp>

#ifdef TOAST_EDITOR
#include <imgui.h>
#endif

void editor::MaterialEditor::Show() {
	if (!enabled) {
		return;
	}

	if (ImGui::Begin("MaterialEditor")) {
		m_material->ShowEditor();
	}
	ImGui::End();
}

void editor::MaterialEditor::SetMaterial(std::string_view path) {
	std::string result(path);
	std::ranges::replace(result, '\\', '/');
	m_material = resource::ResourceManager::GetInstance()->LoadResource<renderer::Material>(result);
}
