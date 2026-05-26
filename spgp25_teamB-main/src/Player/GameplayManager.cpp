/// @file GameplayManager.cpp
/// @author dario
/// @date 09/03/2026.

#include "GameplayManager.hpp"

#include "Player.hpp"
#include "Toast/World.hpp"

Player* GameplayManager::m_player = nullptr;
FollowCamera* GameplayManager::m_followCamera = nullptr;
toast::HtmlView* GameplayManager::m_HUD = nullptr;

Player* GameplayManager::GetPlayer() {
	if (m_player == nullptr) {
		m_player = dynamic_cast<Player*>(toast::World::GetFromType<Player>());
	}
	return m_player;
}

FollowCamera* GameplayManager::GetFollowCamera() {
	// if (m_followCamera == nullptr) {
	m_followCamera = dynamic_cast<FollowCamera*>(toast::World::GetFromType<FollowCamera>());
	// }
	return m_followCamera;
}

void GameplayManager::Init() {
	Object::Init();
	SetSerialize(false);

	SetRunEarlyTick(false);
	SetRunLateTick(false);
}

void GameplayManager::Begin() {
	Object::Begin();
}

void GameplayManager::Destroy() {
	Object::Destroy();

	m_player = nullptr;
}
