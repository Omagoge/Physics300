#include "LoadLevelTrigger.hpp"

#include "LevelScene.hpp"
#include "Player/Player.hpp"
#include "Toast/Time.hpp"
#include "Toast/World.hpp"

#ifdef TOAST_EDITOR
#include "imgui.h"
#include "imgui_stdlib.h"
#endif

#include "Toast/GameEvents.hpp"

namespace game {

void LoadLevelTrigger::NextLevel() const {
	if (m_worldIndex < 0 || m_levelIndex < 0) {
		event::Send(new toast::NextLevel());
	} else {
		event::Send(new toast::LoadLevel(static_cast<unsigned>(m_worldIndex), static_cast<unsigned>(m_levelIndex)));
	}

	// OLD STUFF
	// const std::string scene_path = "SCENES/" + m_sceneName + ".scene";
	// toast::World::LoadScene(scene_path);
}

void LoadLevelTrigger::OnEnter(toast::Object* obj) {
	if (!dynamic_cast<Player*>(obj)) {
		return;
	}
	if (m_triggerMode == 0) {
		NextLevel();
	}
}

void LoadLevelTrigger::OnExit(toast::Object* obj) {
	if (!dynamic_cast<Player*>(obj)) {
		return;
	}
	if (m_triggerMode == 1) {
		NextLevel();
	}
}

void LoadLevelTrigger::Load(json_t j, bool force_create) {
	Trigger::Load(j, force_create);
	if (j.contains("triggerMode")) {
		m_triggerMode = j["triggerMode"].get<int>();
	}
	if (m_triggerMode != 0 && m_triggerMode != 1) {
		m_triggerMode = 0;    // for sanity ig
	}
	if (j.contains("worldIndex")) {
		m_worldIndex = j["worldIndex"].get<int>();
	}
	if (j.contains("levelIndex")) {
		m_levelIndex = j["levelIndex"].get<int>();
	}
}

json_t LoadLevelTrigger::Save() const {
	json_t j = Trigger::Save();
	j["triggerMode"] = m_triggerMode;
	j["worldIndex"] = m_worldIndex;
	j["levelIndex"] = m_levelIndex;
	return j;
}

#ifdef TOAST_EDITOR
void LoadLevelTrigger::Inspector() {
	physics::Trigger::Inspector();
	ImGui::SeparatorText("Trigger Mode");
	ImGui::RadioButton("On Enter", &m_triggerMode, 0);
	ImGui::RadioButton("On Exit", &m_triggerMode, 1);
	ImGui::SeparatorText("Manual Level Change");
	ImGui::TextDisabled("Set both to -1 to use Default NextLevel()");
	ImGui::DragInt("World Index", &m_worldIndex, 1, -1, 15);
	ImGui::DragInt("Level Index", &m_levelIndex, 1, -1, 15);
}
#endif

}    // namespace game
