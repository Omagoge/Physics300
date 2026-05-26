//
// Created by akaan on 05/11/2025.
//

#include "GeneralTrigger.hpp"

#include "GameEvent.hpp"

// Optional editor UI
#ifdef TOAST_EDITOR
#include "imgui.h"
#include "imgui_stdlib.h"
#endif

using namespace game;

void GeneralTrigger::Init() {
	Actor::Init();
}

void GeneralTrigger::Begin() {
	Actor::Begin();
}

void GeneralTrigger::OnTriggerEnter(toast::Object* other) {
	if (m_mode == TriggerMode::OnEnter) {
		Execute(other);
	}
}

void GeneralTrigger::OnTriggerExit(toast::Object* other) {
	if (m_mode == TriggerMode::OnExit) {
		Execute(other);
	}
}

void GeneralTrigger::Execute(toast::Object* other) {
	if (!m_enabled) {
		return;
	}
	if (m_once && m_fired) {
		return;
	}

	m_fired = true;

	event::Send(new game::GameEvent(this, other, m_tag));
	OnTriggered(other);

	if (m_once) {
		m_enabled = false;
	}
}

// -------------------- Serialization --------------------

void GeneralTrigger::Load(json_t j, bool force_create) {
	if (j.contains("enabled")) {
		m_enabled = j["enabled"].get<bool>();
	}
	if (j.contains("once")) {
		m_once = j["once"].get<bool>();
	}
	if (j.contains("playerOnly")) {
		m_playerOnly = j["playerOnly"].get<bool>();
	}
	if (j.contains("tag")) {
		m_tag = j["tag"].get<std::string>();
	}

	if (j.contains("mode")) {
		const auto s = j["mode"].get<std::string>();
		m_mode = (s == "OnExit") ? TriggerMode::OnExit : TriggerMode::OnEnter;
	}
}

json_t GeneralTrigger::Save() const {
	json_t j = Actor::Save();
	j["enabled"] = m_enabled;
	j["once"] = m_once;
	j["playerOnly"] = m_playerOnly;
	j["tag"] = m_tag;
	j["mode"] = (m_mode == TriggerMode::OnExit) ? "OnExit" : "OnEnter";

	// Runtime (debug) – not necessary to serialize, so omit m_fired
	return j;
}

// -------------------- Editor --------------------
#ifdef TOAST_EDITOR
void GeneralTrigger::Inspector() {
	Actor::Inspector();

	ImGui::SeparatorText("General Trigger");

	int mode_id = (m_mode == TriggerMode::OnExit) ? 1 : 0;
	const char* modes[] = { "OnEnter", "OnExit" };
	if (ImGui::Combo("Mode", &mode_id, modes, 2)) {
		m_mode = (mode_id == 1) ? TriggerMode::OnExit : TriggerMode::OnEnter;
	}

	ImGui::Checkbox("Enabled", &m_enabled);
	ImGui::SameLine();
	ImGui::Checkbox("Fire Once", &m_once);
	ImGui::Checkbox("Player Only", &m_playerOnly);
	ImGui::InputText("Tag", &m_tag);

	ImGui::BeginDisabled();
	ImGui::Checkbox("Already Fired", &m_fired);
	ImGui::EndDisabled();
}
#endif
