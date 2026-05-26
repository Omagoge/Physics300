//
// Created by akaan on 10/11/2025.
//

#ifndef PLAYER_EVENTS_HPP
#define PLAYER_EVENTS_HPP
#include "GameEvent.hpp"
#include "glm/vec3.hpp"

#include <Toast/Event/ListenerComponent.hpp>

class PlayerEvents : public event::ListenerComponent {
public:
	void Begin() override;

private:
	using Handler = std::function<bool(game::GameEvent*)>;
	std::unordered_map<std::string, Handler> m_handlers;

	void Register(const std::string& tag, Handler fn);
	bool Route(game::GameEvent* e);

	bool Impulse(game::GameEvent* e);
	bool Death(game::GameEvent* e);
	glm::vec3 SetCheckpoint(game::GameEvent* e);
	bool ChangeLevel(game::GameEvent* e);
};

#endif    // PLAYER_EVENTS_HPP
