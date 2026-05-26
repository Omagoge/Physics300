#include "Player.hpp"

#include "Collision.hpp"
#include "Damaged.hpp"
#include "Falling.hpp"
#include "GameplayManager.hpp"
#include "Gravity/GravityEvents.hpp"
#include "Idle.hpp"
#include "InAir.hpp"
#include "Landing.hpp"
#include "PauseMenu.hpp"
#include "Run.hpp"
#include "Stopping.hpp"
#include "Toast/Audio/Audio.hpp"
#include "Toast/GameFlow.hpp"
#include "Toast/World.hpp"
#include "Weapons/BulletTrailEffect.hpp"
#include "Weapons/SMG.hpp"
#include "Weapons/Shotgun.hpp"
#include "Weapons/WallBulletHitEffect.hpp"
#include "Weapons/WeaponActorModified.hpp"

// #include "../../../../../../../../../../Editor/teamB-rel/_deps/toast-engine-src/src/Input/InputSystem.hpp" //  yeah this happened because I want to check for Dualsense :) //Akaanch please never ever ever push something like this, ask for a getter
#include <Toast/Input/Haptics.hpp>
#include <Toast/Input/InputListener.hpp>
#include <Toast/Log.hpp>
#include <Toast/Objects/Scene.hpp>
#include <Toast/Physics/ColliderFlags.hpp>
#include <Toast/Physics/Raycast.hpp>
#include <Toast/Renderer/DebugDrawLayer.hpp>
#include <Toast/Renderer/HUD/HUDLayer.hpp>
#include <Toast/Renderer/IRendererBase.hpp>
#include <Toast/Renderer/PostProcessManager.hpp>
#include <Toast/Renderer/PostProcessing/DepthOfField.hpp>
#include <Toast/Resources/ResourceManager.hpp>
#include <Toast/Time.hpp>
#include <Toast/WaitAsync.hpp>
#include <algorithm>
#include <cmath>
#include <format>

#ifdef TOAST_EDITOR
#include <imgui.h>
#include <imgui_stdlib.h>
#endif

using namespace player_states;

inline void ScanSceneForWeapons(toast::Scene* s, std::vector<toast::Object*>& outList) {
	if (!s) {
		return;
	}
	std::function<void(toast::Object*)> walk = [&](toast::Object* obj) {
		if (!obj) {
			return;
		}
		if (auto* wa = dynamic_cast<WeaponActor*>(obj)) {
			toast::Object* waobj = static_cast<toast::Object*>(wa);
			bool found = false;
			for (auto* existing : outList) {
				if (existing == waobj) {
					found = true;
					break;
				}
			}
			if (!found) {
				outList.push_back(waobj);
			}
		}
		for (auto& [_, child] : obj->children.GetAll()) {
			walk(child.get());
		}
	};

	for (auto& [_, child] : s->children.GetAll()) {
		walk(child.get());
	}
}

void Player::RefreshJavascript() {
	auto* hud = renderer::HUD::HUDLayer::Get();
	if (!hud) {
		return;
	}

	if (m.parameters.weapons[0] && m.parameters.weapons[0]->weaponData) {
		hud->ExecuteJS(std::format("setWeaponSlot(1, 'file:///assets/{}')", m.parameters.weapons[0]->weaponData->uiIconPath));
		hud->ExecuteJS(std::format("setReserveAmmo(0,{})", m.parameters.weapons[0]->weaponData->GetAvailableAmmo()));
	}

	if (m.parameters.weapons[1] && m.parameters.weapons[1]->weaponData) {
		hud->ExecuteJS(std::format("setWeaponSlot(2, 'file:///assets/{}')", m.parameters.weapons[1]->weaponData->uiIconPath));
		hud->ExecuteJS(std::format("setReserveAmmo(1,{})", m.parameters.weapons[1]->weaponData->GetAvailableAmmo()));
	}

	if (auto* current_weapon = m.parameters.weapons[m.parameters.currentWeaponIndex]) {
		auto* weapon = current_weapon->weaponData;
		if (!weapon) {
			return;
		}
		hud->ExecuteJS(std::format("HUD.setSelectedWeapon({})", m.parameters.currentWeaponIndex + 1));
		hud->ExecuteJS(std::format("HUD.setAmmoBar({})", weapon->GetCurrentAmmo()));
		hud->ExecuteJS(std::format("HUD.setMaxAmmo({})", weapon->magazineSize));
	}
}

void Player::Init() {
	// init GameplayManager
	scene()->children.AddRequired<GameplayManager>();

	// Player states
	m.stateMachine.SetParent(this);
	m.stateMachine.AddState("damaged", std::make_unique<Damaged<Player>>());    // knockback and invincibility frames
	m.stateMachine.AddState("falling", std::make_unique<Falling>());            // air movement
	m.stateMachine.AddState("idle", std::make_unique<Idle>());
	m.stateMachine.AddState("in_air", std::make_unique<InAir>());               // air movement
	m.stateMachine.AddState("landing", std::make_unique<Landing>());
	m.stateMachine.AddState("run", std::make_unique<Run>());                    // ground movement
	m.stateMachine.AddState("stopping", std::make_unique<Stopping>());          // direction change animation
	m.stateMachine.AddState("collision", std::make_unique<Collision>());        // collision animation/state
	// m.stateMachine.AddState("respawn", std::make_unique<Respawn<Player>>());

	// Input callbacks
	m.input.Subscribe2D("move", [this](auto* a) {
		if (!m.playerFuckingRecivesInput) {
			return;
		}
		this->OnMove(a);
	});
	m.input.Subscribe2D("aim", [this](auto* a) {
		if (!m.playerFuckingRecivesInput) {
			return;
		}
		this->OnAim(a);
	});
	m.input.Subscribe1D("shoot", [this](auto* a) {
		if (!m.playerFuckingRecivesInput) {
			return;
		}
		this->OnShoot(a);
		RefreshJavascript();
	});
	m.input.Subscribe1D("grav_gun", [this](auto* a) {
		if (!m.playerFuckingRecivesInput) {
			return;
		}
		this->OnGravGun(a);
	});
	m.input.Subscribe0D("switch_weapon", [this](auto* a) {
		if (!m.playerFuckingRecivesInput) {
			return;
		}
		this->OnSwitchWeapon(a);
		RefreshJavascript();
	});
	m.input.Subscribe0D("throw", [this](auto* a) {
		if (!m.playerFuckingRecivesInput) {
			return;
		}
		this->OnThrow(a);
		RefreshJavascript();
	});
	m.input.Subscribe0D("pause", [this](auto* a) {
		if (a && a->state == input::Action0D::Started) {
			if (auto* pm = PauseMenu::Get()) {
				// If current scene is a HUB_ scene, load the HUD variant of the pause menu
				try {
					bool hub = false;
					for (const auto& [id, obj] : toast::World::GetChildren()) {
						if (!obj) continue;
						if (obj->base_type() != toast::SceneT) continue;
						if (!obj->enabled()) continue;
						const std::string& nm = obj->name();
						if (!nm.empty() && nm.rfind("HUB_", 0) == 0) {
							hub = true;
							break;
						}

						audio::set_param("event:/City", "event:/Paused", 1);
						audio::set_param("event:/Port", "event:/Paused", 1);
					}
					if (hub) {
						pm->SetUrl("file:///assets/UI/menus/PauseMenuHud.html");
					} else {
						pm->SetUrl("file:///assets/UI/menus/PauseMenu.html");
					}
					TOAST_INFO("PauseMenu -> selected URL: {0}", pm->GetUrl());
				} catch (...) {
					pm->SetUrl("file:///assets/UI/menus/PauseMenu.html");
				}
				pm->enabled(true);
			}
		}
	});

	// weapon spawn event
	listener()->Subscribe<WeaponActorModified>([this](WeaponActorModified* e) -> bool {
		if (!e) {
			return false;
		}
		if (e->created) {
			TOAST_TRACE("Player::WeaponActorModified - weapon created id={}", e->weapon ? e->weapon->id() : 0);
			if (e->weapon) {
				toast::Object* obj = e->weapon;
				bool found = false;
				for (auto* existing : m.worldWeaponsList) {
					if (existing == obj) {
						found = true;
						break;
					}
				}
				if (!found) {
					m.worldWeaponsList.push_back(obj);
				}
			}
		} else {
			TOAST_TRACE("Player::WeaponActorModified - weapon removed id={}", e->weapon ? e->weapon->id() : 0);
			if (e->weapon) {
				toast::Object* obj = static_cast<toast::Object*>(e->weapon);
				for (auto it = m.worldWeaponsList.begin(); it != m.worldWeaponsList.end(); ++it) {
					if (*it == obj) {
						m.worldWeaponsList.erase(it);
						break;
					}
				}
			}
		}

		return true;
	});

	//@NOTE: I would not have shooting and reloading mechanics as a state, animations are going to be layered, and the state machine will override
	// falling or run states, we dont want that
	// m.stateMachine.AddState("reloading", std::make_unique<Reloading<Player>>()); // reloading is called as soon as we land on the ground so theres no
	// need for this just another layered animation for feedback m.stateMachine.AddState("Shooting", std::make_unique<Shooting<Player>>());

	// Sprite setup
	m.sprite = children.AddRequired<SpineRendererComponent>();
	auto PlayerAtlas = resource::LoadResource<SpineAtlas>("CHARS/PLAYER/ANIMATIONS/CH_Cat.atlas");
	auto PlayerSkeleton = resource::LoadResource<SpineSkeletonData>("CHARS/PLAYER/ANIMATIONS/CH_Cat.json", PlayerAtlas);
	m.sprite->SetSkeletonData(PlayerSkeleton);
	m.sprite->scale(glm::vec3(.25f, .25f, 1.f));    // scale down the player since the spine data is pretty big
	m.sprite->position(glm::vec3(0, -1, 0));
	m.sprite->GetSkeletonData()->setMix("an_cat_inair", "an_cat_land", .005f);
	m.sprite->SetIsOccluder(true);
	m.sprite->SetRenderLastInGeometry(true);

	// muzzleflash
	m.muzzleEffect = children.AddRequired<MuzzleEffect>();
	m.dustEffect = scene()->children.AddRequired<DustEffect>();

	m.bulletHitEffects = children.Add<Pool<WallBulletHitEffect, 6>>();
	m.bulletTrailEffects = children.Add<Pool<BulletTrailEffect, 12>>();

	m.dustParticles = children.Add<DustParticles>();
	m.dustParticlesWall = children.Add<DustParticles>();
	m.speedDust = children.AddRequired<SpeedDust>();
	m.onAirParticles = children.Add<toast::ParticleSystem>();
	m.onAirParticles->SetSerialize(false);
	DoDustParticles(false);
	m.onAirParticles->LoadFromLua("VFX/PARTICLES/OnAirParticles.lua");
	m.onAirParticles->Stop();

	m.laserBeamEffect = children.AddRequired<LaserBeamEffect>();
	// rigidbody setufp
	m.rigidbody = children.AddRequired<physics::Rigidbody>();
	m.rigidbody->restitution = .1f;
	m.rigidbody->flags |= ColliderFlags::Player;
	m.rigidbody->gravityScale = glm::vec2(0.f, 0.f);    // we will handle gravity manually to have better control over it, especially on slopes
	m.rigidbody->m_skipBoundsCheck = true;

	m.camera = scene()->children.AddRequired<FollowCamera>();

	// gun = new DefaultGun();
	// gun->LoadFromFile();

	// Create default gun actor
	m.parameters.weapons[0] = scene()->children.Add<WeaponActor>();
	// m.parameters.weapons[1] = scene()->children.Add<WeaponActor>();
	// m.parameters.weapons[1]->ChangeWeaponType(1);

	// Keep explicit world list updated for newly created weapons (avoid ordering issues with events)
	if (m.parameters.weapons[0]) {
		m.worldWeaponsList.push_back(m.parameters.weapons[0]);
	}
	if (m.parameters.weapons[1]) {
		m.worldWeaponsList.push_back(m.parameters.weapons[1]);
	}

	// Also scan the scene for any pre-existing WeaponActor instances (defensive - covers ordering issues)
	ScanSceneForWeapons(scene(), m.worldWeaponsList);

	// disable serialization on defaukt weapons
	m.parameters.weapons[0]->SetSerialize(false);
	// m.parameters.weapons[1]->SetSerialize(false);

	// m.parameters.weapons[1] = new Shotgun();
	// m.parameters.weapons[1]->LoadFromFile();

	// m.parameters.weapons[1]->Reload();

	// Do not call HUD here � HUD view may not be attached yet. We'll initialize HUD in Begin().
	RefreshJavascript();

	// mSharedPlayer = std::make_shared<Player*>(this);

	// GameplayManager::m_HUD = m.fade;

	auto e = audio::load_bank("assets/AUDIO/Desktop/Music.bank");
	auto o = audio::load_bank("assets/AUDIO/Desktop/Master.strings.bank");
	auto s = audio::load_bank("assets/AUDIO/Desktop/Master.bank");
	if (!e.has_value()) {
		TOAST_ERROR("Error loading music bank");
	}

	audio::load_event("event:/Placeholder");
	
	toast::Window::GetInstance()->SetShowMouseCursor(false);

#ifndef TOAST_EDITOR
	m.parameters.showDebugVectors = false;
#endif
}

void Player::Begin() {
	if (!toast::World::Has("Fade")) {
		toast::World::LoadSceneSync("SCENES/Fade.scene");
	}
	input::SetLayout("player");

	m.camera->SetFollowCameraTarget(this);
	renderer::IRendererBase::GetInstance()->SetActiveCamera(m.camera);

	// Initialize previous velocity
	SetPreviousVelocity(m.rigidbody->velocity);

	m.parameters.groundTangent = { 1.0f, 0.0f };
	m.parameters.gravity = glm::vec2(0.f, -m.parameters.gravForce);
	m.parameters.currentWeaponIndex = 0;

	m.parameters.weapons[0]->GrabWeapon();
	if (m.parameters.weapons[1]) {
		m.parameters.weapons[1]->GrabWeapon();
	}

	// re-scan scene to capture any weapons
	ScanSceneForWeapons(scene(), m.worldWeaponsList);

	// Initialize HUD now that engine and renderer are initialized (safer to call from Begin)
	if (auto hud = renderer::HUD::HUDLayer::Get()) {
		if (GetWeaponData(0)) {
			hud->ExecuteJS(std::format("setWeaponSlot(1, 'file:///assets/{}')", GetWeaponData(0)->uiIconPath));
		}
		if (GetWeaponData(1)) {
			hud->ExecuteJS(std::format("setWeaponSlot(2, 'file:///assets/{}')", GetWeaponData(1)->uiIconPath));
		}

	// Hide timer on the HUB scenes
		try {
			bool hub = false;
			for (const auto& [id, obj] : toast::World::GetChildren()) {
				if (!obj) continue;
				if (obj->base_type() != toast::SceneT) continue;
				if (!obj->enabled()) continue;
				const std::string& nm = obj->name();
				if (!nm.empty() && nm.rfind("HUB_", 0) == 0) {
					hub = true;
					break;
				}
			}
			if (hub) {
				hud->ExecuteJS("enterHubMode()");
			} else {
				hud->ExecuteJS("exitHubMode()");
			}
		} catch (...) { }
	} else {
		TOAST_TRACE("Player::Begin - HUD not available yet");
	}

	RefreshJavascript();

	// renderer::HUD::HUDLayer::Get()->ExecuteJS(std::format("HUD.setSelectedWeapon({})", m.parameters.currentWeaponIndex + 1));
}

void Player::EarlyTick() {
	float delta = Time::delta();

	milliseconds += delta;
	if (auto hud = renderer::HUD::HUDLayer::Get()) {
		try {
			bool hub = false;
			for (const auto& [id, obj] : toast::World::GetChildren()) {
				if (!obj) continue;
				if (obj->base_type() != toast::SceneT) continue;
				if (!obj->enabled()) continue;
				const std::string& nm = obj->name();
				if (!nm.empty() && nm.rfind("HUB_", 0) == 0) {
					hub = true;
					break;
				}
			}
			if (hub) {
				hud->ExecuteJS("enterHubMode()");
			} else {
				hud->ExecuteJS("exitHubMode()");
				// Only update the timer when not in hub mode
				hud->ExecuteJS(std::format("updateTimer({})", milliseconds * 1000));
			}
		} catch (...) { }
	} else {
		// TOAST_WARN("HUD not available");
	}

	GroundCheck();
	if (!m.parameters.grounded) {
		timeOnAir += Time::delta();
	}

	if (m.parameters.isUsingGrav) {
		timeOnGravity += Time::delta();
	}
	// CollisionCheck();
	UpdateSpriteRotation();
	Aim();
	UpdateSpriteScale();

	UpdateCameraOffset();
	UpdateDepthOfFieldFocusDistance();

	if (m.tickStateMachine) {
		m.stateMachine.Tick();
	}

	if (m.parameters.grounded) {
		m.onAirParticles->Pause();
	} else if (m.parameters.isUsingGrav) {
		m.onAirParticles->Play();
	}

	// gun->DrawCrosshair(m.parameters.aimingDirection, transform()->worldPosition());
	// if (m.parameters.weapons[m.parameters.currentWeaponIndex] != nullptr) {
	// 	m.parameters.weapons[m.parameters.currentWeaponIndex]->weaponData->DrawCrosshair(m.parameters.aimingDirection, transform()->worldPosition());
	// }


	// wall haptic — separate from the dust particles check
	if (!m.parameters.grounded) {
		float speedX = std::abs(m.rigidbody->velocity.x);
		float prevSpeedX = std::abs(m.previousVelocity.x);

		// big drop in X speed while airborne = hit a wall
		if (prevSpeedX > 30.f && (prevSpeedX - speedX) > 20.f) {
			haptics::ImpactHeavy();
		}
	}

	// Save current velocity
	SetPreviousVelocity(m.rigidbody->velocity);

	// Tick down invincibility frames
	if (m.invincibilityTimer > 0.f) {
		m.invincibilityTimer -= delta;
		if (m.invincibilityTimer <= 0.f) {
			m.invincibilityTimer = 0.f;
			mInvincible = false;
		}
	}

	// Health recharge (authoritative)
	if (m.rechargeActive) {
		m.healthRechargeTimer += static_cast<float>(Time::delta());
		// handle possibly multiple completions if delta is large
		while (m.healthRechargeTimer >= HEALTH_RECHARGE_DURATION && mHealth < MAX_HEALTH) {
			m.healthRechargeTimer -= HEALTH_RECHARGE_DURATION;
			int completedIndex = static_cast<int>(mHealth);    // segment index that finished recharging
			mHealth = std::min(mHealth + 1.f, MAX_HEALTH);
			if (auto hud = renderer::HUD::HUDLayer::Get()) {
				// Tell HUD the specific segment completed to avoid duplicate animations/race
				TOAST_TRACE("Player::Recharge complete segment {} -> health {}", completedIndex, static_cast<int>(mHealth));
				hud->ExecuteJS(std::format("HUD.completeRecharge({})", completedIndex));
				// Give the HUD a short moment to start the completion fade before updating the displayed health
				hud->ExecuteJS(std::format("setTimeout(()=>HUD.setHealth({}), 40)", static_cast<int>(mHealth)));
			}
			// if still missing health, queue next recharge and notify HUD
			if (mHealth < MAX_HEALTH) {
				m.rechargeActive = true;
				m.rechargeTargetIndex = static_cast<int>(mHealth);
				if (auto hud = renderer::HUD::HUDLayer::Get()) {
					TOAST_TRACE("Player::Scheduling next recharge for segment {}", m.rechargeTargetIndex);
					hud->ExecuteJS(
					    std::format("setTimeout(()=>HUD.startRecharge({}, {}), 50)", m.rechargeTargetIndex, static_cast<int>(HEALTH_RECHARGE_DURATION * 1000))
					);
				}
			} else {
				m.rechargeActive = false;
				m.rechargeTargetIndex = -1;
				m.healthRechargeTimer = 0.f;
			}
		}
	}

	// Handle flashing
	if (m_flashingActive) {
		const float dt = delta;
		m_flashTimer -= dt;
		if (m_flashTimer <= 0.0f) {
			// toggle visibility
			if (GetSpineRenderer()) {
				GetSpineRenderer()->enabled(m_flashRemaining % 2 == 1);
			}
			m_flashRemaining -= 1;
			if (m_flashRemaining <= 0) {
				// end flashing
				m_flashingActive = false;
				if (GetSpineRenderer()) {
					GetSpineRenderer()->enabled(true);
				}
			} else {
				m_flashTimer = 0.1f;    // FLASH_HALF_PERIOD
			}
		}
	}

	// Handle respawn
	if (m_waitingForRespawn) {
		m_respawnDelayTimer -= delta;
		if (m_respawnDelayTimer <= 0.0f) {
			m_waitingForRespawn = false;
			SetPlayerRecivesInput(true);
			GetStateMachine()->SetState("idle");
			SetPlayerTicksStateMachine(true);
			// restore aiming if needed
			ReapplyAimingIfActive();
		}
	}

	// Handle death
	if (m_waitingForDeathActions) {
		m_deathDelayTimer -= delta;
		if (m_deathDelayTimer <= 0.0f) {
			m_waitingForDeathActions = false;
			if (auto hud = renderer::HUD::HUDLayer::Get()) {
				hud->ExecuteJS("HUD.fadeOut()");
			}
			Respawn();
		}
	}

	// Pending discard
	for (auto it = m_pendingDiscardTimers.begin(); it != m_pendingDiscardTimers.end();) {
		it->second -= delta;
		if (it->second <= 0.0f) {
			IWeapon* w = it->first;
			// If weapon still exists and is empty, throw
			if (w && w->GetAvailableAmmo() == 0 && w->GetCurrentAmmo() == 0) {
				// guard: find player current weapon pointer exists and matches
				if (m.parameters.currentWeaponIndex != 0 && m.parameters.weapons[m.parameters.currentWeaponIndex] != nullptr) {
					// check that the weapon pointer is the same actor's weapon data
					if (m.parameters.weapons[m.parameters.currentWeaponIndex]->weaponData == w) {
						Throw();
					}
				}
			}
			it = m_pendingDiscardTimers.erase(it);
		} else {
			++it;
		}
	}

	// Handle pending reload
	if (m_reloadPending) {
		m_reloadDelayTimer -= delta;
		if (m_reloadDelayTimer <= 0.0f) {
			m_reloadPending = false;
			if (m.parameters.grounded) {
				// Full reload when grounded
				Reload();
			} else {
				if (m.parameters.currentWeaponIndex != 0) {
					auto* currentWeapon = GetCurrentWeaponData();
					if (currentWeapon != nullptr && currentWeapon->GetCurrentAmmo() == 0 && currentWeapon->GetAvailableAmmo() == 0) {
						bool alreadyPending = false;
						for (const auto& entry : m_pendingDiscardTimers) {
							if (entry.first == currentWeapon) {
								alreadyPending = true;
								break;
							}
						}
						if (!alreadyPending) {
							m_pendingDiscardTimers.emplace_back(currentWeapon, 0.6f);
						}
					}
				}
			}
		}
	}

	// blink
	if (m.blinkTimer <= 0.0f) {
		m.blinkTimer = m.blinkTime;
		if (GetHealth() > 0.0f) {
			m.sprite->PlayAnimation("an_cat_blink", false, 4);
		}
	} else {
		m.blinkTimer -= delta;
	}

	m.dustParticles->position(glm::vec3(m.feetPosition.x, m.feetPosition.y, 0.0f));
	m.dustParticles->rotation(glm::vec3(0, 0, m.feetRotation));

	m.speedDust->transform()->position(glm::vec3(m.feetPosition.x, m.feetPosition.y, 0.0f));
	m.speedDust->transform()->rotation(glm::vec3(0, 0, m.feetRotation));

	float vel = glm::length(m.previousVelocity);
	if (vel > maxSpeed) {
		maxSpeed = vel;
	}

	if (m.parameters.grounded) {
		if (vel > 50.f) {
			m.speedDust->GetParticleSystem()->Play();
		} else {
			m.speedDust->GetParticleSystem()->Pause();
		}
	} else {
		m.speedDust->GetParticleSystem()->Pause();
	}

	// raycasting wallss
	if (abs(m.previousVelocity.y) > 5.f) {
		auto right = physics::RayCast(
		    glm::vec2(transform()->worldPosition().x, transform()->worldPosition().y), m.parameters.groundTangent, ColliderFlags::Ground
		);
		if (right.has_value() && right->distance < 1.5f) {
			m.dustParticlesWall->position(glm::vec3(m.parameters.groundTangent.x, m.parameters.groundTangent.y, 0));
			float angleRadians = std::atan2(right->normal.x, right->normal.y);
			m.dustParticlesWall->rotation(glm::vec3(0, 0, glm::degrees(-angleRadians)));
			m.dustParticlesWall->Play();
		} else {
			auto left = physics::RayCast(
			    glm::vec2(transform()->worldPosition().x, transform()->worldPosition().y), -m.parameters.groundTangent, ColliderFlags::Ground
			);
			if (left.has_value() && left->distance < 1.5f) {
				m.dustParticlesWall->position(glm::vec3(-m.parameters.groundTangent.x, -m.parameters.groundTangent.y, 0));
				float angleRadians = std::atan2(left->normal.x, left->normal.y);
				m.dustParticlesWall->rotation(glm::vec3(0, 0, glm::degrees(-angleRadians)));
				m.dustParticlesWall->Play();
			} else {
				m.dustParticlesWall->Pause();
			}
		}
	} else {
		m.dustParticlesWall->Pause();
	}

	float musicSpeed = vel / 80.f;
	float drumSpeed = vel / 35.f;

	audio::set_param("event:/City", "parameter:/Speed Drums", drumSpeed);
	audio::set_param("event:/City", "parameter:/Speed Arp", musicSpeed);
	audio::set_param("event:/Port", "parameter:/Speed Drums", drumSpeed);
	audio::set_param("event:/Port", "parameter:/Speed Arp", musicSpeed);

	if (transform()->worldPosition().y <= -1000) {
		Respawn();
	}
}

void Player::PhysTick() { }

#ifdef TOAST_EDITOR
void Player::EditorTick() { }
#endif

void Player::Destroy() { }

void Player::OnDisable() {
	Actor::OnDisable();

	m.dustParticles->Pause();
	m.dustParticlesWall->Pause();
	m.speedDust->GetParticleSystem()->Pause();
	m.onAirParticles->Pause();
	m.laserBeamEffect->Hide();
	
	// revert changes to hud when player disables
	// idk if this is needed but jst for safety -x
	if (auto hud = renderer::HUD::HUDLayer::Get()) {
		try {
			hud->ExecuteJS("exitHubMode()");
		} catch (...) { }
	}
}

void Player::OnDamage(float damage) {
	TOAST_INFO("Player::OnDamage | health now {:.0f}", mHealth);

	haptics::Damage();

	if (auto hud = renderer::HUD::HUDLayer::Get()) {
		hud->ExecuteJS(std::format("HUD.setHealth({})", static_cast<int>(mHealth)));
		// Cancel any HUD recharge visuals
		hud->ExecuteJS("HUD.cancelRecharge()");
	}

	// Cancel any active recharge in C++
	m.rechargeActive = false;
	m.healthRechargeTimer = 0.f;
	m.rechargeTargetIndex = -1;
	++hitNumber;

	// Start new recharge timer if not at max and still alive
	if (mHealth > 0.f && mHealth < MAX_HEALTH) {
		m.rechargeActive = true;
		m.rechargeTargetIndex = static_cast<int>(mHealth);    // missing segment index (0-based)
		m.healthRechargeTimer = 0.f;
		if (auto hud = renderer::HUD::HUDLayer::Get()) {
			// duration in milliseconds - delay slightly to avoid UI animation races
			hud->ExecuteJS(
			    std::format("setTimeout(()=>HUD.startRecharge({}, {}), 50)", m.rechargeTargetIndex, static_cast<int>(HEALTH_RECHARGE_DURATION * 1000))
			);
		}
	}

	// Grant invincibility frames so consecutive hits don't stack
	m.invincibilityTimer = INVINCIBILITY_DURATION;
	mInvincible = true;

	// Start flashing sequence using ticked timers owned by Player
	const int FLASH_COUNT = 4;
	m_flashingActive = true;
	m_flashTotalToggles = FLASH_COUNT * 2;
	m_flashRemaining = m_flashTotalToggles;
	m_flashTimer = 0.1f;    // FLASH_HALF_PERIOD

	                        // Note: invincibility is handled by invincibilityTimer in EarlyTick(), no detached coroutine needed
}

void Player::Respawn(glm::vec3 spawnPos) {
	input::SetLayout("player");
	milliseconds = 0;
	timeOnAir = 0;
	timeOnGravity = 0;
	maxSpeed = 0;
	m.playerRespawnPos = glm::vec3(spawnPos.x, spawnPos.y, 0.1f);
	Respawn();
}

void Player::Respawn() {
	mHealth = RESPAWN_HEALTH;
	mInvincible = false;
	m.invincibilityTimer = 0.f;
	m.parameters.isAiming = false;

	m.parameters.aimingDirection = glm::vec2(1, 0);

	m.parameters.weapons[0]->GrabWeapon();
	if (m.parameters.weapons[1]) {
		m.parameters.weapons[1]->GrabWeapon();
	}

	Reload();

	SetRunEarlyTick(true);

	transform()->worldPosition(m.playerRespawnPos);
	m.camera->transform()->worldPosition(glm::vec3(m.playerRespawnPos.x, m.playerRespawnPos.y, 15.f));

	if (m.rigidbody) {
		m.rigidbody->SetVelocity(glm::vec2(0.f));
	}

	m.tickStateMachine = false;
	m.playerFuckingRecivesInput = false;

	for (int track = 0; track <= 4; ++track) {
		m.sprite->StopAnimation(track);
	}
	m.sprite->ResetSkeletonToSetupPose();

	m.sprite->PlayAnimation("an_cat_spawn", false, 3);
	m.sprite->NextCrossFadeToDefault(.2f, 3);
	m.stateMachine.SetState("idle");

	if (auto hud = renderer::HUD::HUDLayer::Get()) {
		hud->ExecuteJS("HUD.cancelRecharge()");
		hud->ExecuteJS(std::format("HUD.setHealth({})", static_cast<int>(mHealth)));
	}

	m.parameters.currentWeaponIndex = 0;
	renderer::HUD::HUDLayer::Get()->ExecuteJS(std::format("HUD.setSelectedWeapon({})", m.parameters.currentWeaponIndex + 1));
	renderer::HUD::HUDLayer::Get()->ExecuteJS(std::format("HUD.setAmmoBar({})", GetCurrentWeaponData()->GetCurrentAmmo()));
	renderer::HUD::HUDLayer::Get()->ExecuteJS(std::format("HUD.setMaxAmmo({})", GetCurrentWeaponData()->magazineSize));

	// Setup respawn delay to restore input/state and reapply aiming on completion
	m_waitingForRespawn = true;
	m_respawnDelayTimer = 1.0f;
	
	RefreshJavascript();
}

void Player::OnDeath() {
	TOAST_INFO("Player::OnDeath");

	haptics::ImpactHeavy();

	// Lock input and state-machine immediately
	m.playerFuckingRecivesInput = false;
	m.tickStateMachine = false;

	m.parameters.aimingDirection = glm::vec2(1, 0);

	// SetRunEarlyTick(false);

	for (int track = 0; track <= 4; ++track) {
		m.sprite->StopAnimation(track);
	}

	m.sprite->ResetSkeletonToSetupPose();

	m.sprite->PlayAnimation("an_cat_dead", false, 0);

	// Start death delay actions (fade and respawn) using ticked timer
	if (auto hud = renderer::HUD::HUDLayer::Get()) {
		hud->ExecuteJS("HUD.fadeIn()");
		hud->ExecuteJS("HUD.cancelRecharge()");
	}
	m_waitingForDeathActions = true;
	m_deathDelayTimer = 2.0f;
}

#if TOAST_EDITOR
void Player::Inspector() {
	Actor::Inspector();
	// parameters
	if (ImGui::CollapsingHeader("Player Movement")) {
		ImGui::Spacing();
		ImGui::SeparatorText("Parameters");
		ImGui::DragFloat("Move Speed", &m.parameters.moveSpeed);
		ImGui::DragFloat("Air Move Speed", &m.parameters.airMoveSpeed);
		ImGui::DragFloat("Ground Check Distance", &m.parameters.groundCheckDistance);
		ImGui::DragFloat2("Gravity", &m.parameters.gravity.x);
		ImGui::DragFloat("Gravity Scale", &m.parameters.gravForce);
		ImGui::DragFloat2("Ground Tangent", &m.parameters.groundTangent.x);
		ImGui::Checkbox("Grounded", &m.parameters.grounded);

		ImGui::Text("Current State: %s", m.stateMachine.GetCurrentState().data());
		auto vel = m.rigidbody->GetVelocity();
		ImGui::Text("Velocity: (x)%f (y)%f", vel.x, vel.y);

		ImGui::Spacing();
		ImGui::SeparatorText("Rotation");
		ImGui::DragFloat("Rotation Lerp Speed", &m.parameters.rotationLerpSpeed, 0.1f, 0.1f, 20.0f);
		ImGui::Text("Current Sprite Z: %.2f deg", m.currentRotationZ);
		ImGui::Text("Target Sprite Z: %.2f deg", m.targetRotationZ);

		ImGui::Spacing();
		ImGui::SeparatorText("Direction Change");
		ImGui::DragFloat("Direction Change Threshold", &m.parameters.directionChangeThreshold, 0.1f, 0.1f, 200.0f);
		ImGui::DragFloat("Direction Change Threshold Idle", &m.parameters.directionChangeThresholdIdle, 0.1f, 0.1f, 200.0f);
		ImGui::DragFloat("Stop Animation Duration", &m.parameters.stopAnimationDuration, 0.01f, 0.05f, 1.0f);
		ImGui::DragFloat("Stopping Drag (Grounded)", &m.parameters.stoppingDragGrounded, 0.1f, 0.5f, 10.f);
		ImGui::Text("Sprite Facing: %s", m.spriteFacingDirection > 0 ? "Right" : "Left");
		ImGui::Text("Is Stopping: %s", m.isStopping ? "Yes" : "No");

		ImGui::Spacing();
		ImGui::SeparatorText("Stomping and falling");
		ImGui::DragFloat("Stomp Verical Speed Threshold", &m.parameters.stompVerticalVelocityThreshold, 0.1f, -100.f, 0.f);
		ImGui::DragFloat("Stomp Time", &m.parameters.stompTime, 0.01f, 0.01f, 1.f);

		ImGui::Spacing();
		ImGui::SeparatorText("Collision");
		ImGui::DragFloat("Collision Speed Threshold X", &m.parameters.collisionSpeedThresholdX, 0.1f, 0.1f, 100.0f);
		ImGui::DragFloat("Collision Speed Threshold y", &m.parameters.collisionSpeedThresholdY, 0.1f, 0.1f, 100.0f);
		ImGui::DragFloat("Collision Reduction Threshold", &m.parameters.collisionReductionThreshold, 0.1f, 0.1f, 50.0f);
		ImGui::DragFloat("Collision State Duration", &m.parameters.collisionStateDuration, 0.01f, 0.0f, 5.0f);
		ImGui::Text("Is Colliding: %s", m.isColliding ? "Yes" : "No");
		ImGui::Text("Collision Timer: %.3f", m.collisionTimer);

		ImGui::Spacing();
		ImGui::SeparatorText("Aim IK");
		ImGui::DragFloat("IK Aim Distance", &m.parameters.aimIKDistance, 0.1f, 0.1f, 50.f);
		ImGui::DragFloat2("IK World Offset", &m.parameters.aimIKWorldOffset.x, 0.05f, -20.f, 20.f);
		ImGui::DragFloat("Aim Flip Threshold", &m.parameters.aimFlipThreshold, 0.01f, -1.f, 1.f);
		ImGui::Text("Aiming: %s", m.parameters.isAiming ? "Yes" : "No");
		ImGui::Text("Aim Dir: (%.2f, %.2f)", m.parameters.aimingDirection.x, m.parameters.aimingDirection.y);
		ImGui::Text("Running Backwards: %s", m.isRunningBackwards ? "Yes" : "No");
		if (m.sprite) {
			glm::vec2 bonePos = m.sprite->GetBoneLocalPosition("controller");
			ImGui::Text("Controller bone local: (%.2f, %.2f)", bonePos.x, bonePos.y);
		}

		ImGui::Spacing();
		ImGui::SeparatorText("Wall rebounce");
		ImGui::DragFloat("Min vel to trigger", &m.parameters.minVelocityToTriggerRebounce, 0.1f, 1.0f, 50.f);
		ImGui::DragFloat("Min wall angle", &m.parameters.minRebounceWallAngle, 0.1f, 0.f, 90.f);
		ImGui::DragFloat("Min rebounce distance", &m.parameters.rebounceDistanceThreshold, 0.1f, 0.1f, 10.f);
		ImGui::SliderFloat("rebounce multiplier", &m.parameters.rebounceDampening, -1.0f, 1.0f);
		ImGui::DragFloat("rebounce Y multiplier", &m.parameters.rebounceYMultiplier, 0.1);

		ImGui::Spacing();
		ImGui::SeparatorText("Camera Offset");
		ImGui::DragFloat("Default Zoom Z Offset", &m.defaultZoomOffset, 0.1f, -20.f, 20.f);
		ImGui::DragFloat3("Zoom Offset Scale", &m.cameraSpeedModifier.x, 0.01f, 0.f, 1.f);
		ImGui::DragFloat2("Max Offset Space", &m.cameraMaxOffset.x, 0.1f, 0.f, 30.0f);

		ImGui::Spacing();
		ImGui::SeparatorText("Release for gravity orbit");
		ImGui::DragFloat("Gravity Release Scale", &m.parameters.releaseForce, 0.5f, 0.0f, 5.0f);

		ImGui::Spacing();
		ImGui::SeparatorText("Debug");
		ImGui::Checkbox("Show Debug Vectors", &m.parameters.showDebugVectors);
	}

	if (ImGui::CollapsingHeader("Weapons")) {
		ImGui::Spacing();
		ImGui::Indent(20);

		for (size_t i = 0; i < m.parameters.weapons.size(); ++i) {
			std::string weaponName = (m.parameters.weapons[i] != nullptr) ? m.parameters.weapons[i]->weaponData->name : "Empty";
			weaponName = weaponName + ((i == m.parameters.currentWeaponIndex) ? " [Equipped]" : "");
			ImGui::PushID(i);
			if (ImGui::CollapsingHeader((std::to_string(i) + ": " + weaponName).c_str())) {
				if (m.parameters.weapons[i] != nullptr) {
					m.parameters.weapons[i]->Inspector();
				}
			}
			ImGui::Spacing();
			ImGui::PopID();
		}

		ImGui::Unindent(20);
	}

	// I want to test enemies without dying -ax
	ImGui::Checkbox("Invincible?", &mInvincible);
}
#endif

///-----------------------------------------///

void Player::OnMove(const input::Action2D* action) {
	if (action->state == input::Action2D::Started || action->state == input::Action2D::Ongoing) {
		m.moveInput = action->value;

		// Only switch to run if NOT in ...
		if (m.stateMachine.GetCurrentState() != "falling" && m.stateMachine.GetCurrentState() != "in_air" &&
		    m.stateMachine.GetCurrentState() != "landing" && m.stateMachine.GetCurrentState() != "stopping" &&
		    m.stateMachine.GetCurrentState() != "collision") {
			m.stateMachine.SetState("run");
		}
	} else if (action->state == input::Action2D::Finished) {
		m.moveInput = glm::vec2(0.f);
		// Only go to idle if grounded
		if (m.parameters.grounded && m.stateMachine.GetCurrentState() != "falling" && m.stateMachine.GetCurrentState() != "in_air" &&
		    m.stateMachine.GetCurrentState() != "landing" && m.stateMachine.GetCurrentState() != "stopping" &&
		    m.stateMachine.GetCurrentState() != "collision") {
			m.stateMachine.SetState("idle");
		}
	}
}

void Player::OnAim(const input::Action2D* a) {
	if (GetCurrentWeaponData() == nullptr) {
		// no weapon equipped, do not allow aiming
		m.parameters.isAiming = false;
		m.sprite->CrossFadeToDefault(.1f, 1);
		return;
	}
	glm::vec2 actualVec = a->value;
	if (a->device == input::Device::Mouse) {
		glm::vec2 playerPos = glm::vec2(transform()->worldPosition().x, transform()->worldPosition().y);
		glm::vec2 diff = a->value - playerPos;
		if (glm::length(diff) < 0.1f) {
			m.parameters.isAiming = false;
			if (m.parameters.currentWeaponIndex == 0) {
				m.sprite->CrossFadeToDefault(.1f, 1);
			}
			return;
		} else {
			actualVec = glm::normalize(diff);
			if (GetCurrentWeaponData()) {
				m.sprite->PlayAnimation(GetCurrentWeaponData()->aimingAnimation, true, 1);
				m.parameters.isAiming = true;
			} else {
				m.parameters.isAiming = false;
				if (m.parameters.currentWeaponIndex == 0) {
					m.sprite->CrossFadeToDefault(.1f, 1);
				}
				return;
			}
		}
	}
	if (a->state == input::Action2D::Started || a->state == input::Action2D::Ongoing) {
		m.parameters.aimingDirection = actualVec;

		if (a->state == input::Action2D::Started) {
			if (GetCurrentWeaponData()) {
				m.sprite->PlayAnimation(GetCurrentWeaponData()->aimingAnimation, true, 1);
				m.parameters.isAiming = true;
			} else {
				m.parameters.isAiming = false;
				// just crossfade to default idle ani if on guns not any other weapon
				if (m.parameters.currentWeaponIndex == 0) {
					m.sprite->CrossFadeToDefault(.1f, 1);
				}
			}
		}
	} else if (a->state == input::Action2D::Finished) {
		m.parameters.isAiming = false;
		m.parameters.aimingDirection = glm::vec2(0.f);

		// just crossfade to default idle ani if on guns not any other weapon
		if (m.parameters.currentWeaponIndex == 0) {
			m.sprite->CrossFadeToDefault(.1f, 1);
		}
	}
}

//helper for checking wheter to use dualsense trigger features or not
auto is_dualsense() -> bool {
	const auto type = input::GetGamepadType();
	return type == SDL_GAMEPAD_TYPE_PS5;
}

void Player::OnShoot(const input::Action1D* a) {
	// auto result = gun->Shoot(m.parameters.aimingDirection,  transform()->worldPosition());
	// if (result.has_value()) {
	// 	m.rigidbody->AddForce(result->knockbackForce);
	// }

	auto* current_weapon = GetCurrentWeaponData();

	// Do not shoot if in collision state or is stopping
	if (not current_weapon || m.isColliding || m.isStopping) {
		return;
	}

	// PS5 trigger checks so it should only shoot once trigger passes the actuation threshold.
	if (is_dualsense()) {  //PERF: REALLY EXPENSIVE CALL, MAYBE WE SHOULD CACHE THIS?
		static constexpr float TRIGGER_THRESHOLD = 0.9f;

		if (a->state == input::Action1D::Finished) {
			m_triggerArmed = true;    // re-arm once the trigger is acc released
			return;
		}

		if (current_weapon->isAutomatic) {
			// automatic guns shud fire continuously while the trigger is held past the trig threshold
			if (a->value < TRIGGER_THRESHOLD) return;
		} else {
			// otherwise we should fire once per pull, afta threshold
			if (!m_triggerArmed || a->value < TRIGGER_THRESHOLD) return;
			m_triggerArmed = false;    // now we disarm until the trigger is released again
		}
	} else {

		if (!current_weapon->isAutomatic && a->state != input::Action1D::Started) {
			// If the gun is not automatic, only shoot on started
			return;
		}
	}

	auto result = current_weapon->Shoot(m.parameters.aimingDirection, transform()->worldPosition());
	if (result.has_value()) {
		// y-axis
		if (auto vel = m.rigidbody->GetVelocity(); vel.y < 0.0f) {
			// Skip if player is trying to go down faster
			if (m.parameters.aimingDirection.y > 0) {
				goto SKIP_Y_AXIS_FALLOFF;
			}

			// We need to apply a bit of velocity reduction if the player is falling
			// so that the movement doesn't feel as bad
			// APPLY THIS ONLY ON THE Y AXIS DONT BE A RETARD
			vel.y *= current_weapon->recoil.velocityReduction;
			m.rigidbody->SetVelocity(vel);
		}

SKIP_Y_AXIS_FALLOFF:

		// x-axis
		// We adjust the weapon knockback on the X axis depending on the plaer current orizontal velocit relative to the aiming direction
		auto vel = m.rigidbody->GetVelocity();
		float aimX = m.parameters.aimingDirection.x;
		if (std::abs(aimX) > 0.001f) {
			float mag = std::abs(result->knockbackForce.x);
			float denom = std::max(0.0001f, current_weapon->recoil.forceReduction);

			bool isOpposite = (vel.x * aimX) > 0.f;
			// Detect a recent flip into the opposite direction
			bool justEnteredOpposite = (m.previousVelocity.x * aimX) >= 0.f && isOpposite;

			if (isOpposite && !m.parameters.grounded) {
				// Airborne & opposite: modest boost to X magnitude
				float speedFactor = std::min<float>(std::fabs(vel.x) / denom, current_weapon->recoil.maxSpeedFactor);
				float boost = 1.0f + current_weapon->recoil.oppositeBoostScale * speedFactor;    // e.g. up to ~1.1x
				mag *= boost;

				if (justEnteredOpposite) {
					// Small X impulse to feel snappy
					float velImpulseX = std::min(mag * current_weapon->recoil.xImpulseScale, current_weapon->recoil.xImpulseCap);
					// Small upward boost proportional to knockback magnitude
					float velImpulseY = std::min(mag * current_weapon->recoil.yBoostScale, current_weapon->recoil.yBoostCap);

					auto newVel = vel;
					newVel.x += std::copysign(velImpulseX, result->knockbackForce.x);
					newVel.y += velImpulseY;
					m.rigidbody->SetVelocity(newVel);
				}
			} else {
				// default X recoil
				mag = std::abs(result->knockbackForce.x);
			}

			// Appl ground dampening to the X force so sooting on ground feels less extreme
			if (m.parameters.grounded) {
				mag *= current_weapon->recoil.groundDamp;
			}

			result->knockbackForce.x = std::copysign(mag, result->knockbackForce.x);
		}

		float angle = std::atan2(m.parameters.aimingDirection.y, m.parameters.aimingDirection.x) * 180.f / glm::pi<float>();

		// Wall Rebounce
		// If at high velocity and shooting against a close, approximately vertical wall, rebounce.
		if (result->distance < m.parameters.rebounceDistanceThreshold && glm::length(vel) >= m.parameters.minVelocityToTriggerRebounce &&
		    !m.parameters.grounded) {
			// Find the closest collision to check its wall normal
			const physics::RayResult* closest = nullptr;
			for (const auto& c : result->collisions) {
				if (!closest || c.distance < closest->distance) {
					closest = &c;
				}
			}

			if (closest) {
				const float wallElevation = std::asin(glm::clamp(std::abs(closest->normal.y), 0.0f, 1.0f));
				// Convert minRebounceWallAngle to max allowed elevation from horizontal
				const float maxElevation = glm::radians(90.0f - m.parameters.minRebounceWallAngle);

				if (wallElevation <= maxElevation) {
					// glm::dvec2 opposite_velocity = -m.rigidbody->GetVelocity() * static_cast<double>(m.parameters.rebounceDampening);

					// opposite_velocity.y = glm::max(
					//     opposite_velocity.y * m.parameters.rebounceYMultiplier, 0.0
					//);    // only apply rebounce on the y axis if it's upwards, to avoid boosting downwards velocity when shooting down while falling

					//// Apply the rebounce force in the opposite of the aiming direction
					// glm::dvec2 rebounce_force = -m.parameters.aimingDirection * static_cast<float>(glm::length(opposite_velocity));
					// result->knockbackForce += rebounce_force;

					double velocity_magnitude = glm::length(m.rigidbody->GetVelocity()) * m.parameters.rebounceDampening;
					glm::dvec2 vel = glm::dvec2 { glm::normalize(-m.parameters.aimingDirection) } * velocity_magnitude;
					vel.y *= m.parameters.rebounceYMultiplier;

					// clang-format off
					// TOAST_WARN(
					//     "Changed player velocity from x{.2} y{.2} to x{.2} y{.2}\n\tInitial magnitude: {.4}, Final magnitude: {.4}(1:{.3} ratio)",
					//     m.rigidbody->GetVelocity().x, m.rigidbody->GetVelocity().y,
					//     vel.x, vel.y,
					//     velocity_magnitude, glm::length(vel),
					//     glm::length(vel) / std::abs(velocity_magnitude)
					// );
					// clang-format on

					m.rigidbody->SetVelocity(vel);
				}
			}
		}

		m.rigidbody->AddForce(result->knockbackForce);

		// MECAGO EN DIOS CADA ARMA TIENE DISTINTO ORIGIN NAMEEEEE
		m.muzzleEffect->PlayEffect(m.sprite->GetBoneWorldPosition(current_weapon->effectSocket), angle, current_weapon->muzzleAnimName);
		m.sprite->PlayAnimation(current_weapon->shootingAnimation, false, 2);
		m.sprite->NextCrossFadeToDefault(0.05f, 2);

		m.sprite->PlayAnimation("an_cat_launched", false, 3);
		m.sprite->NextCrossFadeToDefault(0.2f, 3);

		// Hitscan, bullet traces and bullet hit effects
		const glm::vec2 muzzlePos = m.sprite->GetBoneWorldPosition(current_weapon->effectSocket);
		auto* hitPool = m.bulletHitEffects;
		auto* trailPool = m.bulletTrailEffects;
		for (const auto& c : result->collisions) {
			// bullet trail effect
			auto* trail = trailPool->Release();
			if (trail) {
				trail->PlayEffect(muzzlePos, c.point, [trailPool, trail]() {
					trailPool->Hold(trail);
				});
			}

			// bullet hit effect
			auto* effect = hitPool->Release();
			if (effect) {
				effect->PlayEffect(c.point, c.normal, [hitPool, effect]() {
					hitPool->Hold(effect);
				});
			}

			// renderer::DrawDebugArrow(c.point, c.normal * 100.0f, 10.f, glm::vec4(1.f, 0.f, 0.f, 1.f));
		}



		constexpr float MISS_RANGE = 100.0f;

		if (result->collisions.empty()) {
			for (const auto& missDir : result->missedDirections) {
				auto* trail = trailPool->Release();
				if (trail) {
					const glm::vec2 missEnd = muzzlePos + missDir * MISS_RANGE;
					trail->PlayEffect(muzzlePos, missEnd, [trailPool, trail]() {
						trailPool->Hold(trail);
					});
				}
			}
			if (result->missedDirections.empty()) {
				auto* trail = trailPool->Release();
				if (trail) {
					const glm::vec2 missEnd = muzzlePos + m.parameters.aimingDirection * MISS_RANGE;
					trail->PlayEffect(muzzlePos, missEnd, [trailPool, trail]() {
						trailPool->Hold(trail);
					});
				}
			}
		} else {
			for (const auto& missDir : result->missedDirections) {
				auto* trail = trailPool->Release();
				if (trail) {
					const glm::vec2 missEnd = muzzlePos + missDir * MISS_RANGE;
					trail->PlayEffect(muzzlePos, missEnd, [trailPool, trail]() {
						trailPool->Hold(trail);
					});
				}
			}
		}

		// Weapon Based Haptics
		if (dynamic_cast<Shotgun*>(current_weapon)) {
			haptics::TriggerClick(haptics::Trigger::Right, 0.8f, 2.0f);  // heavy boom pow shotgun
			haptics::Rumble(0.4f, 1.0f, 200);
		}
		else if (dynamic_cast<SMG*>(current_weapon)) {
			haptics::TriggerWeapon(haptics::Trigger::Right, 0.1f, 0.6f, 80);  // rapid light prrr SMG
			haptics::Rumble(0.1f, 0.8f, 50);
			if (current_weapon->GetCurrentAmmo() <= 0) {
				haptics::TriggerClear(haptics::Trigger::Right);
			}
		}
		else {
			haptics::TriggerClick(haptics::Trigger::Right, 0.2f, 0.2f);  // default gun click
			haptics::Rumble(0.2f, 0.6f, 100);

		}
		m_reloadPending = true;
		m_reloadDelayTimer = 0.1f;
	}
	++shotNumber;
}

void Player::OnGravGun(const input::Action1D* a) {
	switch (a->state) {
		using input::Action1D;
		case Action1D::Started:
			event::Send(new GravityBegin());
			// m.parameters.isUsingGrav = true;
			break;
		case Action1D::Finished:
			event::Send(new GravityEnd());
			// m.parameters.isUsingGrav = false;
			// puto iniaki te lo juro cuando te vea en digipen te voy a reventar
			// m.rigidbody->SetVelocity(m.rigidbody->GetVelocity() * double{m.parameters.releaseForce});
			break;
		default: break;
	}
}

void Player::OnSwitchWeapon(const input::Action0D* a) {
	if (a->state == input::Action0D::Finished) {
		SwitchWeapon();
	}
}

void Player::OnThrow(const input::Action0D* action) {
	if (action->state == input::Action0D::Started) {
		// If there is an available weapon slot and a weapon in range, grab it instead of throwing currently equipped weapon
		if (GetAvailableWeaponSlot() != -1) {
			if (auto w = IsWeaponInRange(); w.has_value()) {
				m.parameters.holdingThrow = false;
				Grab(w.value());
				return;
			}
		}

		if (m.parameters.currentWeaponIndex == 0 || GetCurrentWeaponData() == nullptr) {
			// no weapon equipped, do not allow throw input to be held
			m.parameters.holdingThrow = false;
			return;
		}

		m.parameters.holdingThrow = true;
		m.parameters.throwHoldTime = 0.0f;
	} else if (action->state == input::Action0D::Ongoing) {
		if (m.parameters.holdingThrow) {
			m.parameters.throwHoldTime += Time::delta();
			if (m.parameters.throwHoldTime >= m.parameters.maxThrowHoldTime) {
				if (m.parameters.currentWeaponIndex != 0 && GetCurrentWeaponData() != nullptr) {
					Throw();
					m.parameters.holdingThrow = false;
				}
			}
		}
	} else if (action->state == input::Action0D::Finished) {
		if (m.parameters.holdingThrow) {
			if (m.parameters.currentWeaponIndex != 0 && GetCurrentWeaponData() != nullptr) {
				Throw();
				m.parameters.holdingThrow = false;
			}
		}
	}
}

void Player::Shoot() const {
	// TODO Everything else with the return value
}

void Player::SwitchWeapon() {
	// Increment weapon index and wrap within bounds
	m.parameters.currentWeaponIndex++;

	if (m.parameters.currentWeaponIndex >= m.parameters.weapons.size()) {
		m.parameters.currentWeaponIndex = 0;
	}

	if (m.parameters.weapons[m.parameters.currentWeaponIndex] == nullptr) {
		// if the next weapon slot is empty, do not switch and return
		m.parameters.currentWeaponIndex--;
		return;
	}

	// Update Spine aim anim if aiming
	// if (m.parameters.isAiming) {
	m.sprite->PlayAnimation(GetCurrentWeaponData()->aimingAnimation, true, 1);
	if (m.parameters.currentWeaponIndex == 0) {
		m.sprite->CrossFadeToDefault(.2f, 1);
	}
	// }

	// m.sprite->PlayAnimation("an_cat_swapWeapon", false, 3);
	// m.sprite->NextCrossFadeToDefault(.01f,2);

	if (not m.parameters.weapons[m.parameters.currentWeaponIndex]) {
		m.parameters.currentWeaponIndex = 0;

		return;
	}

	renderer::HUD::HUDLayer::Get()->ExecuteJS(std::format("HUD.setSelectedWeapon({})", m.parameters.currentWeaponIndex + 1));
	renderer::HUD::HUDLayer::Get()->ExecuteJS(std::format("HUD.setAmmoBar({})", GetCurrentWeaponData()->GetCurrentAmmo()));
	renderer::HUD::HUDLayer::Get()->ExecuteJS(std::format("HUD.setMaxAmmo({})", GetCurrentWeaponData()->magazineSize));
}

std::optional<WeaponActor*> Player::IsWeaponInRange() const {
	for (auto* obj : m.worldWeaponsList) {
		if (!obj) {
			continue;
		}
		if (auto* weapon = dynamic_cast<WeaponActor*>(obj)) {
			if (!weapon->enabled()) {
				continue;
			}
			if (glm::distance(weapon->transform()->worldPosition(), transform()->worldPosition()) <= m.parameters.weaponGrabRange) {
				return weapon;
			}
		}
	}
	return std::nullopt;
}

void Player::Aim() {
	if (!m.sprite) {
		return;
	}

	// Only update facing direction if we are NOT stopping or colliding
	if (!m.isStopping && !m.isColliding && m.parameters.isAiming) {
		float dotTangent = glm::dot(m.parameters.aimingDirection, m.parameters.groundTangent);
		const float flipThreshold = 0.05f;

		if (dotTangent > flipThreshold) {
			m.spriteFacingDirection = 1;
		} else if (dotTangent < -flipThreshold) {
			m.spriteFacingDirection = -1;
		}

		// Do raycast from muzzleflash origin towards +x and check if hitted wall, if so, create a laser pointer from initial position into hit pos
		if (auto r = physics::RayCast(
		        m.sprite->GetBoneWorldPosition(GetCurrentWeaponData()->effectSocket) - m.parameters.aimingDirection * 1.5f,
		        m.parameters.aimingDirection,
		        ColliderFlags::Ground | ColliderFlags::Enemy
		    );
		    r.has_value()) {
			if (r->distance > 2.f) {
				m.laserBeamEffect->SetBeam(m.sprite->GetBoneWorldPosition(GetCurrentWeaponData()->effectSocket), r->point);
				m.laserBeamEffect->Show();
			} else {
				m.laserBeamEffect->Hide();
			}
		} else {
			m.laserBeamEffect->SetBeam(m.sprite->GetBoneWorldPosition(GetCurrentWeaponData()->effectSocket), m.sprite->GetBoneWorldPosition(GetCurrentWeaponData()->effectSocket) + m.parameters.aimingDirection * 100.0f);
			m.laserBeamEffect->Show();
		}
	} else {
		m.laserBeamEffect->Hide();
	}

	glm::vec2 effectiveDir;

	if (m.isStopping || m.isColliding) {
		effectiveDir = m.parameters.groundTangent * static_cast<float>(m.spriteFacingDirection);
	} else if (!m.parameters.isAiming) {
		effectiveDir = m.parameters.groundTangent * static_cast<float>(m.spriteFacingDirection);
	} else {
		effectiveDir = m.parameters.aimingDirection;
	}

	const glm::vec2 playerWorld = transform()->worldPosition();

	float distance = m.parameters.aimIKDistance;
	if (m.isStopping || m.isColliding) {
		distance *= 1.5f;
	}

	const glm::vec2 targetWorld = playerWorld + (effectiveDir * distance) + m.parameters.aimIKWorldOffset;

	const glm::vec2 spinePos = m.sprite->WorldPositionToSpineLocal(targetWorld);
	m.sprite->SetBoneLocalPosition("controller", spinePos);

	if (m.parameters.showDebugVectors) {
		renderer::DrawDebugArrow(playerWorld, effectiveDir, distance, glm::vec4(0.2f, 0.8f, 1.0f, 1.0f));
	}
}

void Player::Reload() {
	for (int i = 0; i < m.parameters.weapons.size(); ++i) {
		auto* weapon = GetWeaponData(i);
		if (weapon != nullptr) {
			weapon->Reload();
		}
	}

	// If the current weapon is completely out of ammo after the reload attempt, queue a timed discard
	if (m.parameters.currentWeaponIndex != 0) {
		auto* currentWeapon = GetCurrentWeaponData();
		if (currentWeapon != nullptr && currentWeapon->GetCurrentAmmo() == 0 && currentWeapon->GetAvailableAmmo() == 0) {
			// Only queue once per weapon
			bool alreadyPending = false;
			for (const auto& entry : m_pendingDiscardTimers) {
				if (entry.first == currentWeapon) {
					alreadyPending = true;
					break;
				}
			}
			if (!alreadyPending) {
				m_pendingDiscardTimers.emplace_back(currentWeapon, 0.6f);
			}
		}
	}

	RefreshJavascript();
}

void Player::Throw() {
	// do not allow to throw weapon 1
	// shit ass fix but works
	if (m.parameters.currentWeaponIndex == 0) {
		m.parameters.currentWeaponIndex++;
	}

	if (m.parameters.weapons[m.parameters.currentWeaponIndex] != nullptr) {
		float forceScale = m.parameters.throwHoldTime / m.parameters.maxThrowHoldTime;

		toast::Scene* s = toast::GameFlow::CurrentScene();
		if (!s) {
			TOAST_WARN("Player::Throw - no current scene to adopt thrown weapon into!, falling back to player's scene");
			s = scene();
		}
		WeaponActor* thrown = m.parameters.weapons[m.parameters.currentWeaponIndex];
		if (thrown) {
			s->Adopt(thrown->id());
			// Ensure worldWeaponsList includes the thrown weapon
			bool found = false;
			for (auto* existing : m.worldWeaponsList) {
				if (existing == static_cast<toast::Object*>(thrown)) {
					found = true;
					break;
				}
			}
			if (!found) {
				m.worldWeaponsList.push_back(static_cast<toast::Object*>(thrown));
			}
		}

		m.parameters.weapons[m.parameters.currentWeaponIndex]->ThrowWeapon(
		    transform()->worldPosition(), m.parameters.aimingDirection * forceScale * m.parameters.maxThrowForce
		);

		m.parameters.weapons[m.parameters.currentWeaponIndex] = nullptr;
		renderer::HUD::HUDLayer::Get()->ExecuteJS(std::format("HUD.removeWeapon({})", m.parameters.currentWeaponIndex + 1));

		SwitchWeapon();    // Auto-switch to next weapon
	}
}

void Player::Grab(WeaponActor* weapon) {
	int index = GetAvailableWeaponSlot();
	m.parameters.weapons[index] = weapon;
	weapon->GrabWeapon();
	Adopt(weapon->id());

	for (auto it = m.worldWeaponsList.begin(); it != m.worldWeaponsList.end(); ++it) {
		if (*it == static_cast<toast::Object*>(weapon)) {
			m.worldWeaponsList.erase(it);
			break;
		}
	}

	// TODO: also update sprite gun on player
	renderer::HUD::HUDLayer::Get()->ExecuteJS(std::format("setWeaponSlot({}, 'file:///assets/{}')", index + 1, GetWeaponData(index)->uiIconPath));

	// swap to newl grabed weapon
	SwitchWeapon();
}

void Player::GroundCheck() {
	///@TODO: PROBABL ADD A RAMP TAG OR SOMETING SO I CAN AVE TE PLAER STICK TO TE GROUND IF ON NORMAL FLOOR BUT FLING IF ON RAMP

	// Raycast to ground to detect slopes and check if grounded for state transitions
	auto raycastResult = physics::RayCast(
	    transform()->worldPosition(),
	    m.parameters.grounded ? -m.parameters.gravity : -glm::vec2(-m.parameters.groundTangent.y, m.parameters.groundTangent.x),
	    ColliderFlags::Ground
	);

	glm::vec2 playerPos = transform()->worldPosition();

	if (raycastResult.has_value() && raycastResult->distance <= m.parameters.groundCheckDistance) {
		// grounded - calculate ground tangent from normal
		m.parameters.grounded = true;

		// Ground tangent is perpendicular to ground normal
		m.parameters.groundTangent = glm::vec2(raycastResult->normal.y, -raycastResult->normal.x);

		// Surface normal points away from surface, negate it to point into surface
		m.parameters.gravity = { -raycastResult->normal.x * m.parameters.gravForce, -raycastResult->normal.y * m.parameters.gravForce };
		// m.rigidbody->gravityScale = m.parameters.gravity;

		if (m.parameters.showDebugVectors) {
			// Draw ground normal
			renderer::DrawDebugArrow(raycastResult->point, raycastResult->normal, 3.0f, glm::vec4(0, 1, 0, 1));

			// Draw ground tangent
			renderer::DrawDebugArrow(raycastResult->point, glm::normalize(m.parameters.groundTangent), 3.0f, glm::vec4(0, 0.5f, 1, 1));

			// Draw gravity direction
			renderer::DrawDebugArrow(playerPos, glm::normalize(-m.parameters.gravity), 2.0f, glm::vec4(1, 0, 0, 1));

			// Draw raycast line
			renderer::DebugLine(playerPos, raycastResult->point, glm::vec4(1, 1, 0, 1));

			// Draw hit point
			renderer::DebugCircle(raycastResult->point, 0.3f, glm::vec4(1, 1, 0, 1), 8, true);
		}
	} else {
		// Not grounded
		m.parameters.grounded = false;
		// m.parameters.groundTangent = glm::vec2(1.f, 0.f);    // Default horizontal tangent
		// m.parameters.gravity = glm::vec2(0.f, -m.parameters.gravForce);
		m.rigidbody->gravityScale = m.parameters.gravity;

		// Debug visualization
		if (m.parameters.showDebugVectors) {
			// Draw raycast line (gray - no hit)
			glm::vec2 rayEnd = playerPos + glm::vec2(0.f, -1.f) * m.parameters.groundCheckDistance;
			renderer::DebugLine(playerPos, rayEnd, glm::vec4(0.5f, 0.5f, 0.5f, 1));
		}
	}
}

void Player::UpdateSpriteRotation() {
	float deltaTime = static_cast<float>(Time::delta());
	glm::vec2 groundNormal = { -m.parameters.groundTangent.y, m.parameters.groundTangent.x };
	// renderer::DrawDebugArrow(glm::vec2(transform()->worldPosition().x, transform()->worldPosition().y), groundNormal, 3.0f, glm::vec4(1, 1, 0, 1));
	float angleRadians = std::atan2(groundNormal.x, groundNormal.y);
	m.targetRotationZ = -glm::degrees(angleRadians);

	float deltaAngle = m.targetRotationZ - m.currentRotationZ;

	while (deltaAngle <= -180.0f) {
		deltaAngle += 360.0f;
	}
	while (deltaAngle > 180.0f) {
		deltaAngle -= 360.0f;
	}

	float LerpFactor = 1.0f - std::exp(-m.parameters.rotationLerpSpeed * deltaTime);
	m.currentRotationZ += deltaAngle * LerpFactor;

	m.currentRotationZ = std::fmod(m.currentRotationZ, 360.0f);

	// Apply to sprite
	glm::vec3 currentRotation = m.sprite->rotation();
	currentRotation.z = m.currentRotationZ;
	m.sprite->rotation(currentRotation);

	glm::vec3 targetPosition = glm::vec3(-glm::sin(angleRadians), -glm::cos(angleRadians), 0.0f);
	m.currentPosition = m.currentPosition + (targetPosition - m.currentPosition) * LerpFactor;
	m.sprite->position(m.currentPosition);

	m.feetPosition = targetPosition;
	m.feetRotation = currentRotation.z;
}

void Player::UpdateSpriteScale() {
	if (!m.sprite) {
		return;
	}

	// Only update facing direction when not in stopping/collision animation and not aiming
	if (!m.isStopping && !m.isColliding && !m.parameters.isAiming && !m.parameters.isUsingGrav) {
		// Project velocity along the ground tangent
		glm::vec2 tangent = m.parameters.groundTangent;
		if (glm::length(tangent) < 1e-4f) {
			tangent = glm::vec2(1.f, 0.f);
		} else {
			tangent = glm::normalize(tangent);
		}
		float velocityAlongTangent = glm::dot(glm::vec2(m.rigidbody->velocity), tangent);

		if (std::abs(velocityAlongTangent) > 0.1f) {
			int newDirection = velocityAlongTangent > 0 ? 1 : -1;
			if (newDirection != m.spriteFacingDirection) {
				m.spriteFacingDirection = newDirection;
			}
		}
	}

	// Determine if we are running backwards:
	// move input direction opposes sprite facing direction (projected along ground tangent)
	{
		float inputAlongTangent = GetInputAlongTangent();
		bool movingSignificantly = std::abs(inputAlongTangent) > 0.1f;
		bool velocityOpposesFacing = (inputAlongTangent * static_cast<float>(m.spriteFacingDirection)) < 0.f;
		m.isRunningBackwards = movingSignificantly && velocityOpposesFacing;
	}

	// Apply scale to sprite
	glm::vec3 currentScale = m.sprite->scale();
	currentScale.x = std::abs(currentScale.x) * static_cast<float>(m.spriteFacingDirection);
	m.sprite->scale(currentScale);
}

void Player::CollisionCheck() {
	// Current velocity

	auto* params = GetParameters();

	float currentSpeedX = glm::length(m.rigidbody->velocity.x);
	float prevSpeedX = glm::length(m.previousVelocity.x);

	float currentSpeedY = glm::length(m.rigidbody->velocity.y);
	float prevSpeedY = glm::length(m.previousVelocity.y);

	if (prevSpeedY > params->collisionSpeedThresholdY) {
		return;
	}

	if (-prevSpeedX >= currentSpeedX) {
		// Complete flip on X detected, this is wall rebound, not collision
		return;
	}
	if (-prevSpeedY >= currentSpeedY) {
		// Complete flip on y detected, this is wall rebound, not collision
		return;
	}

	// If previous speed was high enough and dropped significantly in this tick, consider collision
	if (prevSpeedX >= params->collisionSpeedThresholdX && (prevSpeedX - currentSpeedX) >= params->collisionReductionThreshold) {
		// Trigger collision state if not already colliding
		if (!IsColliding()) {
			SetColliding(true);
			GetStateMachine()->SetState("collision");
			// reset collision timer
			*GetCollisionTimer() = 0.0f;
		}
	}

	// previous velocity is updated at the end of Tick()
}

void Player::UpdateCameraOffset() {
	// if (glm::length(m.rigidbody->velocity) < m.minVelocityForOffset) {
	//	m.camera->SetOffset(glm::vec3(0.0f, m.defaultVerticalOffset, m.defaultZoomOffset));
	//	return;
	// }
	//
	// glm::vec2 vel = glm::normalize(m.rigidbody->velocity);
	// float speedFactor = std::min<float>(glm::length(m.rigidbody->velocity) - m.minVelocityForOffset / m.maxSpeedForOffset, 1.0f);
	//
	// float zoomOffset = m.defaultZoomOffset - (speedFactor * m.zoomOffsetPerUnitSpeed);
	//
	// glm::vec3 offset = glm::vec3(
	//	vel.x * m.maxOffsetDistance * speedFactor,
	//	vel.y * m.maxOffsetDistance * speedFactor,
	//	zoomOffset + m.defaultZoomOffset
	//);

	glm::vec2 velocity = m.rigidbody->GetVelocity();
	float speed = glm::length(velocity);
	speed = speed < 20.0f ? 0.0f : speed;    // cancel z offset if not moving
	velocity.x = std::abs(velocity.x) < 20.0f ? 0.0f : velocity.x - std::copysignf(20.0f, velocity.x);
	velocity.y = std::abs(velocity.y) < 20.0f ? 0.0f : velocity.y - std::copysignf(20.0f, velocity.y);

	glm::vec3 offset = { std::copysignf(std::min(std::abs(velocity.x * m.cameraSpeedModifier.x), m.cameraMaxOffset.x), velocity.x),
		                   std::copysignf(std::min(std::abs(velocity.y * m.cameraSpeedModifier.y), m.cameraMaxOffset.y), velocity.y),
		                   m.defaultZoomOffset + speed * m.cameraSpeedModifier.z };

	m.camera->SetOffset(offset);
}

void Player::UpdateDepthOfFieldFocusDistance() {
	if (!m.camera) {
		return;
	}

	auto* rendererBase = renderer::IRendererBase::GetInstance();
	auto* ppm = rendererBase ? rendererBase->GetPostProcessManager() : nullptr;
	if (!ppm) {
		return;
	}

	const float playerZ = transform()->worldPosition().z;
	const float cameraZ = m.camera->transform()->worldPosition().z;
	const float focusDistance = std::max(0.01f, std::abs(cameraZ - playerZ));

	for (auto& pass : ppm->GetGlobalStack()) {
		if (!pass || pass->GetTypeId() != "DepthOfField") {
			continue;
		}

		if (auto* dof = dynamic_cast<DepthOfField*>(pass.get())) {
			dof->SetFocusDistance(focusDistance);
		}
	}
}

char Player::GetAvailableWeaponSlot() const {
	for (size_t i = 0; i < m.parameters.weapons.size(); ++i) {
		if (m.parameters.weapons[i] == nullptr) {
			return static_cast<char>(i);
		}
	}
	return -1;    // No available slot
}

inline IWeapon* Player::GetWeaponData(int idx) {
	if (m.parameters.weapons[idx]) {
		return m.parameters.weapons[idx]->weaponData;
	}

	return nullptr;
}

IWeapon* Player::GetCurrentWeaponData() {
	return GetWeaponData(m.parameters.currentWeaponIndex);
}

void Player::DoDustEffect(int type) const {
	if (!m.dustEffect || !m.sprite) {
		return;
	}

	// Spawn dust at the player's feet position in world coordinates
	glm::vec3 playerWorld = transform()->worldPosition();
	glm::vec2 feetWorld = glm::vec2(playerWorld.x + m.feetPosition.x, playerWorld.y + m.feetPosition.y);
	m.dustEffect->PlayEffect(feetWorld, m.feetRotation, type);
}

void Player::DoDustParticles(bool enable) const {
	m.dustParticles->EnableEffect(enable);
	m.dustParticles->position(glm::vec3(m.feetPosition.x, m.feetPosition.y, 0.f));
	m.dustParticles->rotation(glm::vec3(0, 0, m.feetRotation));
}

void Player::ReapplyAimingIfActive() {
	if (m.parameters.isAiming) {
		if (auto* wd = GetCurrentWeaponData()) {
			if (GetSpineRenderer()) {
				GetSpineRenderer()->PlayAnimation(wd->aimingAnimation, true, 1);
			}
		}
	}
}
