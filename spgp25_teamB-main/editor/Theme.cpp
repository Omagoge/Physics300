#include "Theme.hpp"

namespace editor {

std::array<ImVec4, EditorColCount> EditorTheme::ColorPallet;

void EditorTheme::LoadDefaultDark() {
	ColorPallet[EditorColPrimary1] = RGBA(77, 77, 79);
	ColorPallet[EditorColPrimary2] = RGBA(70, 70, 77);
	ColorPallet[EditorColPrimary3] = RGBA(30, 30, 30);
	ColorPallet[EditorColSecondary1] = RGBA(20, 20, 20);
	ColorPallet[EditorColSecondary2] = RGBA(55, 55, 61);
	ColorPallet[EditorColAccent1] = RGBA(66, 150, 250);
	ColorPallet[EditorColAccent2] = RGBA(96, 115, 181);
	ColorPallet[EditorColText1] = RGBA(255, 255, 255);
	ColorPallet[EditorColText2] = RGBA(128, 128, 128);
	ColorPallet[EditorColBackground1] = RGBA(37, 37, 38);
	ColorPallet[EditorColBackground2] = RGBA(30, 30, 30);
	ColorPallet[EditorColBackground3] = RGBA(51, 51, 51);
	ColorPallet[EditorColBackground4] = RGBA(0, 0, 0);
	ColorPallet[EditorColError] = RGBA(219, 72, 115);      // Errors
	ColorPallet[EditorColWarning] = RGBA(213, 152, 87);    // Warnings
	ColorPallet[EditorColSuccess] = RGBA(174, 243, 87);    // Success
	ColorPallet[EditorColX] = RGBA(219, 72, 115);          // Transform axis X
	ColorPallet[EditorColY] = RGBA(174, 243, 87);          // Transform axis Y
	ColorPallet[EditorColZ] = RGBA(118, 162, 250);         // Transform axis Z
}

void EditorTheme::LoadDefaultLight() {
	ColorPallet[EditorColPrimary1] = RGBA(180, 180, 185);
	ColorPallet[EditorColPrimary2] = RGBA(160, 160, 170);
	ColorPallet[EditorColPrimary3] = RGBA(210, 210, 210);
	ColorPallet[EditorColSecondary1] = RGBA(225, 225, 225);
	ColorPallet[EditorColSecondary2] = RGBA(190, 190, 200);
	ColorPallet[EditorColAccent1] = RGBA(90, 140, 200);
	ColorPallet[EditorColAccent2] = RGBA(110, 110, 120);
	ColorPallet[EditorColText1] = RGBA(30, 30, 30);
	ColorPallet[EditorColText2] = RGBA(90, 90, 90);
	ColorPallet[EditorColBackground1] = RGBA(240, 240, 240);
	ColorPallet[EditorColBackground2] = RGBA(225, 225, 225);
	ColorPallet[EditorColBackground3] = RGBA(200, 200, 200);
	ColorPallet[EditorColBackground4] = RGBA(255, 255, 255);
	ColorPallet[EditorColError] = RGBA(219, 72, 115);
	ColorPallet[EditorColWarning] = RGBA(213, 152, 87);
	ColorPallet[EditorColSuccess] = RGBA(174, 243, 87);
	ColorPallet[EditorColX] = RGBA(219, 72, 115);
	ColorPallet[EditorColY] = RGBA(174, 243, 87);
	ColorPallet[EditorColZ] = RGBA(118, 162, 250);
}

void EditorTheme::ApplyAllToImgui() {
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg] = ColorPallet[EditorColBackground1];
	style.Colors[ImGuiCol_PopupBg] = ColorPallet[EditorColBackground2];
	style.Colors[ImGuiCol_Border] = ColorPallet[EditorColSecondary2];
	style.Colors[ImGuiCol_Header] = ColorPallet[EditorColPrimary3];
	style.Colors[ImGuiCol_HeaderHovered] = ColorPallet[EditorColPrimary2];
	style.Colors[ImGuiCol_HeaderActive] = ColorPallet[EditorColSecondary2];
	style.Colors[ImGuiCol_Button] = ColorPallet[EditorColPrimary3];
	style.Colors[ImGuiCol_ButtonHovered] = ColorPallet[EditorColPrimary1];
	style.Colors[ImGuiCol_ButtonActive] = ColorPallet[EditorColPrimary2];
	style.Colors[ImGuiCol_CheckMark] = ColorPallet[EditorColText1];
	style.Colors[ImGuiCol_SliderGrab] = ColorPallet[EditorColSecondary2];
	style.Colors[ImGuiCol_SliderGrabActive] = ColorPallet[EditorColAccent1];
	style.Colors[ImGuiCol_FrameBg] = ColorPallet[EditorColPrimary3];
	style.Colors[ImGuiCol_FrameBgHovered] = ColorPallet[EditorColPrimary1];
	style.Colors[ImGuiCol_FrameBgActive] = ColorPallet[EditorColPrimary2];
	style.Colors[ImGuiCol_Tab] = ColorPallet[EditorColBackground2];
	style.Colors[ImGuiCol_TabHovered] = ColorPallet[EditorColSecondary2];
	style.Colors[ImGuiCol_TabActive] = ColorPallet[EditorColSecondary2];
	style.Colors[ImGuiCol_TabSelectedOverline] = ColorPallet[EditorColAccent1];
	style.Colors[ImGuiCol_TabDimmedSelectedOverline] = ColorPallet[EditorColPrimary1];
	style.Colors[ImGuiCol_TabUnfocused] = ColorPallet[EditorColSecondary2];
	style.Colors[ImGuiCol_TabUnfocusedActive] = ColorPallet[EditorColSecondary2];
	style.Colors[ImGuiCol_TableRowBg] = ColorPallet[EditorColBackground2];
	style.Colors[ImGuiCol_TableRowBgAlt] = ColorPallet[EditorColBackground1];
	style.Colors[ImGuiCol_TitleBg] = ColorPallet[EditorColBackground2];
	style.Colors[ImGuiCol_TitleBgActive] = ColorPallet[EditorColBackground2];
	style.Colors[ImGuiCol_TitleBgCollapsed] = ColorPallet[EditorColBackground2];
	style.Colors[ImGuiCol_ScrollbarGrab] = ColorPallet[EditorColSecondary2];
	style.Colors[ImGuiCol_ResizeGrip] = ColorPallet[EditorColSecondary2];
	style.Colors[ImGuiCol_ResizeGripHovered] = ColorPallet[EditorColSecondary2];
	style.Colors[ImGuiCol_ResizeGripActive] = ColorPallet[EditorColSecondary2];
	style.Colors[ImGuiCol_Separator] = ColorPallet[EditorColPrimary2];
	style.Colors[ImGuiCol_SeparatorHovered] = ColorPallet[EditorColSecondary2];
	style.Colors[ImGuiCol_SeparatorActive] = ColorPallet[EditorColSecondary2];
	style.Colors[ImGuiCol_Text] = ColorPallet[EditorColText1];
	style.Colors[ImGuiCol_TextDisabled] = ColorPallet[EditorColText2];
	style.Colors[ImGuiCol_MenuBarBg] = ColorPallet[EditorColSecondary1];
}
}
