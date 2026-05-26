/**
 * @file GraphicsSettingsEvent.hpp
 * @date 7 Apr 2026
 * @author Xein
 */

#pragma once

#include <Toast/Event/Event.hpp>
#include <glm/glm.hpp>
#include <variant>
#include <string>

namespace game {

enum class GraphicsSetting : uint8_t {
	ResolutionScale,
	LightResolutionScale,
	LightQuality,
	PhysicsQuality,
	DisplayMode,
	Resolution,
	VSync,
	MaxFPS,
	Preset
};

struct GraphicsSettingsEvent : event::Event<GraphicsSettingsEvent> {
	GraphicsSetting setting;
	std::variant<float, int, bool, std::string, glm::uvec2> value;

	template<typename T>
	GraphicsSettingsEvent(GraphicsSetting s, T v) : setting(s), value(std::move(v)) { }
};

enum class AudioSetting : uint8_t {
	MusicVolume,
	EffectsVolume,
	AudioMode
};

struct AudioSettingsEvent : event::Event<AudioSettingsEvent> {
	AudioSetting setting;
	std::variant<float, std::string> value;

	template<typename T>
	AudioSettingsEvent(AudioSetting s, T v) : setting(s), value(std::move(v)) { }
};

}    // namespace game
