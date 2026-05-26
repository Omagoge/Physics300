#include "App.hpp"
#include "Localization.hpp"
#include "Stats.hpp"

#include "Toast/Renderer/HUD/ShowHUDLayer.h"

#include <Toast/Localization.hpp>
#include <Toast/Objects/Scene.hpp>
#include <Toast/World.hpp>

// Objects
#include <Toast/Objects/Actor.hpp>
#include <Toast/Objects/Object.hpp>
#include <Toast/Objects/ParticleSystem.hpp>

// Renderer
#include <Toast/Renderer/Camera.hpp>
#include <Toast/Renderer/Lights/2DLight.hpp>
#include <Toast/Renderer/Lights/GlobalLight.hpp>

// Components
#include "Toast/Input/InputListener.hpp"
#include "Toast/Window/Window.hpp"

#include <Toast/Components/AtlasRendererComponent.hpp>
#include <Toast/Components/MeshRendererComponent.hpp>
#include <Toast/Components/SpineRendererComponent.hpp>
#include <Toast/Components/TransformComponent.hpp>
#include <Toast/Physics/Collider.hpp>
#include <Toast/Physics/Rigidbody.hpp>
#include <Toast/Audio/Audio.hpp>

#ifdef _WIN32
#include <Windows.h>
#endif

static std::multimap<toast::BaseType, std::string> m_classes;

void game::Game::Begin() {
	Engine::Begin();
	
	
	FreeConsole();
	
	
	// Show the HUD layer
	toast::Object::Children c;
	c.parent(nullptr);
	c.scene(nullptr);

	auto e = audio::load_bank("assets/AUDIO/Desktop/Music.bank");
	auto o = audio::load_bank("assets/AUDIO/Desktop/Master.strings.bank");
	auto s = audio::load_bank("assets/AUDIO/Desktop/Master.bank");
	if (!e.has_value()) {
		TOAST_ERROR("Error loading music bank");
	}

	audio::load_event("event:/City");
	audio::load_event("event:/Port");

	input::SetViewportPosition({0,0});
	input::SetViewportSize(toast::Window::GetInstance()->GetFramebufferSize());

	for (auto reg = toast::Object::getRegistry(); const auto& [type, allocator] : reg) {
		// Create a temporary object to query its base type
		auto* o = allocator(c, -1);    // This won't comsume ids now
		m_classes.emplace(o->base_type(), type);
	}
	
	event::Send(new ShowHUDLayerEvent(true));

	if (!toast::Localization::LoadFile("UI/locales.json")) {
		toast::Localization::LoadFile("assets/UI/locales.json");
	}

	std::string language = "en";
	if (game::Stats::stats.contains("language") && game::Stats::stats["language"].is_string()) {
		language = game::Stats::stats["language"].get<std::string>();
	}
	language = game::Localization::NormalizeLanguage(language);
	if (!game::Localization::IsSupportedLanguage(language)) {
		language = "en";
	}

	if (!toast::Localization::SetLanguage(language)) {
		language = "en";
		toast::Localization::SetLanguage(language);
	}

	if (!game::Stats::stats.contains("language") || game::Stats::stats["language"] != language) {
		game::Stats::stats["language"] = language;
		game::Stats::Save();
	}

	//
	toast::World::LoadSceneSync("SCENES/MainMenu.scene");
	// 
	// event::Send(new toast::LoadWorld(0));
	// event::Send(new toast::NextLevel());
	

	
}
