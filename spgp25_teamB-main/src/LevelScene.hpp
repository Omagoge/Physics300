//
// Created by akaansh on 26/01/2026.
//

#ifndef GAME_LEVELSCENE_HPP
#define GAME_LEVELSCENE_HPP
#include "FollowCamera.hpp"
#include "Player/Player.hpp"
#include "Toast/Objects/Scene.hpp"

#include <glm/vec2.hpp>

class LevelScene : public toast::Scene {
public:
	REGISTER_TYPE(LevelScene);

	glm::vec2 camMinBounds = glm::vec2(-10.0f, -10.0f);
	glm::vec2 camMaxBounds = glm::vec2(100.0f, 20.0f);

	glm::vec2 playerSpawn = glm::vec2(0.0f, 0.0f);
	std::string sceneName = "MEOW MEOW MEOW";

	void Begin() override;
	void LateTick() override;
#ifdef TOAST_EDITOR
	void EditorTick() override;
	void Inspector() override;
#endif

	[[nodiscard]]
	json_t Save() const override;
	void Load(json_t j, bool force_create = true) override;

private:
	FollowCamera* m_cam = nullptr;
	Player* m_player = nullptr;
};

#endif    // GAME_LEVELSCENE_HPP
