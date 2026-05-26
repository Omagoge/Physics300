/// @file GameplayManager.hpp
/// @author dario
/// @date 09/03/2026.
#pragma once

#include "Toast/Components/HtmlView.hpp"

#include <Toast/Objects/Object.hpp>

class FollowCamera;
class Player;

class GameplayManager : public toast::Object {
public:
	REGISTER_ABSTRACT(GameplayManager);

	static Player* GetPlayer();
	static FollowCamera* GetFollowCamera();

	static toast::HtmlView* m_HUD;

protected:
	void Init() override;
	void Begin() override;

	void Destroy() override;

private:
	static Player* m_player;
	static FollowCamera* m_followCamera;
};
