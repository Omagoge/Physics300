//
// Created by akaan on 05/11/2025.
//

#ifndef GAME_EVENT_HPP
#define GAME_EVENT_HPP
// GameEvents.hpp
#pragma once
#include <Toast/Event/Event.hpp>

namespace game {
struct GameEvent : event::Event<GameEvent> {
	toast::Object* sender = nullptr;
	toast::Object* other = nullptr;
	std::string tag;

	GameEvent() = default;

	GameEvent(toast::Object* s, toast::Object* o, std::string t) : sender(s), other(o), tag(std::move(t)) { }
};
}    // namespace game

#endif    // GAME_EVENT_HPP
