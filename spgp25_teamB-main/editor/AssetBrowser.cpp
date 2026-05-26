/// @file AssetBrowser.cpp
/// @author dario
/// @date 18/10/2025.

#include "AssetBrowser.hpp"

#include "App.hpp"
#include "imgui_internal.h"

#include <Toast/Objects/Scene.hpp>
#include <Toast/Renderer/Material.hpp>
#include <Toast/Resources/ResourceManager.hpp>
#include <Toast/World.hpp>
#include <imgui.h>

namespace editor {

AssetBrowser::AssetBrowser() : m_currentDir("assets") { }

constexpr float m_padding = 16.0f;
constexpr float m_thumbSize = 75.0f;
constexpr float cellsize = m_thumbSize + m_padding;

renderer::Material* g_mat = nullptr;

void AssetBrowser::Init() {
	m_needsRescan = true;
	RescanDirectory();
}

void AssetBrowser::RescanDirectory() {
	PROFILE_ZONE;
	m_entries.clear();

	const std::filesystem::path base("assets");
	if (m_currentDir.empty() || !std::filesystem::exists(m_currentDir)) {
		m_currentDir = base;
	}

	std::error_code ec;
	for (const auto& de : std::filesystem::directory_iterator(m_currentDir, ec)) {
		if (ec) {
			break;    // stop on error
		}

		if (de.is_directory()) {
			editor::ResourceSlot::Entry e;
			e.isDirectory = true;
			e.name = de.path().filename().string();
			e.extension = "";
			e.icon = resource::ResourceManager::GetInstance()->LoadResource<Texture>(resource::ResourceManager::kFolderIconPath);
			m_entries.emplace_back(std::move(e));
			continue;
		}

		auto rel = std::filesystem::relative(de.path(), base, ec);

		m_entries.emplace_back(std::move(resource::ResourceManager::CreateResourceSlotEntry(rel)));
	}

	std::sort(m_entries.begin(), m_entries.end(), [](const editor::ResourceSlot::Entry& a, const editor::ResourceSlot::Entry& b) {
		if (a.isDirectory != b.isDirectory) {
			return a.isDirectory > b.isDirectory;
		}
		return a.name < b.name;
	});

	m_needsRescan = false;
}

void AssetBrowser::RightClickEntry(const ResourceSlot::Entry& entry) {
	if (entry.isDirectory) {
		return;
	}

	// do not right click for these types
	if (entry.extension == ".mat" || entry.extension == ".scene") {
		return;
	}

	//@TODO
}

void AssetBrowser::Show() {
	PROFILE_ZONE;

	m_renameTextModal.Show();
	m_materialEditor.Show();

	if (ImGui::Begin("Asset Browser")) {
		// Navigation bar
		bool can_go_back = (m_currentDir != std::filesystem::path("assets"));

		if (ImGui::Button("Refresh")) {
			m_needsRescan = true;
		}
		if (can_go_back) {
			ImGui::SameLine();
			if (ImGui::Button("<- Back")) {
				m_currentDir = m_currentDir.parent_path();
				m_needsRescan = true;
			}
		}
		ImGui::SameLine();
		ImGui::SetCursorPosX(
		    ImGui::GetCursorPosX() + ImMax(0.0f, ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(m_currentDir.string().c_str()).x)
		);
		ImGui::TextUnformatted(m_currentDir.string().c_str());

		if (m_needsRescan) {
			RescanDirectory();
		}

		float panel_width = ImGui::GetContentRegionAvail().x;
		int column_count = static_cast<int>(panel_width / cellsize);
		column_count = std::max(1, column_count);

		if (ImGui::BeginPopup("AssetContextMenu")) {
			ImGui::TextDisabled("hiiiii");
			ImGui::EndPopup();
		}

		ImGui::Columns(column_count, nullptr, false);

		bool in_asset_context_menu = false;
		unsigned int index = 0;
		for (const auto& e : m_entries) {
			ImGui::PushID(index);

			ImGui::ImageButton("AssetButton", e.icon->id(), { m_thumbSize, m_thumbSize }, ImVec2(0, 1), ImVec2(1, 0));

			if (ImGui::IsItemHovered()) {
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					if (e.isDirectory) {
						m_currentDir /= e.name;
						m_needsRescan = true;
					} else {
						// TODO: open handlers by type
						if (e.extension == ".scene") {
							CLIENT_INFO("Loading Scene: {}", e.relativePath.string());
							// Unload Scene
							for (const auto& id : toast::World::GetChildren() | std::views::keys) {
								toast::World::UnloadScene(id);
							}
							toast::World::LoadSceneSync(e.relativePath.string());
						} else if (e.extension == ".mat") {
							m_materialEditor.enabled = true;
							m_materialEditor.SetMaterial(e.relativePath.string());
						}
					}
				}
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
					ImGui::OpenPopup("AssetContextMenu");
				}
			}

			if (ImGui::BeginPopup("AssetContextMenu")) {
				RightClickEntry(e);
				in_asset_context_menu = true;
				ImGui::EndPopup();
			}

			if (!e.isDirectory) {
				if (ImGui::BeginDragDropSource()) {
					ImGui::SetDragDropPayload("RESOURCE_STRUCT", &e, sizeof(e));
					ImGui::Text("%s", e.name.c_str());
					ImGui::SameLine();
					ImGui::Image(e.icon->id(), ImVec2(32, 32), ImVec2(0, 1), ImVec2(1, 0));
					ImGui::EndDragDropSource();
				}
			}

			ImGui::TextWrapped("%s", e.name.c_str());

			ImGui::NextColumn();
			ImGui::PopID();
			index++;
		}

		// Context menu
		if (!in_asset_context_menu) {
			if (ImGui::BeginPopupContextWindow()) {
				if (ImGui::BeginMenu("Create")) {
					if (ImGui::MenuItem("Material")) {
						// Create a new material file

						std::string material_name = "new_material.mat";

						// imgui rename
						m_renameTextModal.Init("Rename File", material_name, [this](const std::string& new_name) {
							std::string new_mat_path = m_currentDir.string() + "/" + new_name;
							// Create default material JSON
							nlohmann::json mat_json;
							mat_json["shaderPath"] = "SHADERS/default.shader";

							// Save to file
							std::ofstream ofs(new_mat_path);
							if (ofs.is_open()) {
								ofs << mat_json.dump(4);
								ofs.close();
								CLIENT_INFO("Created new material at {}", new_mat_path);
								m_needsRescan = true;
							} else {
								CLIENT_ERROR("Failed to create material file at {}", new_mat_path);
							}
						});
					}
					ImGui::EndMenu();
				}
				ImGui::EndPopup();
			}
		}

		ImGui::Columns(1);
	}
	ImGui::End();
}
}
