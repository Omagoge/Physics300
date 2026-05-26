#include "SettingsEditor.hpp"

#include "App.hpp"

#include <Toast/Log.hpp>
#include <Toast/Renderer/IRendererBase.hpp>
#include <imgui.h>

#include <algorithm>
#include <fstream>

namespace editor {

void SettingsPopup::LoadJson() {
	std::ifstream f("assets/project_settings.toast");
	if (!f.is_open()) {
		TOAST_WARN("Unable to Open Settings Menu File assets/project_settings.toast does not exist");
		enabled = false;
		return;
	}
	m_json = nlohmann::json::parse(f);
	if (m_json["format"] != "projectData") {
		TOAST_WARN("Wrong File Format Expected:projectData got: {}", (std::string)m_json["format"]);
		enabled = false;
	}
}

void SettingsPopup::Show() {
	if (enabled) {
		if (ImGui::Begin("Settings", &enabled, ImGuiWindowFlags_AlwaysAutoResize)) {
			// if (m_json.empty()) {
			// 	LoadJson();
			// }
			// ImGui::SeparatorText("Project Info");
			// // Project Name
			// std::string project_name = m_json["projectName"];
			// ImGui::InputText("Project Name", &project_name);
			// m_json["projectName"] = project_name;
			// // Project Name
			//
			// // Project Version
			// ImGui::PushItemWidth(20);
			// std::array<int, 3> version = m_json["projectVersion"];
			// ImGui::InputInt("Major", version.data(), 0);
			// ImGui::SameLine();
			// ImGui::InputInt("Minor", &version[1], 0);
			// ImGui::SameLine();
			// ImGui::InputInt("Patch", &version[2], 0);
			// m_json["projectVersion"] = version;
			// ImGui::PopItemWidth();
			// // Project Version
			//
			// // Input Layout Paths
			// ImGui::SeparatorText("Input Layouts");
			// std::vector<std::string> input_layouts = m_json["input"]["layouts"];
			// if (ImGui::BeginTable("InputLayoutsTable", 1, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
			// 	ImGui::TableSetupColumn("Paths");
			//
			// 	for (int i = 0; i < input_layouts.size(); ++i) {
			// 		ImGui::PushID(i);
			// 		ImGui::TableNextRow();
			// 		ImGui::TableSetColumnIndex(0);
			//
			// 		ImGui::SetNextItemWidth(200);
			// 		ImGui::InputText("##layout", &input_layouts[i]);
			// 		ImGui::SameLine();
			// 		if (ImGui::Button("Remove")) {
			// 			input_layouts.erase(input_layouts.begin() + i);
			// 			ImGui::PopID();
			// 			break;
			// 		}
			// 		ImGui::PopID();
			// 	}
			// 	ImGui::EndTable();
			// }
			//
			// if (ImGui::Button("Add Layout")) {
			// 	input_layouts.emplace_back("layouts/new.til");
			// }	
			// m_json["input"]["layouts"] = input_layouts;
			// Input Layout Paths

			// Renderer Performance
			ImGui::SeparatorText("Renderer Settings");
			auto* rendererInstance = renderer::IRendererBase::GetInstance();
			if (rendererInstance) {
				const auto& cfg = rendererInstance->GetRendererConfig();
				bool settingsChanged = false;

				bool vsync = cfg.vSync;
				if (ImGui::Checkbox("VSync", &vsync)) {
					rendererInstance->SetVSyncEnabled(vsync);
					settingsChanged = true;
				}

				ImGui::BeginDisabled(vsync);
				int maxFPS = static_cast<int>(cfg.maxFPS);
				ImGui::SetNextItemWidth(160);
				if (ImGui::SliderInt("Max FPS", &maxFPS, 30, 360)) {
					rendererInstance->SetMaxFPS(static_cast<unsigned>(maxFPS));
					settingsChanged = true;
				}
				ImGui::EndDisabled();
				if (vsync) {
					ImGui::SameLine();
					ImGui::TextDisabled("(VSync active)");
				}

				float resolutionScale = cfg.resolutionScale;
				ImGui::SetNextItemWidth(160);
				if (ImGui::SliderFloat("Resolution Scale", &resolutionScale, 0.25f, 1.0f, "%.2f")) {
					rendererInstance->SetResolutionScale(resolutionScale);
					settingsChanged = true;
				}
				
				float lightResolutionScale = cfg.lightResolutionScale;
				ImGui::SetNextItemWidth(160);
				if (ImGui::SliderFloat("Light Resolution Scale", &lightResolutionScale, 0.15f, 1.0f, "%.2f")) {
					rendererInstance->SetLightResolutionScale(lightResolutionScale);
					settingsChanged = true;
				}

				int msaaSamples = static_cast<int>(cfg.msaaSamples);
				ImGui::SetNextItemWidth(160);
				if (ImGui::SliderInt("MSAA Samples", &msaaSamples, 1, 16)) {
					rendererInstance->SetMsaaSamples(static_cast<unsigned>(msaaSamples));
					settingsChanged = true;
				}

				float anisotropyLevel = cfg.anisotropyLevel;
				ImGui::SetNextItemWidth(160);
				if (ImGui::SliderFloat("Anisotropy", &anisotropyLevel, 1.0f, 16.0f, "%.1fx")) {
					rendererInstance->SetAnisotropyLevel(anisotropyLevel);
					settingsChanged = true;
				}

				int shadowMapResolution = static_cast<int>(cfg.shadowMapResolution);
				ImGui::SetNextItemWidth(160);
				if (ImGui::SliderInt("Shadow Map Resolution", &shadowMapResolution, 64, 8192)) {
					rendererInstance->SetShadowMapResolution(static_cast<unsigned>(shadowMapResolution));
					settingsChanged = true;
				}

				int directionalShadowMapResolution = static_cast<int>(cfg.directionalShadowMapResolution);
				ImGui::SetNextItemWidth(200);
				if (ImGui::SliderInt("Directional Shadow Map Resolution", &directionalShadowMapResolution, 64, 8192)) {
					rendererInstance->SetDirectionalShadowMapResolution(static_cast<unsigned>(directionalShadowMapResolution));
					settingsChanged = true;
				}

				int shadowRaymarchSteps = static_cast<int>(cfg.shadowRaymarchSteps);
				ImGui::SetNextItemWidth(160);
				if (ImGui::SliderInt("Shadow Ray Steps", &shadowRaymarchSteps, 1, 128)) {
					rendererInstance->SetShadowRaymarchSteps(static_cast<unsigned>(shadowRaymarchSteps));
					settingsChanged = true;
				}

				ImGui::SeparatorText("Directional Shadows");

				float directionalShadowDistance = cfg.directionalShadowDistance;
				ImGui::SetNextItemWidth(200);
				if (ImGui::SliderFloat("Directional Shadow Distance", &directionalShadowDistance, 16.0f, 500.0f, "%.1f")) {
					rendererInstance->SetDirectionalShadowDistance(directionalShadowDistance);
					settingsChanged = true;
				}

				float directionalShadowRange = cfg.directionalShadowRange;
				ImGui::SetNextItemWidth(200);
				if (ImGui::SliderFloat("Directional Shadow Range", &directionalShadowRange, 8.0f, 300.0f, "%.1f")) {
					rendererInstance->SetDirectionalShadowRange(directionalShadowRange);
					settingsChanged = true;
				}

				float directionalShadowBias = cfg.directionalShadowBias;
				ImGui::SetNextItemWidth(200);
				if (ImGui::SliderFloat("Directional Shadow Bias", &directionalShadowBias, 0.00001f, 0.02f, "%.5f", ImGuiSliderFlags_Logarithmic)) {
					rendererInstance->SetDirectionalShadowBias(directionalShadowBias);
					settingsChanged = true;
				}

				float directionalShadowStrength = cfg.directionalShadowStrength;
				ImGui::SetNextItemWidth(200);
				if (ImGui::SliderFloat("Directional Shadow Strength", &directionalShadowStrength, 0.0f, 1.0f, "%.2f")) {
					rendererInstance->SetDirectionalShadowStrength(directionalShadowStrength);
					settingsChanged = true;
				}

				int displayMode = static_cast<int>(cfg.currentDisplayMode);
				if (ImGui::Combo("Display Mode", &displayMode, "Windowed\0Fullscreen\0")) {
					rendererInstance->SetDisplayMode(static_cast<toast::DisplayMode>(displayMode));
					settingsChanged = true;
				}

				int resolution[2] = { static_cast<int>(cfg.resolution.x), static_cast<int>(cfg.resolution.y) };
				ImGui::SetNextItemWidth(220);
				if (ImGui::InputInt2("Resolution", resolution)) {
					resolution[0] = std::max(1, resolution[0]);
					resolution[1] = std::max(1, resolution[1]);
					rendererInstance->SetResolution(
					    { static_cast<unsigned>(resolution[0]), static_cast<unsigned>(resolution[1]) }
					);
					settingsChanged = true;
				}

				if (settingsChanged) {
					rendererInstance->ApplyRenderSettings();
				}
			}
			// Renderer Performance

			// // Save and Quit buttons
			// ImGui::Separator();
			// if (ImGui::Button("Save")) {
			// 	std::ofstream f("assets/project_settings.toast");
			// 	f << m_json;
			// 	f.close();
			// 	ToastEditor::input_layout_popup().LoadLayoutList();
			// }
			// ImGui::SameLine();
			// if (ImGui::Button("Quit")) {
			// 	enabled = false;
			// }
			// // Save and Quit buttons
		}
		ImGui::End();    // Settings
	} else if (!enabled && !m_json.empty()) {
		m_json.clear();
	}
}
}
