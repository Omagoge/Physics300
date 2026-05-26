/**
 * @file StatusBar.hpp
 * @author Dante Harper
 * @date 20/10/25
 *
 * @brief Status Bar for the editor
 */

#pragma once
#include "Tools/DebugStats.hpp"
#include "Tools/TextModal.hpp"

#include <imgui.h>

namespace editor {
class StatusBar {
public:
	void Init();
	void Show();

	bool ruler = false;

private:
	void FileMenu();
	void AppearanceMenu();
	void WindowMenu();

	void FontSelector();
	void ChooseFont(unsigned int idx);

	bool m_playMode = false;
	bool m_imguiDemo = false;

	DebugStats m_debugStats;
	TextModal m_textMenu;

	unsigned int m_currentFont = 0;
	std::vector<std::string> m_fonts;
	std::vector<ImFont*> m_imguiFonts;

	std::list<std::string> m_scenesBeforePlay;
};
}
