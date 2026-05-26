/**
 * @file MenuItems.hpp
 * @author Dante Harper
 * @date 27/10/25
 *
 * @brief Helper Functions for using ImGui::MenuItem
 */

#pragma once

namespace toast {
class Object;
}

namespace editor {

void LoadSceneMenuItem();
int SaveSceneMenuItem();

unsigned int CreateSceneMenuItem(toast::Object* parent);
unsigned int CreateActorMenuItem(toast::Object* parent);
unsigned int CreateComponentMenuItem(toast::Object* parent);

}
