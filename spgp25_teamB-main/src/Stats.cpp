#include "Stats.hpp"

#include "Player/Player.hpp"

namespace game {

json_t Stats::stats = Stats::LoadStats();

void Stats::Save() {
	std::ofstream out("assets/toast.data", std::ios::out | std::ios::trunc);
	out << stats.dump(2);
}

auto Stats::LoadStats() -> json_t {
	std::ifstream in("assets/toast.data");
	if (in) {
		return json_t::parse(in);
	}
	return json_t {};
}

void Stats::AddLevelStats(Player* player, const std::string& scene_name, int enemy_kills) {
	if (stats["best_time"].contains(scene_name)) {
		int best_time = stats["best_time"][scene_name];
		if (best_time > static_cast<int>(player->milliseconds * 1000)) {
			stats["best_time"][scene_name] = static_cast<int>(player->milliseconds * 1000);
			stats["air_time"][scene_name] = static_cast<int>(player->timeOnAir * 1000);
			stats["grav_time"][scene_name] = static_cast<int>(player->timeOnGravity * 1000);
		}
	} else {
		stats["best_time"][scene_name] = static_cast<int>(player->milliseconds * 1000);
		stats["air_time"][scene_name] = static_cast<int>(player->timeOnAir * 1000);
		stats["grav_time"][scene_name] = static_cast<int>(player->timeOnGravity * 1000);
	}

	if (stats["shot_number"].contains(scene_name)) {
		int number = stats["shot_number"][scene_name];
		number += player->shotNumber;
		stats["shot_number"][scene_name] = number;
	} else {
		stats["shot_number"][scene_name] = player->shotNumber;
	}

	if (stats["enemy_kills"].contains(scene_name)) {
		int number = stats["enemy_kills"][scene_name];
		if (number > enemy_kills) {
			stats["enemy_kills"][scene_name] = number;
		}
	} else {
		stats["enemy_kills"][scene_name] = enemy_kills;
	}

	if (stats["max_speed"].contains(scene_name)) {
		int speed = stats["max_speed"][scene_name];
		if(speed > player->maxSpeed) {
			stats["max_speed"][scene_name] = speed;
		}
	} else {
		stats["max_speed"][scene_name] = static_cast<int>(player->maxSpeed);
	}

	if (stats["hit_number"].contains(scene_name)) {
		int hits = stats["hit_number"][scene_name];
		if(hits > player->hitNumber) {
			stats["hit_number"][scene_name] = player->hitNumber;
		}
	} else {
		stats["hit_number"][scene_name] = player->hitNumber;
	}


	Save();
}

void Stats::UnlockLevel(const std::string& scene_name) {
	stats["levels"][scene_name] = true;
}

void Stats::BlockLevel(const std::string& scene_name) {
	stats["levels"][scene_name] = false;
}
}
