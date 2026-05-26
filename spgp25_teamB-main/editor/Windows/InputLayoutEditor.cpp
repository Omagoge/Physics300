#include "InputLayoutEditor.hpp"

#include "nlohmann/json_fwd.hpp"

#include <Toast/Log.hpp>
#include <Toast/Window/WindowEvents.hpp>
#include <imgui.h>
#include <imgui_stdlib.h>

namespace editor {

void InputLayoutPopup::LoadLayoutList() {
	std::ifstream f("assets/project_settings.toast");
	if (!f.is_open()) {
		TOAST_WARN("Unable to Open Settings Menu File assets/project_settings.toast does not exist");
		enabled = false;
		return;
	}
	auto settings = nlohmann::json::parse(f);
	if (settings["format"] != "projectData") {
		TOAST_WARN("Wrong File Format Expected:projectData got: {}", (std::string)m_json["format"]);
		enabled = false;
	}
	m_layoutPaths = settings["input"]["layouts"];
	settings.clear();
	if (m_layoutPaths.empty()) {
		CLIENT_WARN("THERE ARE NO INPUT LAYOUT PATHS");
		enabled = false;
		return;
	}
	m_currentLayout = (m_currentLayout.empty()) ? m_layoutPaths[0] : m_currentLayout;
}

void InputLayoutPopup::LoadLayoutJson() {
	m_json.clear();
	if (std::find(m_layoutPaths.begin(), m_layoutPaths.end(), m_currentLayout) == m_layoutPaths.end()) {
		m_currentLayout = m_layoutPaths[0];
	}
	std::ifstream f(std::format("assets/{}", m_currentLayout));
	if (!f.is_open()) {
		TOAST_WARN("Unable to Open Input Layout {} does not exist", std::format("assets/{}", m_currentLayout));
		enabled = false;
		return;
	}
	m_json = nlohmann::json::parse(f);
	if (m_json["format"] != "inputLayout") {
		TOAST_WARN("Wrong File Format Expected:inputLayout got: {}", (std::string)m_json["format"]);
		enabled = false;
	}
}

void InputLayoutPopup::Show() {
	if (enabled) {
		m_inputModal.Show();
		if (ImGui::Begin("Input Layout", &enabled)) {
			if (m_json.empty()) {
				LoadLayoutList();
				LoadLayoutJson();
			}
			ShowSettings();

			ImGui::SeparatorText("Actions");
			for (auto& action : m_json["actions"]) {
				ShowActions(action);
			}

			if (ImGui::Button("Add Action")) {
				m_json["actions"].push_back(
				    {
				      {  "name",                  "null" },
              {  "type",                  "null" },
              { "binds", nlohmann::json::array() }
        }
				);
			}

			ImGui::Separator();
			if (ImGui::Button("Save")) {
				std::ofstream f(std::format("assets/{}", m_currentLayout));
				f << m_json;
				f.close();
			}
			ImGui::SameLine();
			if (ImGui::Button("Quit")) {
				enabled = false;
			}
			ImGui::End();
		} else if (!enabled && !m_json.empty()) {
			m_json.clear();
		}
	}
}

void InputLayoutPopup::ShowSettings() {
	if (ImGui::BeginCombo("Select Input Layout", m_currentLayout.c_str())) {
		for (const auto& path : m_layoutPaths) {
			if (ImGui::Selectable(path.c_str())) {
				m_currentLayout = path;
				LoadLayoutJson();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::Text("Om nom nom %s", m_json["name"].get<std::string>().c_str());
}

void InputLayoutPopup::ShowActions(nlohmann::json& action) {
	ImGui::PushID(&action);
	if (ImGui::BeginTable("InputLayoutsTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
		ImGui::TableSetupColumn("##", ImGuiTableColumnFlags_WidthFixed, 200);
		ImGui::TableNextRow();

		std::string dimension = action["type"];

		ShowActionConfig(action);
		ShowActionTypes(action);
		for (auto& bind : action["binds"]) {
			ShowActionBinds(bind, action);
		}

		ImGui::EndTable();
		if (ImGui::Button("Add Bind")) {
			action["binds"].push_back(
			    {
			      {  "type", "select type plz" },
			      { "input",            "null" },
      }
			);
		}
	}
	ImGui::PopID();
}

void InputLayoutPopup::ShowActionConfig(nlohmann::json& action) {
	std::string name = action["name"];
	ImGui::TableSetColumnIndex(0);
	ImGui::Text("Name: ");
	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(200);
	if (ImGui::InputText("##name", &name)) {
		action["name"] = name;
	}
}

void InputLayoutPopup::ShowActionTypes(nlohmann::json& action) {
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	std::string dimension = action["type"];
	ImGui::Text("Type: ");
	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(200);
	if (ImGui::BeginCombo("##action", dimension.c_str())) {
		for (const auto& type : m_inputDimensions) {
			if (ImGui::Selectable(type.c_str())) {
				dimension = type;
				action["type"] = type;
				for (auto& bind : action["binds"]) {
					bind["type"] = "select type plz";
				}
				for (auto& bind : action["binds"]) {
					if (dimension == "axis2D") {
						bind["input"] = { '1', '2', '3', '4' };
					} else if (dimension == "axis1D") {
						bind["input"] = { '1', '2' };
					} else if (dimension == "button") {
						bind["input"] = '1';
					}
				}
			}
		}
		ImGui::EndCombo();
	}
}

void InputLayoutPopup::ShowActionBindTypes(nlohmann::json& bind, nlohmann::json& action) {
	ImGui::TableSetColumnIndex(0);
	ImGui::SetNextItemWidth(200);
	std::string action_type = bind["type"];
	if (ImGui::BeginCombo("##type", action_type.c_str())) {
		if (action["type"] == "button") {
			for (const auto& type : m_buttonActions) {
				if (ImGui::Selectable(type.c_str())) {
					action_type = type;
					bind["type"] = type;
					bind["input"] = '1';
				}
			}
		} else if (action["type"] == "axis1D") {
			for (const auto& type : m_1dActions) {
				if (ImGui::Selectable(type.c_str())) {
					action_type = type;
					bind["type"] = type;
					bind["input"] = { '1', '2' };
				}
			}
		} else if (action["type"] == "axis2D") {
			for (const auto& type : m_2dActions) {
				if (ImGui::Selectable(type.c_str())) {
					action_type = type;
					bind["type"] = type;
					bind["input"] = { '1', '2', '3', '4' };
				}
			}
		}
		ImGui::EndCombo();
	}
}

void InputLayoutPopup::ShowActionBinds(nlohmann::json& bind, nlohmann::json& action) {
	ImGui::PushID(&bind);
	ImGui::TableNextRow();

	std::string action_type = bind["type"];

	ShowActionBindTypes(bind, action);
	ShowActionBindButtons(bind, action);

	ImGui::PopID();
}

void InputLayoutPopup::ShowActionBindButtons(nlohmann::json& bind, nlohmann::json& action) {
	ImGui::TableSetColumnIndex(1);
	auto& input = bind["input"];
	std::string action_type = bind["type"];

	if (action_type == "select type plz") {
		ImGui::Text("<-- select type plz");
	} else if (bind["type"] == "mousePosition") {
		ImGui::Text("Mouse Position");
	} else if (bind["type"] == "controllerStick") {
		if (!input.is_number()) {
			input = 0;
		}
		if (ImGui::BeginCombo("##LeftRight", (!input.get<int>()) ? "Left" : "Right")) {
			if (ImGui::Selectable("Left")) {
				input = 0;
			}
			if (ImGui::Selectable("Right")) {
				input = 1;
			}
			ImGui::EndCombo();
		}
	} else if (action["type"] == "button") {
		if (!input.is_number()) {
			input = '0';
		}
		if (ImGui::Selectable(std::format("{}##num", (char)input.get<int>()).c_str(), false, 0, ImVec2(25, 25))) {
			ModalCallback(input);
		}
	} else if (input.is_array()) {
		if (action["type"] == "axis1D") {
			if (!input.is_array()) {
				input = { '1', '2' };
			}
			std::array<int, 2> array = input;
			if (ImGui::Selectable(std::format("{}##0", (char)array[0]).c_str(), false, 0, ImVec2(25, 25))) {
				ModalCallback(input[0]);
			}

			ImGui::SameLine();
			if (ImGui::Selectable(std::format("{}##1", (char)array[1]).c_str(), false, 0, ImVec2(25, 25))) {
				ModalCallback(input[1]);
			}

		} else if (action["type"] == "axis2D") {
			if (!input.is_array()) {
				input = { '1', '2', '3', '4' };
			}
			std::array<int, 4> array = input;
			if (ImGui::Selectable(std::format("{}##0", (char)array[0]).c_str(), false, 0, ImVec2(25, 25))) {
				ModalCallback(input[0]);
			}

			ImGui::SameLine();
			if (ImGui::Selectable(std::format("{}##1", (char)array[1]).c_str(), false, 0, ImVec2(25, 25))) {
				ModalCallback(input[1]);
			}

			ImGui::SameLine();
			if (ImGui::Selectable(std::format("{}##2", (char)array[2]).c_str(), false, 0, ImVec2(25, 25))) {
				ModalCallback(input[2]);
			}

			ImGui::SameLine();
			if (ImGui::Selectable(std::format("{}##3", (char)array[3]).c_str(), false, 0, ImVec2(25, 25))) {
				ModalCallback(input[3]);
			}
		}
	}
}

void InputLayoutPopup::ModalCallback(nlohmann::json& json) {
	CLIENT_INFO("Input Layout (press button modal)");
	m_pressed = 0;
	m_listener.Subscribe<event::WindowKey>(
	    [this](const event::WindowKey* e) -> bool {
		    m_pressed = e->key;
		    return false;
	    },
	    255
	);
	m_listener.Subscribe<event::WindowMouseButton>(
	    [this](const event::WindowMouseButton* e) -> bool {
		    m_pressed = e->button;
		    return false;
	    },
	    255
	);
	m_inputModal.Init("PRESS BUTTON", [this, &json]() {
		ImGui::Text("Press Button To Update Input Layout");
		if (m_pressed != 0) {
			json = m_pressed;
			m_listener.Unsubscribe<event::WindowKey>();
			m_listener.Unsubscribe<event::WindowMouseButton>();
			m_pressed = 0;
			return true;
		}
		return false;
	});
}
}
