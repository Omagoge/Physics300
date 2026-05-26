//
// Created by xein on 11/3/25 I;;;;;;;;;;;;
// Modified by Akaansh on 03/11/25
//

#ifndef SCENE_TRIGGER_H
#define SCENE_TRIGGER_H
#include <Toast/Objects/Actor.hpp>

namespace game {

enum class TriggerMode {
	OnEnter,
	OnExit
};

// enum to choose what type of trigger we are using is
enum class TriggerAction {
	None = 0,

	// Scene / level
	LoadScene,       // Load a scene file (inactive by default)
	EnableScene,     // Enable an already-loaded scene
	DisableScene,    // Disable an active scene
	SwitchScene,     // alternatively enable if disabled, disable if enabled (could be useful)

	// Player / gameplay
	Damage,        // Death trigger
	AddImpulse,    // Apply impulse force to the player

	// Messaging
	FireEvent,    // Publish an event

	// Misc ( we'll add more xD )
	TeleportPlayer,    // Move player to a target position or smth
	PlaySound,         // One-shot SFX by id/name when we have audio
	CameraZoom         // Player Camera Zoom
};

class SceneTrigger : public toast::Actor {
public:
	REGISTER_TYPE(SceneTrigger);

	void Init() override;
	void Begin() override;
	void Tick() override;

	void Load(json_t j, bool force_create = true) override;
	[[nodiscard]]
	json_t Save() const override;

#ifdef TOAST_EDITOR
	void Inspector() override;
#endif

private:
	// GENERAL
	TriggerMode m_mode = TriggerMode::OnEnter;       // On Enter or On Exit
	TriggerAction m_action = TriggerAction::None;    // Type of Action															[Serialized]
	bool m_repeatable = false;                       // Triggers once or triggers multiple times?	[Serialized]

	float m_delay = 0.0f;                            // Delay after triggered to do the action			[Serialized]
	float m_timerRemaining = 0.0f;
	bool m_waiting = false;
	std::function<void()> m_delayedFn;

	bool m_activated = false;    // Set if trigger has already been activated	[Private]

	// ACTION DEPENDANT

	// LOAD SCENE (params)
	std::string m_sceneName;

	// IMPULSE
	float m_impulseX = 0.0f;    // [Serialize]
	float m_impulseY = 0.0f;    // [Serialize]
	// float m_impulseZ = 0.0f;	// [Serialize] not needed for player so idk, but i could think of some cool things that cud happen

	// EVENTS
	std::string m_eventName;

	// TELEPORT
	glm::vec2 m_teleportPos = glm::vec2(0, 0);

	// DAMAGE
	float m_damage = 0.0f;

	// SFX
	std::string m_sfxName;

	// CAMERA ZOOM
	float m_zoomScale = 1.0f;    // target zoom for CameraZoom action

	// ACTION FUNCTIONS
	void OnTriggerEnter(toast::Object* other);
	void OnTriggerExit(toast::Object* other);
	void Execute(toast::Object* other);

	void Delay(float seconds, const std::function<void()>& fn);

	// Action implementations
	void DoLoadScene() const;
	void DoEnableScene() const;
	void DoDisableScene() const;
	void DoSwitchScenes() const;

	void DoDamagePlayer(toast::Object* player) const;

	void AddImpulse(toast::Object* player) const;
	void TeleportPlayer(toast::Object* player) const;

	void FireEvent(toast::Object* player);
	void PlaySound() const;
	void DoCameraZoom() const;
};

}

#endif
