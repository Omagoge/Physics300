#include "Hierarchy.hpp"

#include "Actions/Actions.hpp"
#include "App.hpp"
#include "Objects/EditorCamera.hpp"
#include "Theme.hpp"
#include "Toast/ISerializable.hpp"
#include "Toast/Physics/Collider.hpp"
#include "Toast/Physics/Raycast.hpp"
#include "Toast/Physics/Rigidbody.hpp"
#include "Toast/Resources/ResourceManager.hpp"
#include "Tools/MenuItems.hpp"
#include "Tools/VisibilityToggles.hpp"

#include <Toast/Engine.hpp>
#include <Toast/Log.hpp>
#include <Toast/Objects/Object.hpp>
#include <Toast/Objects/Scene.hpp>
#include <Toast/World.hpp>
#include <cctype>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <optional>
#include <string>
#include <unordered_set>

using toast::Object;

namespace editor {

Hierarchy::~Hierarchy() {
	delete m_listener;
}

void Hierarchy::Init() {
	m_listener = new input::Listener();

	m_listener->Subscribe2D("mouse", [this](const input::Action2D* a) {
		lastMouseWorldPos = a->value;
	});

	m_listener->Subscribe0D("mouse_click", [this](const input::Action0D* a) {
		if (a->state != input::Action0D::State::Started) {
			return;
		}
		OnMouse(lastMouseWorldPos);
	});
}

void Hierarchy::Show() {
	PROFILE_ZONE;

	if (ImGui::Begin("Object Tree", nullptr, ImGuiWindowFlags_MenuBar)) {
		windowSelected = ImGui::IsWindowFocused();

		if (ImGui::BeginMenuBar()) {
			VisibilityToggles::Show(m_showScenes, m_showActors, m_showComponents, m_showDisabled);
			ImGui::EndMenuBar();
		}

		if (ImGui::InputText("Find##search", &m_search)) {
			m_searchLower = m_search;
			std::transform(m_searchLower.begin(), m_searchLower.end(), m_searchLower.begin(), [](unsigned char c) {
				return std::tolower(c);
			});
		}

		m_selectedObject = nullptr;    // i reset the pointer and not the id here to check if the object still exists
		warning = false;

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DrawLinesToNodes |
		                           ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_FramePadding;

		// static std::optional<unsigned> selected_scene = std::nullopt;
		// if (toast::World::GetChildren().size() != 0) {
		// 	auto* curr_scene = selected_scene ? toast::World::Get(*selected_scene) : nullptr;
		// 	if (not selected_scene || not curr_scene) {
		// 		auto iter = toast::World::GetChildren().begin();
		// 		selected_scene = iter->first;
		// 		curr_scene = iter->second.get();
		// 	}
		// 	for (auto& [id, scene] : toast::World::GetChildren()) {
		// 		ImGui::Dummy({ 0, 2 });
		// 		if (ImGui::Selectable(
		// 		        std::format("- {}:{}", scene->id(), scene->name()).c_str(), id == *selected_scene, ImGuiSelectableFlags_SpanAllColumns
		// 		    )) {
		// 			selected_scene = id;
		// 		}
		// 	}
		// } else {
		// 	ImGui::Text("No Scenes Open");
		// 	selected_scene = std::nullopt;
		// }
		// if (selected_scene) {
		// 	ImGui::Dummy({ 0, 4 });
		// 	ImGui::Separator();
		// 	ObjectTree(toast::World::Get(*selected_scene), flags);
		// }

		for (const auto& t : toast::World::GetTickables()) {
			if (ToastEditor::EngineSceneId == t->id()) {
				continue;
			}
			ObjectTree(t, flags);
		}

		// Right Click Outside The Tree
		if (ImGui::InvisibleButton("HierarchyBackground", ImGui::GetContentRegionAvail())) {
			SelectObject(-1);
		}

		// auto* scene = selected_scene ? toast::World::Get(*selected_scene) : nullptr;
		if (ImGui::BeginPopupContextItem("##WindowContext", ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight)) {
			if (ImGui::BeginMenu("Load Scene")) {
				LoadSceneMenuItem();
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Create Scene")) {
				CreateSceneMenuItem(nullptr);
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}

		if (ImGui::IsKeyDown(ImGuiKey_Delete) && not m_isRenaming) {
			if (m_selectedObject) {
				auto* tmp = m_selectedObject;
				SelectObject(-1);
				ActionStack::DeleteObject(tmp);
			}
		}
		if (m_adoptable) {
			auto [parent, orphan] = m_adoptable.value();
			parent->Adopt(orphan);
			m_adoptable = std::nullopt;
		}
	}
	ImGui::End();
}

void Hierarchy::OnMouse(glm::vec2 worldPos) {
	// Only select from viewport clicks with Ctrl held
	if (!ToastEditor::viewport_window().isFocused || !ToastEditor::viewport_window().is_inFucking_Actual_Focus()) {
		return;
	}

	ImGuiIO& io = ImGui::GetIO();
	const bool ctrlDown = io.KeyCtrl;    // uses current ImGui modifier state
	if (!ctrlDown) {
		return;
	}

	toast::Object* hit = physics::PointCast(worldPos);
	if (hit) {
		if (auto c = dynamic_cast<toast::Component*>(SelectedObject())) {
			if (SelectedObject()->parent() == hit) {
				return;
			}
		}
		if (SelectedObject() != hit) {
			SelectObject(hit);
		}
	} else {
		// Optionally clear selection when clicking empty space with Ctrl; currently left commented out
		// SelectObject(nullptr);
	}
}

bool Hierarchy::MatchesQuery(toast::Object* object) {
	if (m_searchLower.empty()) {
		return true;
	}

	std::string name_lower = object->name();
	std::transform(name_lower.begin(), name_lower.end(), name_lower.begin(), [](unsigned char c) {
		return std::tolower(c);
	});
	if (name_lower.contains(m_searchLower)) {
		return true;
	}

	std::string type_lower = object->type();
	std::transform(type_lower.begin(), type_lower.end(), type_lower.begin(), [](unsigned char c) {
		return std::tolower(c);
	});
	if (type_lower.contains(m_searchLower)) {
		return true;
	}

	return false;
}

bool Hierarchy::ShouldShow(toast::Object* object) {
	if (MatchesQuery(object)) {
		return true;
	}
	for (auto& child : object->children.GetAll() | std::views::values) {
		if (child->base_type() == toast::ActorT && !m_showActors) {
			continue;
		}
		if (child->base_type() == toast::ComponentT && !m_showComponents) {
			continue;
		}
		if (child->base_type() == toast::SceneT && !m_showScenes) {
			continue;
		}
		if (!m_showDisabled && !child->enabled()) {
			continue;
		}

		if (ShouldShow(child.get())) {
			return true;
		}
	}
	return false;
}

void Hierarchy::ObjectTree(Object* object, ImGuiTreeNodeFlags flags) {
	if (!m_showDisabled && !object->enabled()) {
		return;
	}
	if (!m_search.empty() && !ShouldShow(object)) {
		return;
	}
	assert(object == object->children.parent());

	ImGui::PushID(object->id());
	ImGuiTreeNodeFlags new_flags = flags;
	if (!m_search.empty()) {
		new_flags |= ImGuiTreeNodeFlags_DefaultOpen;
	}
	if (object->id() == m_selectedId) {
		m_selectedObject = object;
		new_flags |= ImGuiTreeNodeFlags_Selected;
	}
	if (!object->children.size()) {
		new_flags |= ImGuiTreeNodeFlags_Bullet;
	}    // u can use leaf if you want nothing to appear
	bool renaming = m_isRenaming && object->id() == m_selectedId;
	bool enabled = object->enabled();
	RenamingMenu(object, renaming);
	std::string imgui_name;
	if (object->base_type() == toast::SceneT) {
		imgui_name = std::format("{0}: {1}", object->id(), object->name());
	} else {
		imgui_name = object->name();
	}

	// highlight gray if disabled
	if (!enabled) {
		ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::ColorPallet[EditorColText2]);
	}

	// Always create the tree node, but store whether it is open
	float available_width = ImGui::GetContentRegionAvail().x - 100;
	ImGui::PushItemWidth(available_width);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 2.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

	bool name_match = !m_search.empty() && object->name().contains(m_search);
	bool type_match = !m_search.empty() && std::string(object->type()).contains(m_search);

	if (name_match) {
		ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::ColorPallet[EditorColX]);
	} else if (type_match) {
		ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::ColorPallet[EditorColZ]);
	}

	bool open = ImGui::TreeNodeEx(imgui_name.c_str(), new_flags, "%s", renaming ? "" : imgui_name.c_str());

	if (name_match || type_match) {
		ImGui::PopStyleColor(1);
	}

	ImGui::PopStyleVar(2);

	if (ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1)) {
		SelectObject(object);
	}
	if (open) {
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
			m_isRenaming = true;
			m_renameBuffer.clear();
		}
	}

	if (!dynamic_cast<physics::Rigidbody*>(object) && ImGui::BeginDragDropSource()) {
		// HACK: we need to design our engine next year to work in completly invalid states since its impossible to prevent Rigidbody from being in a
		// invalid state with moving
		ImGui::SetDragDropPayload("Object", &object, sizeof(nullptr));
		ImGui::EndDragDropSource();
	}
	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Object")) {
			auto* dropped = *reinterpret_cast<toast::Object**>(payload->Data);
			// object->Adopt(dropped->id());

			m_adoptable = std::pair(object, dropped->id());
		} else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("RESOURCE_STRUCT")) {
			auto* dropped = reinterpret_cast<editor::ResourceSlot::Entry*>(payload->Data);
			if (dropped->extension == ".prefab") {
				json_t json;
				auto file = resource::Open(dropped->relativePath.string());
				if (file.has_value()) {
					json = json_t::parse(file.value());
					object->children.Add(json["type"].get<std::string>(), std::nullopt, json)->_LoadTextures();
				}
			}
		}
		ImGui::EndDragDropTarget();
	}

	// Right Click Context Menu
	if (ImGui::BeginPopupContextItem("##ObjectContext")) {
		SelectObject(object);
		if (ImGui::BeginMenu("Create")) {
			// Scenes Can Be a Child of a Scene Only
			if (ImGui::BeginMenu("Scene")) {
				int id = CreateSceneMenuItem(object->scene());
				if (id != -1) {
					SelectObject(id);
					m_isRenaming = true;
				}
				ImGui::EndMenu();    // Scene
			}
			if (ImGui::BeginMenu("Actor")) {
				toast::Object* parent = object;
				unsigned int id = CreateActorMenuItem(parent);
				if (id != -1) {
					SelectObject(id);
					m_isRenaming = true;
				}
				ImGui::EndMenu();    // Actor
			}
			if (ImGui::BeginMenu("Component")) {
				toast::Object* parent = object;
				unsigned int id = CreateComponentMenuItem(parent);
				if (id != -1) {
					SelectObject(id);
				}
				ImGui::EndMenu();    // Component
			}
			ImGui::EndMenu();
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Rename")) {
			m_isRenaming = true;
		}
		if (ImGui::MenuItem("Delete")) {
			ActionStack::DeleteObject(object);
			SelectObject(-1);
		}
		if (ImGui::MenuItem("Focus")) {
			auto* camera = EditorCamera::GetInstance();
			if (auto* t = dynamic_cast<toast::TransformComponent*>(object)) {
				auto pos = t->position();
				pos.z = camera->transform()->position().z;
				camera->transform()->position(pos);
			} else if (auto* actor = dynamic_cast<toast::Actor*>(object)) {
				auto pos = actor->transform()->position();
				pos.z = camera->transform()->position().z;
				camera->transform()->position(pos);
			}
		}
		if (ImGui::MenuItem("Save As Prefab")) {
			auto json = object->Save();
			resource::ResourceManager::SaveFile(std::format("/PREFABS/{}.prefab", object->name()), json.dump(2));
		}
		ImGui::EndMenu();
	}

	// Remove Button
	ImGui::SameLine();
	float x_width = 20;
	float x_align = ImGui::GetWindowContentRegionMax().x + ImGui::GetWindowPos().x;
	float x_pos = x_align - x_width - ImGui::GetStyle().WindowPadding.x;
	ImGui::SetCursorPosX(x_pos - ImGui::GetWindowPos().x);
	if (ImGui::Button("X", ImVec2(20, 20))) {
		ActionStack::DeleteObject(object);
	}

	if (open) {
		std::unordered_set<std::string> names;
		for (auto& child : object->children.GetAll() | std::views::values) {
			if (child->base_type() == toast::ActorT && !m_showActors) {
				continue;
			}
			if (child->base_type() == toast::ComponentT && !m_showComponents) {
				continue;
			}
			if (child->base_type() == toast::SceneT && !m_showScenes) {
				continue;
			}
			std::string lower_case_name = child.get()->name();
			std::transform(lower_case_name.begin(), lower_case_name.end(), lower_case_name.begin(), [](unsigned char c) {
				return tolower(c);
			});

			bool name_confict = names.contains(child->name());
			if (name_confict) {
				ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::ColorPallet[EditorColWarning]);
				warning = true;

				if (child->name().contains("_(")) {
					auto prefix = child->name().find("_(");
					auto name = child->name().substr(0, prefix);
					auto count = std::atoi(child->name().substr(prefix + 2, child->name().length() - 1).c_str());
					auto new_name(name);
					while (names.contains(new_name)) {
						new_name = name;
						auto new_name = name.append(std::format("_({})", std::to_string(++count)));
					}
					child->name(std::move(new_name));
				} else {
					std::string new_name = child->name();
					new_name.append("_(0)");
					child->name(std::move(new_name));
				}
			}
			ObjectTree(child.get(), flags);
			if (name_confict) {
				ImGui::PopStyleColor(1);
			}
			names.emplace(child->name());
		}

		ImGui::PopItemWidth();
		ImGui::TreePop();
	}
	if (!enabled) {
		ImGui::PopStyleColor(1);
	}    // pop gray highlight
	ImGui::PopID();
}

void Hierarchy::RenamingMenu(Object* object, bool renaming) {
	if (renaming) {
		if (m_renameBuffer.empty()) {
			m_renameBuffer = object->name();
		}
		auto input_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
		ImGui::SetKeyboardFocusHere();
		if (ImGui::InputText("##rename", &m_renameBuffer, input_flags)) {
			m_isRenaming = false;
			std::ranges::replace(m_renameBuffer, ' ', '_');
			object->name(m_renameBuffer.data());
			m_renameBuffer.clear();
		}
		ImGui::SameLine();
	} else if (!m_renameBuffer.empty()) {
		m_renameBuffer.clear();
	}
}

void Hierarchy::SelectObject(unsigned int id) {
	if (id == m_selectedId) {
		return;
	}
	m_isRenaming = false;
	m_selectedObject = toast::World::Get(id);
	m_selectedId = id;
}

void Hierarchy::SelectObject(toast::Object* object) {
	if (object->id() == m_selectedId) {
		return;
	}
	m_isRenaming = false;
	m_selectedObject = object;
	m_selectedId = object->id();
}
}
