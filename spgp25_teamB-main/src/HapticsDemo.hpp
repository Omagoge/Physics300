/// @file HapticsDemo.hpp
/// @date 06 Apr 2026
/// @brief Demo actor to test all haptics features including DualSense adaptive triggers

#pragma once

#include <Toast/Input/InputListener.hpp>
#include <Toast/Objects/Actor.hpp>

/// Demonstrates all haptics features with controller input.
/// Use d-pad to cycle through effects, face buttons to trigger them.
class HapticsDemo : public toast::Actor {
public:
	REGISTER_TYPE(HapticsDemo)

	void Init() override;
	void Begin() override;
	void Tick() override;

private:
	input::Listener m_input;

	// Tracks which effect category we're demoing
	int m_currentEffect = 0;
	static constexpr int EFFECT_COUNT = 8;

	// Names for display
	static constexpr const char* EFFECT_NAMES[] = {
		"Rumble: Light Impact",
		"Rumble: Heavy Impact",
		"Rumble: Explosion",
		"Rumble: Damage",
		"Rumble: UI Confirm",
		"DualSense: Resistance",
		"DualSense: Click",
		"DualSense: Weapon Recoil"
	};

	void NextEffect();
	void PreviousEffect();
	void TriggerCurrentEffect();
	void ClearEffects();
};
