/**
 * @file theme.hpp
 * @author Dante Harper
 * @date 22/10/25
 *
 * @brief Managing Color Theme for the editor
 */

#pragma once
#include <imgui.h>

namespace editor {
enum EColor : char {
	EditorColPrimary1,       ///< UI highlights
	EditorColPrimary2,       ///< Hover backgrounds
	EditorColPrimary3,       ///< Panel backgrounds
	EditorColSecondary1,     ///< Window background
	EditorColSecondary2,     ///< Inactive UI areas
	EditorColAccent1,        ///< Main accent blue (more subtle)
	EditorColAccent2,        ///< Minor accents
	EditorColText1,          ///< Main text
	EditorColText2,          ///< Disabled/secondary text
	EditorColBackground1,    ///< Window background
	EditorColBackground2,    ///< Group panels
	EditorColBackground3,    ///< Inner panels
	EditorColBackground4,    ///< Transparent
	EditorColError,          ///< Errors
	EditorColWarning,        ///< Warnings
	EditorColSuccess,        ///< Success
	EditorColX,              ///< Transform axis X
	EditorColY,              ///< Transform axis Y
	EditorColZ,              ///< Transform axis Z
	EditorColCount
};

constexpr const char* EDITOR_COL_STRINGS[] = {
	"EditorCol_Primary1",    "EditorCol_Primary2",    "EditorCol_Primary3",    "EditorCol_Secondary1", "EditorCol_Secondary2",
	"EditorCol_Accent1",     "EditorCol_Accent2",     "EditorCol_Text1",       "EditorCol_Text2",      "EditorCol_Background1",
	"EditorCol_Background2", "EditorCol_Background3", "EditorCol_Background4", "EditorCol_Error",      "EditorCol_Warning",
	"EditorCol_Success",     "EditorCol_X",           "EditorCol_Y",           "EditorCol_Z"
};

class EditorTheme {
public:
	void LoadDefaultDark();
	void LoadDefaultLight();

	void ApplyAllToImgui();

	static std::array<ImVec4, EditorColCount> ColorPallet;    // TODO: make private and setter/getter probably

private:
	static ImVec4 RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
		constexpr float INV255 = 1.0f / 255.0f;
		return ImVec4 { r * INV255, g * INV255, b * INV255, a * INV255 };
	}
};
}
