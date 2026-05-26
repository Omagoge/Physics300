/**
 * @file SettingsEditor.hpp
 * @author Dante Harper
 * @date 29/10/25
 *
 * @brief Settings Editor
 */

#pragma once

#include "nlohmann/json.hpp"

namespace editor {
class SettingsPopup {
public:
	void Show();

	bool enabled = false;

private:
	void LoadJson();

	nlohmann::json m_json;
};
}
