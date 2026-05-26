#include "Actions.hpp"

#include "App.hpp"
#include "nlohmann/json_fwd.hpp"

#include <Toast/Components/TransformComponent.hpp>
#include <Toast/Objects/Actor.hpp>
#include <Toast/Window/WindowEvents.hpp>
#include <Toast/World.hpp>
#include <cstdlib>
#include <string>
#include <cctype>
#include <SDL3/SDL.h>
#ifdef TOAST_EDITOR
#include <imgui.h>
#endif

namespace editor {

void ActionStack::Init() {
	CLIENT_INFO("Action Stack Initialized");
	GetInstance()->m_listener.Subscribe<event::WindowKey>([](event::WindowKey* e) -> bool {
		if (ToastEditor::isSimulate) {
			return false;
		}

#ifdef TOAST_EDITOR
		// Only block shortcuts while ImGui needs text input (typing in a field / IME).
		// Do not block when ImGui simply wants keyboard navigation.
		if (ImGui::GetIO().WantTextInput) {
			return false;
		}
#endif

		// Debug log incoming key event to help diagnose missing shortcuts
		CLIENT_INFO("WindowKey event: key={} scancode={} action={} mods=0x{:x}", e->key, e->scancode, e->action, e->mods);

		// SDL modifiers are a mask; check for ctrl key
		const bool ctrl = (e->mods & SDL_KMOD_CTRL) != 0;

		// Normalize ASCII key to uppercase if possible
		int key_up = e->key;
		if (key_up >= 0 && key_up <= 0xFF) {
			key_up = static_cast<int>(std::toupper(static_cast<unsigned char>(key_up)));
		}

		if (e->action == 1 && ctrl && (key_up == 'Z' || e->scancode == SDL_SCANCODE_Z)) {
			CLIENT_INFO("UNDO!");
			PopAction();
			return true;
		}
		if (e->action == 1 && ctrl && (key_up == 'V' || e->scancode == SDL_SCANCODE_V)) {
			if (!ToastEditor::viewport_window().isFocused && !ToastEditor::hierarchy().windowSelected) {
				return false;
			}
			CLIENT_INFO("PASTE!");
			PasteObject();
			return true;
		}
		if (e->action == 1 && ctrl && (key_up == 'C' || e->scancode == SDL_SCANCODE_C)) {
			if (!ToastEditor::viewport_window().isFocused && !ToastEditor::hierarchy().windowSelected) {
				return false;
			}
			CLIENT_INFO("COPY!");
			CopyObject(ToastEditor::hierarchy().SelectedObject());
			return true;
		}
		if (e->action == 1 && ctrl && (key_up == 'S' || e->scancode == SDL_SCANCODE_S)) {
			CLIENT_INFO("SAVE!");
			SaveAll();
			return true;
		}
		return false;
	});

	// Fallback: some platforms/backends may produce text input events instead of key events for certain combos.
	GetInstance()->m_listener.Subscribe<event::WindowChar>([](event::WindowChar* e) -> bool {
		// Check ctrl state at the moment of the char event
		const bool ctrl = (SDL_GetModState() & SDL_KMOD_CTRL) != 0;
		if (!ctrl) return false;

		unsigned ch = e->key;
		if (ch <= 0xFF) ch = static_cast<unsigned>(std::toupper(static_cast<unsigned char>(ch)));

		if (ch == 'V') {
			if (!ToastEditor::viewport_window().isFocused && !ToastEditor::hierarchy().windowSelected) return false;
			CLIENT_INFO("PASTE! (via WindowChar)");
			GetInstance()->PasteObject();
			return true;
		}
		if (ch == 'C') {
			if (!ToastEditor::viewport_window().isFocused && !ToastEditor::hierarchy().windowSelected) return false;
			CLIENT_INFO("COPY! (via WindowChar)");
			GetInstance()->CopyObject(ToastEditor::hierarchy().SelectedObject());
			return true;
		}
		if (ch == 'S') {
			CLIENT_INFO("SAVE! (via WindowChar)");
			GetInstance()->SaveAll();
			return true;
		}
		if (ch == 'Z') {
			CLIENT_INFO("UNDO! (via WindowChar)");
			GetInstance()->PopAction();
			return true;
		}
		return false;
	});
}

void ActionStack::SaveAll() {
	if (ToastEditor::hierarchy().warning) {
		ToastEditor::modal.Init("ERROR", []() {
			ImGui::Text("NAME CONFICTS IN SCENE");
			return ImGui::Button("Ok");
		});
		return;
	}
	for (const auto& [id, scene] : toast::World::GetChildren()) {
		if (ToastEditor::EngineSceneId == id) {
			continue;
		}
		auto* scene_ptr = dynamic_cast<toast::Scene*>(scene.get());
		if (!scene_ptr) {
			TOAST_ERROR("Get Scenes Function Returned Something thats NOT A SCENE");
		}
		if (scene_ptr->json_path().empty()) {
			auto json = scene->Save();
			std::string path = scene->name();
			std::ranges::replace(path, ' ', '_');
			path = std::format("SCENES/{}.scene", path);
			json["file_path"] = path;
			path = std::format("assets/{}", path);
			resource::ResourceManager::SaveFile(path, json.dump(2));
		} else {
			auto json = scene->Save();
			std::string path = std::format("assets/{0}", scene_ptr->json_path());
			resource::ResourceManager::SaveFile(path, json.dump(2));
		}
	}
}

void ActionStack::CopyObject(toast::Object* object) {
	if (!object) {
		TOAST_WARN("No Selcted Object To Copy");
		return;
	}
	nlohmann::json& copy_buffer = GetInstance()->m_copyBuffer;
	if (object->name().contains("_(")) {
		auto prefix = object->name().find("_(");
		auto name = object->name().substr(0, prefix);
		copy_buffer["object_name"] = name;
		auto count = std::atoi(object->name().substr(prefix + 2, object->name().length() - 1).c_str());
		copy_buffer["paste_count"] = count + 1;
	} else {
		copy_buffer["object_name"] = object->name();
		copy_buffer["paste_count"] = 0;
	}
	copy_buffer["parent_id"] = (object->parent()) ? object->parent()->id() : -1;
	copy_buffer["object_id"] = object->id();
	copy_buffer["object_data"] = object->Save();
}

void ActionStack::PasteObject() {
	nlohmann::json& copy_buffer = GetInstance()->m_copyBuffer;
	if (copy_buffer.empty()) {
		TOAST_WARN("No Obects in Copy Buffer");
		return;
	}
	unsigned int parent_id = copy_buffer["parent_id"];
	toast::Object::Children* children = nullptr;

	if (parent_id == -1) {
		children = &toast::World::GetChildren();
	} else {
		if (toast::World::Get(parent_id) == nullptr) {
			TOAST_WARN("Copied Objects Parent does not exist anymore resetting copy buffer");
			return;
		}
		children = &toast::World::Get(parent_id)->children;
	}

	copy_buffer["object_data"]["name"] = std::format("{0}_({1})", copy_buffer["object_name"].get<std::string>(), copy_buffer["paste_count"].get<int>());
	copy_buffer["paste_count"] = copy_buffer["paste_count"].get<int>() + 1;

	auto registry = toast::Object::getRegistry();
	std::string type = copy_buffer["object_data"]["type"];
	auto* obj = registry[type](*children, std::nullopt);
	children->_ConfigureObject(obj, copy_buffer["object_data"]["name"].get<std::string>(), copy_buffer["object_data"]);
	toast::World::Get(obj->id())->_LoadTextures();

	nlohmann::json action;
	action["type"] = "create_object";
	action["parent_id"] = copy_buffer["parent_id"];
	action["object_type"] = type;
	action["object_id"] = obj->id();
	GetInstance()->m_actionStack.push(action);

	ToastEditor::hierarchy().SelectObject(obj);
}

void ActionStack::PopAction() {
	if (GetInstance()->m_actionStack.empty()) {
		CLIENT_INFO("NO ACTIONS TO UNDO");
		return;
	}
	nlohmann::json action = GetInstance()->m_actionStack.top();
	if (action["type"] == "create_object") {
		toast::World::Get(action["object_id"].get<unsigned int>())->Nuke();
	} else if (action["type"] == "delete_object") {
		unsigned int parent_id = action["parent_id"];
		toast::Object::Children* children = nullptr;
		if (parent_id == -1) {
			children = &toast::World::GetChildren();
		} else {
			if (toast::World::Get(parent_id) == nullptr) {
				TOAST_ERROR("Object Does not exist for undo to undo");
			}
			children = &toast::World::Get(parent_id)->children;
		}

		auto registry = toast::Object::getRegistry();
		std::string type = action["object_data"]["type"];
		auto* obj = registry[type](*children, action["object_id"]);
		children->_ConfigureObject(obj, action["object_data"]["name"].get<std::string>(), action["object_data"]);

		toast::World::Get(obj->id())->_LoadTextures();
	} else if (action["type"] == "gizmo_object") {
		toast::TransformComponent* comp = nullptr;
		toast::Actor* actor = nullptr;
		if ((comp = dynamic_cast<toast::TransformComponent*>(toast::World::Get(action["component_id"].get<unsigned int>())))) {
		} else if ((actor = dynamic_cast<toast::Actor*>(toast::World::Get(action["actor_id"].get<unsigned int>())))) {
			comp = actor->transform();
		}
		if (comp == nullptr) {
			TOAST_ERROR("EDITOR NEEDS XEINS NEW WORLD TO CREATE OBJECTS WITH SPECIFED ID");
		}
		comp->Load(action["component_data"],false);
	}
	GetInstance()->m_actionStack.pop();
}

void ActionStack::GizmoObject(toast::TransformComponent* transform) {
	nlohmann::json action;
	action["type"] = "gizmo_object";
	action["base_type"] = transform->base_type();
	if (transform->parent()) {
		action["actor_id"] = transform->parent()->id();
	}
	action["component_id"] = transform->id();
	action["component_data"] = transform->Save();
	GetInstance()->m_actionStack.push(action);
}

unsigned int ActionStack::CreateObject(toast::Object* parent, std::string_view type) {
	toast::Object::Children* children = nullptr;
	toast::Object* obj = nullptr;
	if (parent) {
		obj = parent->children.Add(std::string(type));
	} else {
		obj = toast::World::New(type);
	}

	obj->enabled(true);

	nlohmann::json action;
	action["type"] = "create_object";
	action["parent_id"] = (parent) ? parent->id() : -1;
	action["object_type"] = std::string(type);
	action["object_id"] = obj->id();
	GetInstance()->m_actionStack.push(action);
	obj->_LoadTextures();
	return obj->id();
}

void ActionStack::DeleteObject(toast::Object* object) {
	nlohmann::json action;
	action["type"] = "delete_object";
	action["parent_id"] = (object->parent()) ? object->parent()->id() : -1;
	action["object_id"] = object->id();
	action["object_data"] = object->Save();
	GetInstance()->m_actionStack.push(action);
	object->Nuke();
}
}
