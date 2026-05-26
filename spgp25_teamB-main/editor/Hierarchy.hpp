#pragma once
#include "Toast/Input/InputListener.hpp"

#include <Toast/Objects/Object.hpp>
#include <glm/vec2.hpp>
#include <imgui.h>
#include <optional>

namespace editor {

class Hierarchy {
public:
	~Hierarchy();
	
	void Init();
	void Show();

	[[nodiscard]]
	toast::Object* SelectedObject() const {
		return m_selectedObject;
	}

	[[nodiscard]]
	unsigned int SelectedObjectId() const {
		return m_selectedId;
	}

	void SelectObject(unsigned int id);
	void SelectObject(toast::Object* object);

	bool windowSelected = false;
	bool warning = false;
  glm::vec2 lastMouseWorldPos = { 0.0f, 0.0f };

private:
	
	void OnMouse(glm::vec2 worldPos);
	
	bool MatchesQuery(toast::Object* object);
	bool ShouldShow(toast::Object* object);
	void ObjectTree(toast::Object* object, ImGuiTreeNodeFlags flags);
	void RenamingMenu(toast::Object* object, bool renaming);

	unsigned m_selectedId = -1;
	toast::Object* m_selectedObject = nullptr;
	bool m_isRenaming = false;
	std::string m_renameBuffer;
	bool m_showActors = true, m_showComponents = false, m_showScenes = true, m_showDisabled = false;
	std::string m_search, m_searchLower;
	std::optional<std::pair<toast::Object*, unsigned>> m_adoptable = std::nullopt;
	
	
	input::Listener* m_listener = nullptr;
	
};

}
