#pragma once
#include "Player/Player.hpp"
#include "Toast/ISerializable.hpp"

namespace game {
class Stats {
	friend class Game;

public:
	static json_t stats;

	static void Save();

	static auto LoadStats() -> json_t;
	static void AddLevelStats(Player* player, const std::string& scene_name, int enemy_kills);
	static void UnlockLevel(const std::string& scene_name);
	static void BlockLevel(const std::string& scene_name);
};
}
