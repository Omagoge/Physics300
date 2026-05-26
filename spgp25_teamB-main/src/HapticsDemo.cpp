/// @file HapticsDemo.cpp
/// @date 06 Apr 2026

#include "HapticsDemo.hpp"

#include <Toast/Input/Haptics.hpp>
#include <Toast/Log.hpp>

void HapticsDemo::Init() {
	Actor::Init();

	input::SetLayout("haptics_demo");
}

void HapticsDemo::Begin() {
	Actor::Begin();
	m_input.Subscribe0D("fire", [this](const input::Action0D* a) {
		if (a->state == input::Action0D::Started) {
			TriggerCurrentEffect();
		}
	});

	m_input.Subscribe0D("clear", [this](const input::Action0D* a) {
		if (a->state == input::Action0D::Started) {
			ClearEffects();
		}
	});

	m_input.Subscribe2D("move", [this](const input::Action2D* a) {
		if (a->state != input::Action2D::Started) {
			return;
		}

		if (a->value.y > 0.5f) {
			NextEffect();
			TriggerCurrentEffect();
		} else if (a->value.y < -0.5f) {
			PreviousEffect();
			TriggerCurrentEffect();
		}
	});

	TOAST_INFO("=== Haptics Demo Started ===");
	TOAST_INFO("Press A/Cross to trigger current effect");
	TOAST_INFO("Press X/Square to clear all effects");
	TOAST_INFO("Use up/down to cycle through effects");
	TOAST_INFO("Current: {}", EFFECT_NAMES[m_currentEffect]);
}

void HapticsDemo::Tick() {
	Actor::Tick();
}

void HapticsDemo::NextEffect() {
	m_currentEffect = (m_currentEffect + 1) % EFFECT_COUNT;
	TOAST_INFO("Effect: {}", EFFECT_NAMES[m_currentEffect]);
}

void HapticsDemo::PreviousEffect() {
	m_currentEffect = (m_currentEffect - 1 + EFFECT_COUNT) % EFFECT_COUNT;
	TOAST_INFO("Effect: {}", EFFECT_NAMES[m_currentEffect]);
}

void HapticsDemo::TriggerCurrentEffect() {
	TOAST_INFO("Triggering: {}", EFFECT_NAMES[m_currentEffect]);

	switch (m_currentEffect) {
		case 0:    // Light Impact
			haptics::ImpactLight();
			break;

		case 1:    // Heavy Impact
			haptics::ImpactHeavy();
			break;

		case 2:    // Explosion
			haptics::Explosion();
			break;

		case 3:    // Damage
			haptics::Damage();
			break;

		case 4:    // UI Confirm
			haptics::UIConfirm();
			break;

		case 5:
			// Feels like pushing through something heavy on R2
			haptics::TriggerResistance(haptics::Trigger::Right, 0.2f, 0.8f);
			break;

		case 6:
			// Sharp click halfway through the trigger
			haptics::TriggerClick(haptics::Trigger::Right, 0.5f, 1.0f);
			break;

		case 7:
			// Bouncing gun recoil
			haptics::TriggerWeapon(haptics::Trigger::Right, 0.3f, 1.0f, 150);
			break;

		default:
			break;
	}
}

void HapticsDemo::ClearEffects() {
	TOAST_INFO("Clearing all haptics");
	haptics::Stop();
	haptics::TriggerClearAll();
}
