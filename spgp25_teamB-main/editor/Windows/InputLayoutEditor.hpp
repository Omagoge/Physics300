/**
 * @file InputLayoutEditor.hpp
 * @author Dante Harper
 * @date 29/10/25
 *
 * @brief Input Layout Editor
 */

#pragma once

#include "Tools/PopupModal.hpp"
#include "nlohmann/json_fwd.hpp"

#include <Toast/Event/ListenerComponent.hpp>
#include <nlohmann/json.hpp>
#include <vector>

namespace editor {
class InputLayoutPopup {
public:
	void Show();

	void LoadLayoutList();
	void LoadLayoutJson();

	bool enabled = false;

private:
	void ShowSettings();
	void ShowActions(nlohmann::json& action);
	void ShowActionConfig(nlohmann::json& action);
	void ShowActionTypes(nlohmann::json& action);
	void ShowActionBinds(nlohmann::json& binds, nlohmann::json& action);
	void ShowActionBindTypes(nlohmann::json& bind, nlohmann::json& action);
	void ShowActionBindButtons(nlohmann::json& bind, nlohmann::json& action);

	void ModalCallback(nlohmann::json& json);

	nlohmann::json m_json;

	event::ListenerComponent m_listener;
	PopupModal m_inputModal;

	std::string m_currentLayout;
	std::vector<std::string> m_layoutPaths;

	int m_pressed = 0;
	int m_mods = 0;

	// TODO: mouseScroll

	const std::array<std::string, 3> m_inputDimensions = { "button", "axis1D", "axis2D" };
	const std::array<std::string, 4> m_buttonActions = { "keyboard", "mouseButton", "controllerButton", "controllerTrigger" };
	const std::array<std::string, 2> m_2dActions = { "controllerStick", "composite2D" };
	const std::array<std::string, 2> m_1dActions = { "composite1D", "mousePosition" };
};
}
