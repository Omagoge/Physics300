/// @file PostProcessingEditor.cpp
/// @author dario
/// @date 25/03/2026.

#include "PostProcessingEditor.hpp"

#include "Toast/Renderer/IRendererBase.hpp"
#include "Toast/Renderer/PostProcessManager.hpp"
#include "imgui.h"

void editor::PostProcessingEditor::Show() {
	if (!ImGui::Begin("Post Processing")) {
		ImGui::End();
		return;
	}

	auto* renderer = renderer::IRendererBase::GetInstance();
	auto* ppm = renderer ? renderer->GetPostProcessManager() : nullptr;
	if (!ppm) {
		ImGui::TextUnformatted("PostProcessManager unavailable.");
		ImGui::End();
		return;
	}

	ImGui::Separator();
	ppm->GlobalInspector();

	ImGui::End();

}

