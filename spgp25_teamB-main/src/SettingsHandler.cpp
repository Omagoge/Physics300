#include "SettingsHandler.hpp"
#include "GraphicsSettingsEvent.hpp"
#include "Localization.hpp"
#include "Stats.hpp"

#include <Toast/Audio/Audio.hpp>
#include <Toast/Event/Event.hpp>
#include <Toast/Localization.hpp>
#include <Toast/Log.hpp>
#include <Toast/Renderer/IRendererBase.hpp>
#include <Toast/Resources/ResourceManager.hpp>
#include <Toast/Window/Window.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>

namespace game {

namespace {

void EnsureToastLocalizationReady() {
	if (!toast::Localization::GetLanguages().empty()) {
		return;
	}
	if (!toast::Localization::LoadFile("UI/locales.json")) {
		toast::Localization::LoadFile("assets/UI/locales.json");
	}
}

std::string CoerceSupportedLanguage(const std::string& language) {
	std::string normalized = Localization::NormalizeLanguage(language);
	if (!Localization::IsSupportedLanguage(normalized)) {
		return "en";
	}
	return normalized;
}

}    // namespace

std::map<std::string, std::string> SettingsHandler::s_stagedGraphics;

bool SettingsHandler::ProcessMessage(const std::string& msg, EvalJSFunc evalJS) {
	if (msg.find("[Settings]") == std::string::npos) {
		return false;
	}

	// Extract after [Settings]
	size_t start = msg.find("[Settings]") + 11;
	std::string payload = msg.substr(start);
	while (!payload.empty() && std::isspace(payload.front())) payload.erase(0, 1);
	while (!payload.empty() && std::isspace(payload.back())) payload.pop_back();

	if (payload == "init") {
		HandleInit(evalJS);
		return true;
	}

	// Split into parts
	std::istringstream iss(payload);
	std::string categoryDotSetting, value;
	iss >> categoryDotSetting;
	std::getline(iss, value);
	// Trim value
	while (!value.empty() && std::isspace(value.front())) value.erase(0, 1);

	// Parse category.setting
	size_t dot = categoryDotSetting.find('.');
	if (dot == std::string::npos) {
		if (categoryDotSetting == "language") {
			HandleLanguage(value, evalJS);
			return true;
		}
		return false;
	}

	std::string category = categoryDotSetting.substr(0, dot);
	std::string setting = categoryDotSetting.substr(dot + 1);

	if (category == "audio") {
		HandleAudio(setting, value);
	} else if (category == "graphics") {
		HandleGraphics(setting, value);
	}

	return true;
}

void SettingsHandler::HandleInit(EvalJSFunc evalJS) {
	if (!evalJS) {
		TOAST_WARN("Settings init called without EvalJS function");
		return;
	}

	PopulateResolutionOptions(evalJS);
	const std::string savedLanguage =
	    (Stats::stats.contains("language") && Stats::stats["language"].is_string()) ? Stats::stats["language"].get<std::string>() : "en";
	const std::string language = CoerceSupportedLanguage(savedLanguage);
	if (!Stats::stats.contains("language") || !Stats::stats["language"].is_string() || Stats::stats["language"].get<std::string>() != language) {
		Stats::stats["language"] = language;
		Stats::Save();
	}
	PushLocalization(evalJS, language);

	TOAST_INFO("Settings UI initialized");
}

void SettingsHandler::HandleLanguage(const std::string& value, EvalJSFunc evalJS) {
	const std::string language = CoerceSupportedLanguage(value);

	EnsureToastLocalizationReady();
	if (!toast::Localization::SetLanguage(language)) {
		TOAST_WARN("Failed to set runtime language '{}', falling back to 'en'", language);
		toast::Localization::SetLanguage("en");
	}

	const std::string activeLanguage = toast::Localization::GetLanguage();
	Stats::stats["language"] = activeLanguage.empty() ? language : activeLanguage;
	Stats::Save();
	PushLocalization(evalJS, Stats::stats["language"].get<std::string>());
	TOAST_INFO("Language set to: {}", Stats::stats["language"].get<std::string>());
}

void SettingsHandler::HandleAudio(const std::string& setting, const std::string& value) {
	if (setting == "musicVolume") {
		float vol = std::stof(value);
		audio::set_music_volume(vol);
		Stats::stats["audio"]["musicVolume"] = vol;
		Stats::Save();
	} else if (setting == "effectsVolume") {
		float vol = std::stof(value);
		audio::set_effects_volume(vol);
		Stats::stats["audio"]["effectsVolume"] = vol;
		Stats::Save();
	} else if (setting == "mode") {
		audio::AudioMode mode = audio::AudioMode::Stereo;
		if (value == "mono") mode = audio::AudioMode::Mono;
		else if (value == "stereo") mode = audio::AudioMode::Stereo;
		else if (value == "binaural") mode = audio::AudioMode::Binaural;
		else if (value == "surround") mode = audio::AudioMode::Surround;

		audio::set_audio_mode(mode);
		Stats::stats["audio"]["audioMode"] = value;
		Stats::Save();
		TOAST_INFO("Audio mode set to: {}", value);
	}
}

void SettingsHandler::HandleGraphics(const std::string& setting, const std::string& value) {
	// "apply" triggers the actual application of staged settings
	if (setting == "apply") {
		ApplyGraphicsSettings();
		return;
	}

	s_stagedGraphics[setting] = value;
}

int SettingsHandler::GetPhysicsTPS(const std::string& quality) {
	if (quality == "low") return 50;
	if (quality == "mid") return 60;
	if (quality == "high") return 90;
	if (quality == "ultra") return 120;
	return 60;
}

void SettingsHandler::ApplyGraphicsSettings() {
	auto* renderer = renderer::IRendererBase::GetInstance();
	if (!renderer) {
		TOAST_ERROR("No renderer instance available");
		return;
	}

	auto& config = Stats::stats["graphics"];

	// Process each staged setting
	for (const auto& [key, value] : s_stagedGraphics) {
		if (key == "graphicsPreset") {
			config["preset"] = value;
			event::Send(new GraphicsSettingsEvent(GraphicsSetting::Preset, value));
		} else if (key == "resolutionScale") {
			float scale = std::stof(value) / 100.0f;
			renderer->SetResolutionScale(scale);
			config["resolutionScale"] = scale;
			event::Send(new GraphicsSettingsEvent(GraphicsSetting::ResolutionScale, scale));
		} else if (key == "lightResolutionScale") {
			float scale = std::stof(value) / 100.0f;
			renderer->SetLightResolutionScale(scale);
			config["lightResolutionScale"] = scale;
			event::Send(new GraphicsSettingsEvent(GraphicsSetting::LightResolutionScale, scale));
		} else if (key == "lightQuality") {
			config["lightQuality"] = value;
			// Map quality to shadow settings
			if (value == "low") {
				renderer->SetShadowMapResolution(256);
				renderer->SetShadowRaymarchSteps(8);
			} else if (value == "mid") {
				renderer->SetShadowMapResolution(512);
				renderer->SetShadowRaymarchSteps(16);
			} else if (value == "high") {
				renderer->SetShadowMapResolution(1024);
				renderer->SetShadowRaymarchSteps(32);
			} else if (value == "ultra") {
				renderer->SetShadowMapResolution(2048);
				renderer->SetShadowRaymarchSteps(64);
			}
			event::Send(new GraphicsSettingsEvent(GraphicsSetting::LightQuality, value));
		} else if (key == "physicsQuality") {
			config["physicsQuality"] = value;
			int tps = GetPhysicsTPS(value);
			event::Send(new GraphicsSettingsEvent(GraphicsSetting::PhysicsQuality, tps));
		} else if (key == "displayMode") {
			auto mode = (value == "fullscreen") ? toast::DisplayMode::FULLSCREEN : toast::DisplayMode::WINDOWED;
			renderer->SetDisplayMode(mode);
			toast::Window::GetInstance()->SetDisplayMode(mode);
			config["displayMode"] = value;
			event::Send(new GraphicsSettingsEvent(GraphicsSetting::DisplayMode, value));
		} else if (key == "resolution") {
			// Parse "1920x1080" format
			size_t x = value.find('x');
			if (x != std::string::npos) {
				unsigned w = std::stoul(value.substr(0, x));
				unsigned h = std::stoul(value.substr(x + 1));
				renderer->SetResolution({w, h});
				// Window resize is handled by ApplyRenderSettings
				config["resolution"] = {w, h};
				event::Send(new GraphicsSettingsEvent(GraphicsSetting::Resolution, glm::uvec2{w, h}));
			}
		} else if (key == "vsync") {
			bool enabled = (value == "true");
			renderer->SetVSyncEnabled(enabled);
			config["vsync"] = enabled;
			event::Send(new GraphicsSettingsEvent(GraphicsSetting::VSync, enabled));
		} else if (key == "maxFps") {
			if (value == "unlimited") {
				renderer->SetMaxFPS(0);
				config["maxFps"] = 0;
				event::Send(new GraphicsSettingsEvent(GraphicsSetting::MaxFPS, 0));
			} else {
				int fps = std::stoi(value);
				renderer->SetMaxFPS(fps);
				config["maxFps"] = fps;
				event::Send(new GraphicsSettingsEvent(GraphicsSetting::MaxFPS, fps));
			}
		}
	}

	// Apply all changes to renderer
	renderer->ApplyRenderSettings();
	renderer::SaveRendererSettings();

	Stats::Save();

	s_stagedGraphics.clear();

	TOAST_INFO("Graphics settings applied");
}

void SettingsHandler::LoadSavedSettings() {
	try {
		// Load audio settings
		if (Stats::stats.contains("audio")) {
			auto& audio = Stats::stats["audio"];
			if (audio.contains("musicVolume")) {
				audio::set_music_volume(audio["musicVolume"].get<float>());
			}
			if (audio.contains("effectsVolume")) {
				audio::set_effects_volume(audio["effectsVolume"].get<float>());
			}
			if (audio.contains("audioMode")) {
				std::string mode = audio["audioMode"].get<std::string>();
				audio::AudioMode audioMode = audio::AudioMode::Stereo;
				if (mode == "mono") audioMode = audio::AudioMode::Mono;
				else if (mode == "binaural") audioMode = audio::AudioMode::Binaural;
				else if (mode == "surround") audioMode = audio::AudioMode::Surround;
				audio::set_audio_mode(audioMode);
			}
		}

		TOAST_INFO("Settings loaded from save data");
	} catch (const std::exception& e) {
		TOAST_WARN("Failed to load some settings: {}", e.what());
	}
}

void SettingsHandler::PopulateResolutionOptions(EvalJSFunc evalJS) {
	if (!evalJS) {
		return;
	}
	evalJS("if (typeof window.refreshResolutionOptions === 'function') window.refreshResolutionOptions();");
}

void SettingsHandler::PushLocalization(EvalJSFunc evalJS, const std::string& language) {
	if (!evalJS) {
		return;
	}
	const std::string safeLanguage = CoerceSupportedLanguage(language);
	const auto languageMap = Localization::BuildLanguageMap(safeLanguage);
	std::string script = "if (typeof window.setLocalizationData === 'function') window.setLocalizationData(" + languageMap.dump() + ", " +
	                     json_t(safeLanguage).dump() + ");";
	evalJS(script);
}


}    // namespace game
