/// @file MuzzleEffect.cpp
/// @author dario
/// @date 23/02/2026.

#include "MuzzleEffect.hpp"

#include "Toast/Objects/ParticleSystem.hpp"
#include "Toast/Resources/ResourceManager.hpp"

void MuzzleEffect::Init() {
	Actor::Init();

	SetSerialize(false);

	m_spineRenderer = children.AddRequired<SpineRendererComponent>();
	auto atlas = resource::ResourceManager::GetInstance()->LoadResource<SpineAtlas>("CHARS/PLAYER/VFX/VFX_shot.atlas");
	auto skeleton = resource::ResourceManager::GetInstance()->LoadResource<SpineSkeletonData>("CHARS/PLAYER/VFX/VFX_shot.json", atlas);
	m_spineRenderer->SetSkeletonData(skeleton);
	transform()->scale(glm::vec3(.25f, .25, 1.0f));

	m_spineRenderer->PlayAnimation("VFX_gun_1", false, 0);
	m_spineRenderer->SetIsOccluder(false);
	m_spineRenderer->SetDrawToDepth(false);

	m_particleSystem = children.AddRequired<toast::ParticleSystem>();
	m_particleSystem->LoadFromLua("VFX/PARTICLES/MuzzleFlash.lua");
	m_particleSystem->Stop();
	m_particleSystem->rotation(glm::vec3(0, 0, -90));
}

void MuzzleEffect::PlayEffect(glm::vec2 pos, float rotation, std::string_view animName, int number) const {
	transform()->worldPosition(glm::vec3(pos.x, pos.y, 0.2f));
	transform()->worldRotation(glm::vec3(0, 0, rotation));

	// random number between 1 and number
	int random_num = 1 + (std::rand() % number);
	std::string anim_to_play = std::format("VFX_{}_{}", animName, random_num);

	m_spineRenderer->PlayAnimation(anim_to_play, false, 0);
	m_spineRenderer->SetIsOccluder(false);
	m_particleSystem->Stop();
	m_particleSystem->Play();
}
