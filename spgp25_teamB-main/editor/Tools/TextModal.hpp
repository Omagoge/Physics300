/**
 * @file TextModal.hpp
 * @author Dante Harper
 * @date 22/10/25
 *
 * @brief custom modal for prompting the user for text
 */

#pragma once

namespace editor {

class TextModal {
public:
	void Init(std::string_view name, std::string_view defaultText, const std::function<void(std::string&)>& callback);
	void Show();

private:
	bool m_enabled = false;
	std::string m_menuName;
	std::string m_textBuffer;
	std::function<void(std::string&)> m_callback;
};

}
