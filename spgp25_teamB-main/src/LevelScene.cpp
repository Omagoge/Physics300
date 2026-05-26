//
// Created by akaansh on 26/01/2026.
//

#include "LevelScene.hpp"

#include "Player/GameplayManager.hpp"
#include "Stats.hpp"
#include "Toast/GameFlow.hpp"
#include "Toast/Renderer/OclussionVolume.hpp"

#include <Toast/Components/HtmlView.hpp>
#include <Toast/CoroutineHandler.hpp>
#include <Toast/WaitAsync.hpp>
#include <Toast/World.hpp>
#include <string>
#ifdef TOAST_EDITOR
#include <imgui.h>
#include <imgui_stdlib.h>
#endif

void LevelScene::Begin() {
	if (toast::GameFlow::GetWorld() && toast::GameFlow::GetLevel()) {
		// HACK: this is a hack :P
		auto level_id = *toast::GameFlow::GetLevel();
		auto world_id = *toast::GameFlow::GetWorld();
		game::Stats::stats["enabled"][world_id][level_id] = true;
	}
	// // TODO: THIS IS TEMPORARY BC I DONT KNOW WHERE EXACTLY TO ACTUALLY CALL THISS

	// tf is this -x
	// static bool initialized = false;
	// if (!initialized) {
	//	event::Send(new toast::LoadWorld(0));
	//	initialized = true;
	//}
	//

	game::Stats::UnlockLevel(scene()->name());
	game::Stats::Save();
	if (auto* cam = GameplayManager::GetFollowCamera()) {
		cam->SetCameraBounds(camMinBounds, camMaxBounds);
		cam->transform()->worldPosition(glm::vec3(playerSpawn.x, playerSpawn.y, 15.0f));
	}
	if (auto* p = GameplayManager::GetPlayer()) {
		p->enabled(true);
		p->Respawn(glm::vec3(playerSpawn.x, playerSpawn.y, 0));
	}

	[](LevelScene& self) -> toast::CoroutineTask {
		auto& title = *toast::World::GetChildren().Add<toast::HtmlView>("Title");
		title.SetUrl("file:///assets/UI/text_effect.html");
		co_await toast::WaitSeconds(0.6);
		title.EvalJS(std::format("setText(\"{}\")", self.sceneName));
		title.EvalJS("fadeTextIn()");

		co_await toast::WaitSeconds(3.6);
		title.EvalJS("fadeTextOut()");
		co_await toast::WaitSeconds(1);
		title.Nuke();
	}(*this);
}

void LevelScene::LateTick() {
	Scene::LateTick();

	//TODO out of bounds check stuff... (The below is like mega dumb and does not work properly)

	// Restart if we are like fucking out of boundsd or something
	//const glm::vec2 player_pos = GameplayManager::GetPlayer()->transform()->position();
	// glm::vec2 clamped = glm::clamp(player_pos, camMinBounds, camMaxBounds);
	// bool inside = (clamped == player_pos);
	//
	// if (!inside) {
	// 	Restart();
	// }

	// const glm::vec3 player_pos = GameplayManager::GetPlayer()->transform()->position();
	// if (!OclussionVolume::isSphereOnPlanes(player_pos, 5.f)) {
	// 	Restart();
	// }

}

#ifdef TOAST_EDITOR
void LevelScene::EditorTick() {
	Scene::EditorTick();

	renderer::DebugLine(glm::vec2(camMinBounds.x, -100000.f), glm::vec2(camMinBounds.x, 100000.f), { 1, 0, 0, 1 });
	renderer::DebugLine(glm::vec2(camMaxBounds.x, -100000.f), glm::vec2(camMaxBounds.x, 100000.f), { 1, 0, 0, 1 });
	renderer::DebugLine(glm::vec2(-100000.f, camMinBounds.y), glm::vec2(100000.f, camMinBounds.y), { 1, 0, 0, 1 });
	renderer::DebugLine(glm::vec2(-100000.f, camMaxBounds.y), glm::vec2(100000.f, camMaxBounds.y), { 1, 0, 0, 1 });
	renderer::DebugCircle(playerSpawn, 1.f, { 0, 1, 0, 1 }, 16, true);
}
#endif

#ifdef TOAST_EDITOR
void LevelScene::Inspector() {
	ImGui::SeparatorText("Level Settings");
	ImGui::InputFloat2("Minimum Camera Bounds", &camMinBounds.x);
	ImGui::InputFloat2("Maximum Camera Bounds", &camMaxBounds.x);
	ImGui::InputFloat2("Player Spawn Position", &playerSpawn.x);
	ImGui::InputText("Scene Name", &sceneName);
}
#endif

void LevelScene::Load(json_t j, bool force_create) {
	Scene::Load(j, force_create);
	// Cam Boundz
	if (j.contains("camMin")) {
		camMinBounds = { j["camMin"][0].get<float>(), j["camMin"][1].get<float>() };
	}
	if (j.contains("camMax")) {
		camMaxBounds = { j["camMax"][0].get<float>(), j["camMax"][1].get<float>() };
	}
	// Player Spawn
	if (j.contains("playerSpawn")) {
		playerSpawn = {
			j["playerSpawn"][0].get<float>(),
			j["playerSpawn"][1].get<float>(),
		};
	}

	if (j.contains("sceneName")) {
		sceneName = j["sceneName"];
	}
}

json_t LevelScene::Save() const {
	json_t j = Scene::Save();
	j["camMin"] = { camMinBounds.x, camMinBounds.y };
	j["camMax"] = { camMaxBounds.x, camMaxBounds.y };
	j["playerSpawn"] = { playerSpawn.x, playerSpawn.y };
	j["sceneName"] = sceneName;
	return j;
}
