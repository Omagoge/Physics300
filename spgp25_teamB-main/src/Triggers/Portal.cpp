#include "Portal.hpp"

#include "Localization.hpp"
#include "Player/Player.hpp"
#include "Stats.hpp"
#include "Toast/Components/MeshRendererComponent.hpp"
#include "Toast/CoroutineHandler.hpp"
#include "Toast/GameEvents.hpp"
#include "Toast/GameFlow.hpp"
#include "Toast/Input/InputListener.hpp"
#include "Toast/Renderer/HUD/HUDLayer.hpp"
#include "Toast/WaitAsync.hpp"
#include "Toast/World.hpp"
#ifdef TOAST_EDITOR
#include "imgui.h"
#include "imgui_stdlib.h"
#endif

namespace game {

#ifdef TOAST_EDITOR
void Portal::Inspector() {
	physics::Trigger::Inspector();
	ImGui::InputInt("World", &m.world);
	ImGui::InputInt("Level", &m.level);
	ImGui::InputText("Path", &m.path);
	ImGui::InputText("Previous Level", &m.prevLevel);
	ImGui::Checkbox("Active", &m.active);
	ImGui::Checkbox("Always Active", &m.alwaysActive);
	ImGui::Checkbox("Use Path Instead", &m.usePathInstead);
	ImGui::Checkbox("PLAYER COLLIDING", &m.playerIsColliding);
	ImGui::DragFloat2("Dante scale", &scale_for_dante.x);
}
#endif

void Portal::Init() {
	if (not m.usePathInstead) {
		m.hud = children.AddRequired<HUDActor>("hud");
		m.hud->SetUrl("file:///assets/UI/billboard.html");
		m.hud->SetDimestion(2383, 1280);
		auto mesh = children.AddRequired<toast::MeshRendererComponent>();
		mesh->enabled(false);
	}
	m.input.Subscribe0D("accept", [this](const input::Action0D* a) {
		if (not enabled()) {
			return;
		}
		if (not m.playerIsColliding) {
			return;
		}
		if (m.locked) {
			return;
		}

		m.playerIsColliding = false;
		m.active = false;
		// enabled(false);
		[](auto& t) -> toast::CoroutineTask {
			renderer::HUD::HUDLayer::Get()->ExecuteJS("fadeIn()");
			co_await toast::WaitSeconds(0.75f);
			if (t.m.usePathInstead) {
				toast::World::LoadSceneSync(t.m.path);
			} else {
				event::Send(new toast::LoadLevel(t.m.world, t.m.level));
			}
			t.scene()->Nuke();
			co_await toast::WaitSeconds(0.25f);
			renderer::HUD::HUDLayer::Get()->ExecuteJS("fadeOut()");
		}(*this);
	});
}

void Portal::Begin() {
	Trigger::Begin();

	m.active = true;
	if (m.hud) {
		std::string language = "en";
		if (Stats::stats.contains("language") && Stats::stats["language"].is_string()) {
			language = Stats::stats["language"].get<std::string>();
		}
		language = Localization::NormalizeLanguage(language);
		if (!Localization::IsSupportedLanguage(language)) {
			language = "en";
		}
		const auto languageMap = Localization::BuildLanguageMap(language);
		m.hud->ExecuteJS(
		    std::format("if(typeof setLocalizationData==='function') setLocalizationData({}, {});", languageMap.dump(), json_t(language).dump())
		);

		std::string path = toast::GameFlow::GetLevelName(m.world, m.level);
		auto file = resource::Open(path);
		std::string world_str = std::format("{}:", m.level);
		if (file.has_value()) {
			auto json = json_t::parse(file.value());
			std::string name = json["sceneName"];
			m.hud->ExecuteJS(std::format("setName('{} {}')", world_str, name));
		} else {
			m.hud->ExecuteJS(std::format("setName('{} {}')", world_str, "level"));
		}

		m.hud->ExecuteJS("setIsSelected(false)");

		if (m.level == 0) {
			m.hud->ExecuteJS("setIsUnlocked(true)");
			m.locked = false;
		} else if (Stats::stats["enabled"][m.world][m.level] == true) {
			m.hud->ExecuteJS("setIsUnlocked(true)");
			m.locked = false;
		} else if (Stats::stats["enabled"][m.world][m.level - 1] == true) {
			m.hud->ExecuteJS("setIsUnlocked(true)");
			m.locked = false;
		} else {
			m.hud->ExecuteJS("setIsUnlocked(false)");
			m.locked = true;
		}

		path = path.substr(9);
		path = path.substr(0, path.length() - 6);
		if (Stats::stats["best_time"].contains(path)) {
			m.hud->ExecuteJS("setIsPlayed(true)");
			int milli = Stats::stats["best_time"][path];
			m.hud->ExecuteJS(std::format("setTimer({})", milli));
		} else {
			m.hud->ExecuteJS("setIsPlayed(false)");
		}
	}
}

void Portal::Load(json_t j, bool force_create) {
	physics::Trigger::Load(j, force_create);
	if (j.contains("world") || j.contains("level")) {
		m.world = j["world"].get<unsigned>();
		m.level = j["level"].get<unsigned>();
	}
	if (j.contains("path_instead")) {
		m.usePathInstead = j["path_instead"].get<bool>();
	}
	if (j.contains("alwaysActive")) {
		m.alwaysActive = j["alwaysActive"].get<bool>();
	}
	if (j.contains("path")) {
		m.path = j["path"].get<std::string>();
	}
	if (j.contains("prevLevel")) {
		m.prevLevel = j["prevLevel"].get<std::string>();
	}
}

[[nodiscard]]
json_t Portal::Save() const {
	json_t json = physics::Trigger::Save();
	// json["name"] = m.name;
	json["world"] = m.world;
	json["level"] = m.level;
	json["path_instead"] = m.usePathInstead;
	json["alwaysActive"] = m.alwaysActive;
	json["path"] = m.path;
	json["prevLevel"] = m.prevLevel;
	return json;
}

void Portal::OnEnter(toast::Object* obj) {
	if (!dynamic_cast<Player*>(obj) || !m.active) {
		return;
	}

	m.playerIsColliding = true;
	m.hud->ExecuteJS("setIsSelected(true)");
}

void Portal::OnExit(toast::Object* obj) {
	if (!dynamic_cast<Player*>(obj) || !m.active) {
		return;
	}
	m.playerIsColliding = false;
	m.hud->ExecuteJS("setIsSelected(false)");
}
}
