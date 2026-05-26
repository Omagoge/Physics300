/// @file DustEffect.cpp
/// @author dario
/// @date 10/03/2026.

#include "DustEffect.hpp"

#include "Toast/Renderer/DebugDrawLayer.hpp"
#include "Toast/Resources/ResourceManager.hpp"

void DustEffect::Init() {
	SetSerialize(false);
	m_spine = children.AddRequired<SpineRendererComponent>();
	auto atlas = resource::ResourceManager::GetInstance()->LoadResource<SpineAtlas>("CHARS/PLAYER/VFX/VFX_falldust.atlas");
	auto skeleton = resource::ResourceManager::GetInstance()->LoadResource<SpineSkeletonData>("CHARS/PLAYER/VFX/VFX_falldust.json", atlas);
	m_spine->SetSkeletonData(skeleton);
	transform()->scale(glm::vec3(.25f, .25f, 1.0f));

	// m_spine->PlayAnimation("VFX_gun_1", false, 0);
	SetRunEarlyTick(false);
	SetRunLateTick(false);
}

void DustEffect::PlayEffect(glm::vec2 pos, float rot, int type) {
	transform()->worldPosition(glm::vec3(pos.x, pos.y, 0.1f));

	transform()->rotation(glm::vec3(0.f, 0.f, rot));

	std::string animName;
	switch (type) {
		case 0:
			return;    // dust small is so fucking bad i would rather not have it
			break;
		case 1: animName = "medium"; break;
		case 2: animName = "big"; break;
		default: animName = "medium"; break;
	}
	int random_num = 1 + (std::rand() % 3);
	std::string anim_to_play = std::format("VFX_falldust_{}_{}", animName, random_num);

	m_spine->PlayAnimation(anim_to_play, false, 0);
	m_spine->NextCrossFadeToDefault(.01f, 0);
}
