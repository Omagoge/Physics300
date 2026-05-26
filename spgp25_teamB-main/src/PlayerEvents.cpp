//
// Created by akaan on 10/11/2025.
//

#include "PlayerEvents.hpp"

#include <Toast/Objects/Actor.hpp>
#include <Toast/Objects/Scene.hpp>
#include <Toast/World.hpp>

void PlayerEvents::Begin() {
	Subscribe<game::GameEvent>("PlayerListener", [this](game::GameEvent* e) -> bool {
		return Route(e);
	});

	Register("Impulse", [this](game::GameEvent* e) {
		return Impulse(e);
	});
	Register("Death", [this](game::GameEvent* e) {
		return Death(e);
	});
	Register("Checkpoint", [](game::GameEvent* e) {
		return true;
	});
	Register("LevelChange", [this](game::GameEvent* e) {
		return ChangeLevel(e);
	});
}

void PlayerEvents::Register(const std::string& tag, Handler fn) {
	m_handlers[tag] = std::move(fn);
}

bool PlayerEvents::Route(game::GameEvent* e) {
	if (!e) {
		return false;
	}
	auto it = m_handlers.find(e->tag);
	if (it == m_handlers.end()) {
		return false;
	}
	return it->second(e);
}

// Hard coded Impulse - for more options use scene trigger
bool PlayerEvents::Impulse(game::GameEvent* e) {
	return true;
}

// Set player health to 0
bool PlayerEvents::Death(game::GameEvent* e) {
	auto* actor = dynamic_cast<toast::Actor*>(e->other);
	if (!actor) {
		return false;
	}

	// TODO: Rework this, health components are now deprecated...
	// game::HealthComponent* hp = actor->children.Get<game::HealthComponent>();
	// hp->health(0);
	return true;
}

// Just returns the positon of the player for now
// When we get checkpoints we can set checkpoint or smth
glm::vec3 PlayerEvents::SetCheckpoint(game::GameEvent* e) {
	auto* actor = dynamic_cast<toast::Actor*>(e->other);
	toast::TransformComponent* T = actor->children.Get<toast::TransformComponent>();
	return T->worldPosition();
}

// For now it is hardcoded to set to Demo_Level3
// bc i wasn't sure if we need to serialize variables
// bc then it becomes more like the scene trigger
bool PlayerEvents::ChangeLevel(game::GameEvent* e) {
	const std::string scene_path = "SCENES/Demo_Level3.scene";
	toast::World::LoadScene(scene_path);
	toast::World::EnableScene(scene_path);
	toast::World::DisableScene(scene()->id());
	return true;
}
