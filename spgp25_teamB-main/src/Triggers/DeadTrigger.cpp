//
// Created by inaki on 26/01/2026.
//

#include "DeadTrigger.hpp"

#include "Player/Player.hpp"

namespace game {

void DeadTrigger::OnEnter(toast::Object* obj) {
	if (auto* player = dynamic_cast<IDamageable*>(obj)) {
		player->DoDamage(player->mHealth);
	}
}
}    // game
