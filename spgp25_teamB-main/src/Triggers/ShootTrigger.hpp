/**
 * @file ShootTrigger.hpp
 * @author Dante Harper
 * @date 18/03/26
 */

#pragma once

#include "Toast/Components/MeshRendererComponent.hpp"
#include "Toast/CoroutineHandler.hpp"
#include "Toast/Event/ListenerComponent.hpp"
#include "Toast/GameEvents.hpp"
#include "Toast/ISerializable.hpp"
#include "Toast/Input/InputListener.hpp"
#include "Toast/Log.hpp"
#include "Toast/Objects/Actor.hpp"
#include "Toast/Objects/Scene.hpp"
#include "Toast/Physics/ColliderFlags.hpp"
#include "Toast/Physics/Rigidbody.hpp"
#include "Toast/RTTIMacros.h"
#include "Toast/Renderer/HUD/HUDLayer.hpp"
#include "Toast/Renderer/IRendererBase.hpp"
#include "Toast/WaitAsync.hpp"
#include "Weapons/IDamageable.hpp"

#include <limits>

namespace game {

class ShootTrigger : public toast::Actor,
                     public IDamageable {
	struct {
		int world;
		int level;
		physics::Rigidbody* rg;
		toast::MeshRendererComponent* mesh;
	} m;

public:
	REGISTER_TYPE(ShootTrigger);

	void Init() override {
		toast::Actor::Init();
		m.rg = children.AddRequired<physics::Rigidbody>();
	}

	void Begin() override {
		toast::Actor::Begin();
		m.rg->flags = ColliderFlags::Enemy;
		m.rg->mass = std::numeric_limits<double>::infinity();
		m.rg->gravityScale = { 0, 0 };
		mHealth = 1;
	}

	void OnDamage(float damage) override {
		mHealth = 2;
		if (m.world == -1 || m.level == -1) {
			TOAST_WARN("Invalid World and Level");
			return;
		}

		[](auto& rat) -> toast::CoroutineTask {
			renderer::HUD::HUDLayer::Get()->ExecuteJS("fadeIn()");
			co_await toast::WaitSeconds(0.6f);
			rat.scene()->Nuke();
			event::Send(new toast::LoadLevel(rat.m.world, rat.m.level));
			co_await toast::WaitSeconds(0.1f);
			renderer::HUD::HUDLayer::Get()->ExecuteJS("fadeOut()");
		}(*this);
	}

	void OnDeath() override { }

	[[nodiscard]]
	json_t Save() const override {
		json_t json = toast::Actor::Save();
		json["world"] = m.world;
		json["level"] = m.level;
		return json;
	}

	void Load(json_t j, bool force_create = true) override {
		toast::Actor::Load(j, force_create);
		if (j.contains("world") || j.contains("level")) {
			m.world = j["world"];
			m.level = j["level"];
		}
	}

#ifdef TOAST_EDITOR
	void Inspector() override;
#endif
};
}
