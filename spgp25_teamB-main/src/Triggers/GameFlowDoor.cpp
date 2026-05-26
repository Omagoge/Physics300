#include "GameFlowDoor.hpp"

#include "Player/Player.hpp"
#include "Toast/Components/AtlasSpriteComponent.hpp"
#include "Toast/Log.hpp"
#include "Toast/Objects/Actor.hpp"
#include "Toast/Objects/Object.hpp"
#include "Triggers/GameFlowTrigger.hpp"

#ifdef TOAST_EDITOR
#include "imgui.h"
#include "imgui_stdlib.h"
#endif

namespace game {
void GameFlowDoor::Init() { }

void GameFlowDoor::Begin() {
	float player_level = 0.1;
	// m.gameFlow = children.Get<toast::Actor>("GameFlow");
	m.gLeft = children.Get<GameFlowTrigger>("gLeft");
	m.gRight = children.Get<GameFlowTrigger>("gRight");

	if (m.usePath) {
		m.gLeft->usePath = m.usePath;
		m.gRight->usePath = m.usePath;
		m.gLeft->path = m.path;
		m.gRight->path = m.path;
	}

	m.tLeft = children.Get<physics::Trigger>("tLeft");
	m.tMiddle = children.Get<physics::Trigger>("tMiddle");
	m.tRight = children.Get<physics::Trigger>("tRight");

	m.sLeft = children.Get<toast::AtlasSpriteComponent>("sLeft");
	m.sDoor = children.Get<toast::AtlasSpriteComponent>("sDoor");
	m.sRight = children.Get<toast::AtlasSpriteComponent>("sRight");
	m.aLeft = false;
	m.aRight = false;

	{
		glm::vec3 door = m.sDoor->position();
		glm::vec3 left = m.sLeft->position();
		glm::vec3 right = m.sRight->position();
		door.z = .089;
		left.z = 0.09;
		right.z = .09;
		m.sDoor->position(door);
		m.sLeft->position(left);
		m.sRight->position(right);
	}
	m.tLeft->enterCallback = [this](toast::Object* o) {
		if (auto* p = dynamic_cast<Player*>(o)) {
			if (not m.aLeft) {
				TOAST_INFO("Printed Left");
			}
			m.aLeft = true;
		}
	};
	m.tRight->enterCallback = [this](toast::Object* o) {
		if (auto* p = dynamic_cast<Player*>(o)) {
			if (not m.aRight) {
				TOAST_INFO("Printed Right");
			}
			m.aRight = true;
		}
	};
	m.tMiddle->enterCallback = [this](toast::Object* o) {
		if (auto* p = dynamic_cast<Player*>(o)) {
			if (m.aLeft) {
				glm::vec3 right = m.sRight->position();
				right.z = .15;
				m.sRight->position(right);
				m.gRight->enabled(true);
			}
			if (m.aRight) {
				glm::vec3 left = m.sLeft->position();
				left.z = .15;
				m.sLeft->position(left);
				m.gLeft->enabled(true);
			}
		}
	};
}

#ifdef TOAST_EDITOR
void GameFlowDoor::Inspector() {
	ImGui::Checkbox("Use Path", &m.usePath);
	ImGui::InputText("Path", &m.path);
}
#endif


json_t GameFlowDoor::Save() const {
	auto j = toast::Actor::Save();
	j["use_path"] = m.usePath;
	j["path"] = m.path;
	return j;
}

void GameFlowDoor::Load(json_t j, bool force_create) {
	toast::Actor::Load(j, force_create);
	if (j.contains("use_path")) {
		m.usePath = j["use_path"];
		m.path = j["path"];
	}
}
}
