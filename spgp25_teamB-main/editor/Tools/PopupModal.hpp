/**
 * @file PopupModal.hpp
 * @author Dante Harper, Xein
 * @date 25/10/25
 *
 * @brief custom modal
 */

#pragma once

namespace editor {

class PopupModal {
public:
	void Init(std::string_view name, const std::function<bool()>& logic);
	void Show();

private:
	bool m_enabled = false;
	std::string m_menuName;
	std::function<bool()> m_logic;
	// std::string m_textBuffer;
	// std::function<bool()> m_callback;
};

}
