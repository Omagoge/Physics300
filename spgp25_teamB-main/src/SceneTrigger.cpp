//
// Created by xein on 11/3/25.
//

#include "SceneTrigger.hpp"

#include "GameEvent.hpp"
#include "Player/Player.hpp"

#include <Toast/Objects/Scene.hpp>
#include <Toast/Time.hpp>
#include <Toast/World.hpp>

#ifdef TOAST_EDITOR
#include "imgui.h"
#include "imgui_stdlib.h"
#endif

#include "FollowCamera.hpp"

#include <Toast/Components/TransformComponent.hpp>
#include <Toast/Event/Event.hpp>

namespace game {
void SceneTrigger::Init() {
	Actor::Init();
	m_activated = false;
}

void SceneTrigger::Begin() {
	Actor::Begin();

	m_activated = false;
	m_waiting = false;
	m_timerRemaining = 0.f;
	m_delayedFn = nullptr;
}

void SceneTrigger::Tick() {
	Actor::Tick();

	if (!m_waiting) {
		return;
	}

	m_timerRemaining -= static_cast<float>(Time::delta());
	if (m_timerRemaining <= 0.0f) {
		m_waiting = false;        // we're done waiting
		if (m_delayedFn) {        // if there's something to run
			m_delayedFn();          // run it
		}
		m_delayedFn = nullptr;    // clear
	}
}

// ---------- serialization helpers ----------
static inline const char* ToString(TriggerAction a) {
	switch (a) {
		case TriggerAction::None: return "None";
		case TriggerAction::LoadScene: return "LoadScene";
		case TriggerAction::EnableScene: return "EnableScene";
		case TriggerAction::DisableScene: return "DisableScene";
		case TriggerAction::SwitchScene: return "SwitchScene";
		case TriggerAction::Damage: return "Damage";
		case TriggerAction::AddImpulse: return "AddImpulse";
		case TriggerAction::FireEvent: return "FireEvent";
		case TriggerAction::TeleportPlayer: return "TeleportPlayer";
		case TriggerAction::PlaySound: return "PlaySound";
		case TriggerAction::CameraZoom: return "CameraZoom";
		default: return "None";
	}
}

static inline TriggerAction ActionFromString(const std::string& s) {
	if (s == "LoadScene") {
		return TriggerAction::LoadScene;
	}
	if (s == "EnableScene") {
		return TriggerAction::EnableScene;
	}
	if (s == "DisableScene") {
		return TriggerAction::DisableScene;
	}
	if (s == "SwitchScene") {
		return TriggerAction::SwitchScene;
	}
	if (s == "Damage") {
		return TriggerAction::Damage;
	}
	if (s == "AddImpulse") {
		return TriggerAction::AddImpulse;
	}
	if (s == "FireEvent") {
		return TriggerAction::FireEvent;
	}
	if (s == "TeleportPlayer") {
		return TriggerAction::TeleportPlayer;
	}
	if (s == "PlaySound") {
		return TriggerAction::PlaySound;
	}
	if (s == "CameraZoom") {
		return TriggerAction::CameraZoom;
	}
	return TriggerAction::None;
}

void SceneTrigger::Load(json_t j, bool force_create) {
	Actor::Load(j, force_create);

	// Mode
	if (j.contains("mode")) {
		if (std::string modeStr = j.at("mode").get<std::string>(); modeStr == "OnExit") {
			m_mode = TriggerMode::OnExit;
		} else {
			m_mode = TriggerMode::OnEnter;
		}
	} else {
		m_mode = TriggerMode::OnEnter;
	}

	if (j.contains("action")) {
		m_action = ActionFromString(j.at("action").get<std::string>());
	}
	if (j.contains("repeatable")) {
		m_repeatable = j.at("repeatable").get<bool>();
	}
	if (j.contains("delay")) {
		m_delay = j.at("delay").get<float>();
	}

	if (j.contains("nextSceneName")) {
		m_sceneName = j.at("nextSceneName").get<std::string>();
	}
	if (j.contains("impulseX")) {
		m_impulseX = j.at("impulseX").get<float>();
	}
	if (j.contains("impulseY")) {
		m_impulseY = j.at("impulseY").get<float>();
	}
	if (j.contains("eventName")) {
		m_eventName = j.at("eventName").get<std::string>();
	}
	if (j.contains("teleportPos")) {
		auto arr = j.at("teleportPos");
		if (arr.is_array() && arr.size() >= 2) {
			m_teleportPos = { arr[0].get<float>(), arr[1].get<float>() };
		}
	}
	if (j.contains("damage")) {
		m_damage = j.at("damage").get<float>();
	}
	if (j.contains("sfxName")) {
		m_sfxName = j.at("sfxName").get<std::string>();
	}
	if (j.contains("zoomScale")) {
		m_zoomScale = j.at("zoomScale").get<float>();
	}
}

json_t SceneTrigger::Save() const {
	json_t j = Actor::Save();
	j["mode"] = (m_mode == TriggerMode::OnExit ? "OnExit" : "OnEnter");
	j["action"] = ToString(m_action);
	j["repeatable"] = m_repeatable;
	j["delay"] = m_delay;

	// Only write fields that matter (optional; you can write all if you prefer)
	if (!m_sceneName.empty()) {
		j["nextSceneName"] = m_sceneName;
	}
	if (m_impulseX != 0 || m_impulseY != 0) {
		j["impulseX"] = m_impulseX;
		j["impulseY"] = m_impulseY;
	}
	if (!m_eventName.empty()) {
		j["eventName"] = m_eventName;
	}
	j["teleportPos"] = { m_teleportPos.x, m_teleportPos.y };
	if (!m_sfxName.empty()) {
		j["sfxName"] = m_sfxName;
	}
	if (m_damage != 0) {
		j["damage"] = m_damage;
	}
	if (m_zoomScale != 1.0f) {
		j["zoomScale"] = m_zoomScale;
	}
	return j;
}

#ifdef TOAST_EDITOR
void SceneTrigger::Inspector() {
	// Draw base Actor inspector (transform, enable, name, etc.)
	Actor::Inspector();

	ImGui::SeparatorText("Trigger");
	ImGui::SameLine();
	ImGui::Checkbox("Repeatable", &m_repeatable);
	ImGui::DragFloat("Delay (s)", &m_delay, 0.01f, 0.0f, 10.0f, "%.2f");

	const char* mode_options[] = { "OnEnter", "OnExit" };
	int mode_id = (m_mode == TriggerMode::OnExit) ? 1 : 0;
	if (ImGui::Combo("Mode", &mode_id, mode_options, IM_ARRAYSIZE(mode_options))) {
		m_mode = (mode_id == 1) ? TriggerMode::OnExit : TriggerMode::OnEnter;
	}

	// Action combo
	static const char* action_options[] = { "None",       "LoadScene", "EnableScene",    "DisableScene", "SwitchScene", "Damage",
		                                      "AddImpulse", "FireEvent", "TeleportPlayer", "PlaySound",    "CameraZoom" };
	int action_id = 0;
	// map enum -> index
	for (int i = 0; i < (int)(sizeof(action_options) / sizeof(action_options[0])); ++i) {
		if (std::string(action_options[i]) == ToString(m_action)) {
			action_id = i;
			break;
		}
	}
	if (ImGui::Combo("Action", &action_id, action_options, (int)(sizeof(action_options) / sizeof(action_options[0])))) {
		m_action = ActionFromString(action_options[action_id]);
	}

	// Per-action parameter UI
	switch (m_action) {
		case TriggerAction::LoadScene:
		case TriggerAction::EnableScene:
		case TriggerAction::DisableScene:
		case TriggerAction::SwitchScene: {
			ImGui::InputText("Scene Name", &m_sceneName);
		} break;

		case TriggerAction::AddImpulse: {
			ImGui::DragFloat("Impulse X", &m_impulseX, 0.1f);
			ImGui::DragFloat("Impulse Y", &m_impulseY, 0.1f);
		} break;

		case TriggerAction::FireEvent: {
			ImGui::InputText("Event Tag", &m_eventName);
		} break;

		case TriggerAction::TeleportPlayer: {
			ImGui::DragFloat2("Teleport Pos", &m_teleportPos.x, 0.1f);
		} break;

		case TriggerAction::PlaySound: {
			ImGui::InputText("SFX Name", &m_sfxName);
		} break;

		case TriggerAction::Damage: {
			ImGui::InputFloat("Damage", &m_damage);
		}
		case TriggerAction::CameraZoom: {
			ImGui::DragFloat("Zoom Scale", &m_zoomScale, 0.01f, 0.1f, 5.0f);
		} break;
		case TriggerAction::None:
		default: break;
	}
}
#endif

// I know exit and enter are the same which is redundant... but im tired and i thought it would be useful idk xD
void SceneTrigger::OnTriggerEnter(toast::Object* other) {
	if (!m_repeatable && m_activated) {
		return;
	}

	auto run = [this, other]() {
		Execute(other);
	};

	if (m_delay > 0.0f) {
		// If a delay is already running:
		if (m_waiting) {
			if (m_repeatable) {
				// restart the countdown (debounce)
				Delay(m_delay, run);
			} else {
				// non-repeatable and already pending → ignore
				return;
			}
		} else {
			// start a new countdown
			Delay(m_delay, run);
		}
	} else {
		// immediate
		run();
	}
}

void SceneTrigger::OnTriggerExit(toast::Object* other) {
	if (!m_repeatable && m_activated) {
		return;
	}

	auto run = [this, other]() {
		Execute(other);
	};

	if (m_delay > 0.0f) {
		// If a delay is already running:
		if (m_waiting) {
			if (m_repeatable) {
				// restart the countdown (debounce)
				Delay(m_delay, run);
			} else {
				// non-repeatable and already pending → ignore
				return;
			}
		} else {
			// start a new countdown
			Delay(m_delay, run);
		}
	} else {
		// immediate
		run();
	}
}

void SceneTrigger::Execute(toast::Object* other) {
	switch (m_action) {
		case TriggerAction::LoadScene: DoLoadScene(); break;
		case TriggerAction::EnableScene: DoEnableScene(); break;
		case TriggerAction::DisableScene: DoDisableScene(); break;
		case TriggerAction::SwitchScene: DoSwitchScenes(); break;
		case TriggerAction::Damage: DoDamagePlayer(other); break;
		case TriggerAction::AddImpulse: AddImpulse(other); break;
		case TriggerAction::FireEvent: FireEvent(other); break;
		case TriggerAction::TeleportPlayer: TeleportPlayer(other); break;
		case TriggerAction::PlaySound: PlaySound(); break;
		case TriggerAction::CameraZoom: DoCameraZoom(); break;
		case TriggerAction::None:
		default: break;
	}

	m_activated = true;
}

void SceneTrigger::Delay(float seconds, const std::function<void()>& fn) {
	if (seconds <= 0.0f) {
		fn();
		return;
	}

	m_timerRemaining = seconds;
	m_delayedFn = fn;
	m_waiting = true;
}

void SceneTrigger::DoLoadScene() const {
	if (m_sceneName.empty()) {
		return;
	}
	const std::string scene_path = "SCENES/" + m_sceneName + ".scene";
	toast::World::LoadScene(scene_path);
	// i was doing this but then everything kept crashing - for swicthing scenes instead of just loading
}

void SceneTrigger::DoEnableScene() const {
	if (m_sceneName.empty()) {
		return;
	}
	toast::World::EnableScene(m_sceneName);
}

void SceneTrigger::DoDisableScene() const {
	if (m_sceneName.empty()) {
		return;
	}
	toast::World::DisableScene(m_sceneName);
}

void SceneTrigger::DoSwitchScenes() const {
	if (m_sceneName.empty()) {
		return;
	}
	toast::World::EnableScene(m_sceneName);
	toast::World::DisableScene(scene()->id());
}

void SceneTrigger::DoDamagePlayer(toast::Object* player) const {
	// static_cast<Player*>(player)->OnDamage(m_damage);
}

void SceneTrigger::AddImpulse(toast::Object* player) const { }

void SceneTrigger::TeleportPlayer(toast::Object* player) const {
	toast::TransformComponent* transform_ = player->children.Get<toast::TransformComponent>();
	transform_->worldPosition(glm::vec3(m_teleportPos.x, m_teleportPos.y, -100));
}

void SceneTrigger::FireEvent(toast::Object* player) {
	if (m_eventName.empty()) {
		return;
	}
	event::Send(new game::GameEvent(this, player, m_eventName));
}

void SceneTrigger::PlaySound() const {
	if (m_sfxName.empty()) {
		return;
	}
	// TODO: call audio system play sound or smth
}

void SceneTrigger::DoCameraZoom() const {
	auto* cam = scene()->children.Get<FollowCamera>();
	if (!cam) {
		return;
	}
	cam->SetZoomScale(m_zoomScale);
}

}
