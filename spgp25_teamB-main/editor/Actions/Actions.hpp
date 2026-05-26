/**
 * @file Actions.hpp
 * @author Dante Harper
 * @date 12/11/25
 */

#pragma once

#include <Toast/Components/TransformComponent.hpp>
#include <Toast/Event/ListenerComponent.hpp>
#include <nlohmann/json.hpp>

namespace editor {
class ActionStack {
public:
	static void Init();

	static void CopyObject(toast::Object* object);
	static void PasteObject();
	static unsigned int CreateObject(toast::Object* parent, std::string_view type);
	static void DeleteObject(toast::Object* object);
	static void GizmoObject(toast::TransformComponent* transform);
	static void SaveAll();

	static void PopAction();

private:
	static ActionStack* GetInstance() {
		static ActionStack action_list;
		return &action_list;
	}

	event::ListenerComponent m_listener;
	nlohmann::json m_copyBuffer;
	std::stack<nlohmann::json> m_actionStack;
};
}
