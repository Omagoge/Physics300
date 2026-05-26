#include "MenuItems.hpp"

#include "Actions/Actions.hpp"
#include "App.hpp"
#include "Tools/BaseTypeRegistry.hpp"

#include <Toast/Log.hpp>
#include <Toast/Objects/Object.hpp>
#include <Toast/Objects/Scene.hpp>
#include <Toast/World.hpp>
#include <filesystem>
#include <imgui.h>
#include <ranges>
#include <utility>
#include <vector>

namespace editor {

std::vector<std::string> g_scenes;
std::vector<std::string> g_nested_folders;
std::vector<std::vector<std::string>> g_nested_scenes;

void RefreshSceneDir() {
	g_scenes.clear();
	g_nested_scenes.clear();
	g_nested_folders.clear();
	namespace fs = std::filesystem;
	fs::path base = "assets/SCENES";

	for (const auto& entry : fs::directory_iterator(base)) {
		if (entry.is_regular_file() && entry.path().extension() == ".scene") {
			g_scenes.push_back(entry.path().filename().string());
		} else if (entry.is_directory()) {
			std::string folder = entry.path().filename().string();
			g_nested_folders.push_back(folder);

			std::vector<std::string> nest;
			for (const auto& sub : fs::directory_iterator(entry.path())) {
				if (sub.is_regular_file() && sub.path().extension() == ".scene") {
					nest.push_back(sub.path().filename().string());
				}
			}
			g_nested_scenes.push_back(std::move(nest));
		}
	}
}

void LoadSceneMenuItem() {
	if (g_scenes.empty()) {
		RefreshSceneDir();
	}
	for (const auto& [index, nested] : g_nested_scenes | std::views::enumerate) {
		if (ImGui::BeginMenu(g_nested_folders[index].c_str())) {
			for (auto scene_name : nested) {
				if (ImGui::MenuItem(scene_name.c_str())) {
					CLIENT_INFO("Loading Scene: {0}", scene_name);
					toast::World::LoadSceneSync(std::format("SCENES/{}/{}", g_nested_folders[index], scene_name));
					RefreshSceneDir();
					ImGui::EndMenu();
					return;
				}
			}
			ImGui::EndMenu();
		}
	}
	for (auto scene_name : g_scenes) {
		if (ImGui::MenuItem(scene_name.c_str())) {
			CLIENT_INFO("Loading Scene: {0}", scene_name);
			toast::World::LoadSceneSync(std::format("SCENES/{}", scene_name));
			RefreshSceneDir();
		}
	}
}

int SaveSceneMenuItem() {
	if (toast::World::GetChildren().size() < 1) {
		ImGui::Text("No Scenes in World");
	}
	for (const auto& [id, scene] : toast::World::GetChildren()) {
		if (ToastEditor::EngineSceneId == id) {
			continue;
		}
		if (ImGui::MenuItem(std::format("{1}: {0}", scene->name(), scene->id()).c_str())) {
			if (ToastEditor::hierarchy().warning) {
				ToastEditor::modal.Init("ERROR", []() {
					ImGui::Text("NAME CONFICTS IN SCENE");
					return ImGui::Button("Ok");
				});
				return -1;
			}
			// NOTE: now the Get Scenes returns Objects so we need to cast to Scene*, bruh
			auto* scene_ptr = dynamic_cast<toast::Scene*>(scene.get());
			if (!scene_ptr) {
				TOAST_ERROR("Get Scenes Function Returned Something thats NOT A SCENE");
			}
			if (scene_ptr->json_path().empty()) {
				auto json = scene->Save();
				std::string path = scene->name();
				std::ranges::replace(path, ' ', '_');
				path = std::format("scenes/{}.scene", path);
				json["file_path"] = path;
				path = std::format("assets/{}", path);
				resource::ResourceManager::SaveFile(path, json.dump(2));
			} else {
				auto json = scene->Save();
				std::string path = std::format("assets/{0}", scene_ptr->json_path());
				resource::ResourceManager::SaveFile(path, json.dump(2));
			}
			RefreshSceneDir();
			return id;
		}
	}
	return -1;
}

unsigned int CreateSceneMenuItem(toast::Object* parent) {
	unsigned int id = -1;
	for (const auto& [type, name] : Registry::BaseTypeRegistry()) {
		if (name == "EditorScene") {
			continue;
		}
		if (type == toast::SceneT && ImGui::MenuItem(name.c_str())) {
			id = ActionStack::CreateObject(parent, name);
		}
	}
	return id;
}

unsigned int CreateActorMenuItem(toast::Object* parent) {
	unsigned int id = -1;
	for (const auto& [type, name] : Registry::BaseTypeRegistry()) {
		if (name == "EditorCamera") {
			continue;
		}
		if (type == toast::ActorT && ImGui::MenuItem(name.c_str())) {
			id = ActionStack::CreateObject(parent, name);
		}
	}
	return id;
}

unsigned int CreateComponentMenuItem(toast::Object* parent) {
	unsigned int id = -1;
	for (const auto& [type, name] : Registry::BaseTypeRegistry()) {
		if (type == toast::ComponentT && ImGui::MenuItem(name.c_str())) {
			id = ActionStack::CreateObject(parent, name);
		}
	}
	return id;
}
}
