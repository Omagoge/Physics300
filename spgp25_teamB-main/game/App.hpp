/**
 * @file App.hpp
 * @author Dante Harper
 * @date 06/10/25
 *
 * @brief Game Executable for the engine
 */

#pragma once

#include <Toast/Engine.hpp>

namespace game {
class Game final : public toast::Engine {
	void Begin() override;
};
}

namespace toast {
Engine* CreateApplication() {    // if you put inline this breaks :( -x
	return new game::Game();       // or however you construct it
}
}
