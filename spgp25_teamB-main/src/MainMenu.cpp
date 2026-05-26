//
// Created by xein on 11/3/25.
//

#include "MainMenu.hpp"

#include "Localization.hpp"
#include "SettingsHandler.hpp"
#include "Stats.hpp"
#include "Toast/CoroutineHandler.hpp"
#include "Toast/Localization.hpp"
#include "Toast/Renderer/Camera.hpp"
#include "Toast/Renderer/IRendererBase.hpp"
#include "Toast/Resources/ResourceManager.hpp"
#include "Toast/Resources/Spine/SpineEvent.hpp"
#include "Toast/WaitAsync.hpp"

#include <Toast/Components/HtmlView.hpp>
#include <Toast/Event/ListenerComponent.hpp>
#include <Toast/GameEvents.hpp>
#include <Toast/Log.hpp>
#include <Toast/Renderer/HUD/HUDLayer.hpp>
#include <Toast/Renderer/HUD/ShowHUDLayer.h>
#include <Toast/Time.hpp>
#include <Toast/Window/WindowEvents.hpp>
#include <Toast/World.hpp>
#include <Ultralight/Ultralight.h>

#ifdef TOAST_EDITOR
#include <Toast/SimulateWorldEvent.hpp>
#endif
#include "Stats.hpp"

namespace game {

bool intro_sequence = true;

namespace {
auto SumNumericValues(const json_t& value) -> long long {
	if (value.is_number_integer()) {
		return value.get<long long>();
	}
	if (value.is_number_float()) {
		return static_cast<long long>(value.get<double>());
	}
	if (!value.is_object()) {
		return 0;
	}

	long long total = 0;
	for (const auto& item : value.items()) {
		const auto& v = item.value();
		if (v.is_number_integer()) {
			total += v.get<long long>();
		} else if (v.is_number_float()) {
			total += static_cast<long long>(v.get<double>());
		}
	}
	return total;
}

auto MaxNumericValues(const json_t& value) -> long long {
	if (value.is_number_integer()) {
		return value.get<long long>();
	}
	if (value.is_number_float()) {
		return static_cast<long long>(value.get<double>());
	}
	if (!value.is_object()) {
		return 0;
	}

	long long currentMax = 0;
	for (const auto& item : value.items()) {
		const auto& v = item.value();
		if (v.is_number_integer()) {
			currentMax = std::max(currentMax, v.get<long long>());
		} else if (v.is_number_float()) {
			currentMax = std::max(currentMax, static_cast<long long>(v.get<double>()));
		}
	}
	return currentMax;
}

auto FormatMilliseconds(long long ms) -> std::string {
	const long long totalSeconds = std::max(0LL, ms / 1000);
	const long long hours = totalSeconds / 3600;
	const long long minutes = (totalSeconds % 3600) / 60;
	const long long seconds = totalSeconds % 60;
	return std::format("{:02}:{:02}:{:02}", hours, minutes, seconds);
}

void EnsureLocalizationReady() {
	if (toast::Localization::GetLanguages().empty()) {
		if (!toast::Localization::LoadFile("UI/locales.json")) {
			toast::Localization::LoadFile("assets/UI/locales.json");
		}
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

	const std::string active = toast::Localization::GetLanguage();
	if (!game::Stats::stats.contains("language") || game::Stats::stats["language"] != active) {
		game::Stats::stats["language"] = active;
		game::Stats::Save();
	}
}
}    // namespace

void MainMenu::Init() {
	Scene::Init();
	EnsureLocalizationReady();
	// Reuse existing HtmlView if loaded from scene file, otherwise create one
	m_view = children.Get<toast::HtmlView>();
	if (!m_view) {
		m_view = children.AddRequired<toast::HtmlView>("MenuView");
	}
	if (m_view) {
		m_view->SetConsoleCallback([this](const std::string& msg) {
			OnConsoleMessage(msg);
		});

		if (intro_sequence) {
			m_view->SetUrl("file:///assets/UI/intro.html");
			intro_sequence = false;
		} else {
			m_view->SetUrl("file:///assets/UI/menus/MainMenu.html");
		}
	}

	m_cutsceneSpine = scene()->children.AddRequired<SpineRendererComponent>();

	auto atlas = resource::ResourceManager::GetInstance()->LoadResource<SpineAtlas>("UI/cinematic/Cinematic_cat_city.atlas");
	auto skeleton = resource::ResourceManager::GetInstance()->LoadResource<SpineSkeletonData>("UI/cinematic/Cinematic_cat_city.json", atlas);
	m_cutsceneSpine->SetSkeletonData(skeleton);
	m_cutsceneSpine->position(glm::vec3(0.0f, 0.0f, 1.f));
	m_cutsceneSpine->scale(glm::vec3(1.1f, 1.1f, 1.f));
	m_cutsceneSpine->enabled(false);

	m_cam = scene()->children.AddRequired<toast::Camera>();
	m_cam->transform()->position(glm::vec3(19.2f, 10.8f, 10.0f));

	m_eventListener = children.AddRequired<event::ListenerComponent>();
	m_eventListener->Subscribe<SpineAnimationPlaybackEvent>([this](SpineAnimationPlaybackEvent* e) -> bool {
		if (e->uniqueID != m_cutsceneSpine->id()) {
			return false;
		}

		if (e->playbackType == SpineAnimationPlaybackEvent::Type::Complete) {
			HandlePlay();
		}
		return true;
	});
}

void MainMenu::Begin() {
	m_isPlayingCutscene = false;
	m_pendingAction = PendingAction::None;
	Scene::Begin();

	static bool settingsLoaded = false;
	if (!settingsLoaded) {
		SettingsHandler::LoadSavedSettings();
		settingsLoaded = true;
	}

	if (!toast::World::Has("Fade")) {
		toast::World::LoadSceneSync("SCENES/Fade.scene");
	}

	input::SetLayout("ui");

	m_input.Subscribe2D("move", [this](auto* a) {
		OnMove(a);
	});
	m_input.Subscribe0D("select", [this](auto* a) {
		OnSelect(a);
	});
	m_input.Subscribe0D("back", [this](auto* a) {
		OnBack(a);
	});

	renderer::IRendererBase::GetInstance()->SetActiveCamera(m_cam);

	event::Send(new toast::LoadWorld(0));
	
	// Set fullscreen by default
	toast::Window::GetInstance()->SetDisplayMode(toast::DisplayMode::FULLSCREEN);
}

void MainMenu::Tick() {
	Scene::Tick();

	float delta = Time::delta();
	wastedTime += delta;
	// Execute deferred actions (avoid modifying world during callbacks)
	if (m_pendingAction == PendingAction::Play) {
		if (Stats::stats.contains("wastedTime")) {
			int time = Stats::stats["wastedTime"];
			time += static_cast<int>(wastedTime * 1000);
			Stats::stats["wastedTime"] = time;
		}
		else {
			Stats::stats["wastedTime"] = static_cast<int>(wastedTime * 1000);
		}
		Stats::Save();
		m_pendingAction = PendingAction::None;

		if (m_isPlayingCutscene) {
			return;
		}

		m_cutsceneSpine->enabled(true);
		m_cutsceneSpine->PlayAnimation("cinematic_opening", false, 0);
		if (m_view) {
			m_view->Nuke();
			m_view = nullptr;
		}
		m_isPlayingCutscene = true;
		return;
	}
	if (m_pendingAction == PendingAction::Exit) {
		m_pendingAction = PendingAction::None;
		HandleExit();
		return;
	}
	if (m_pendingAction == PendingAction::Credits) {
		m_pendingAction = PendingAction::None;
		HandleCredits();
		return;
	}

	// Handle held-down stick/key repeat
	if (m_moveDir != 0) {
		m_moveHeldTime += static_cast<float>(Time::delta());
		if (m_moveHeldTime >= m_nextRepeatTime) {
			if (m_moveDir > 0) {
				EvalJS("if(typeof menuMoveUp==='function') menuMoveUp();");
			} else {
				EvalJS("if(typeof menuMoveDown==='function') menuMoveDown();");
			}
			m_nextRepeatTime = m_moveHeldTime + kRepeatRate;
		}
	}

	// Handle key repeat
	if (m_moveDirH != 0) {
		m_moveHeldTimeH += static_cast<float>(Time::delta());
		if (m_moveHeldTimeH >= m_nextRepeatTimeH) {
			if (m_moveDirH > 0) {
				EvalJS("if(typeof menuMoveRight==='function') menuMoveRight();");
			} else {
				EvalJS("if(typeof menuMoveLeft==='function') menuMoveLeft();");
			}
			m_nextRepeatTimeH = m_moveHeldTimeH + kSliderRepeatRate;
		}
	}
}

void MainMenu::Destroy() {
	m_view = nullptr;
}

void MainMenu::OnMove(const input::Action2D* a) {
	if (!a) {
		return;
	}
	UpdateInputModeFromDevice(a->device);

	const float y = a->value.y;
	if (std::abs(y) < kDeadzone) {
		m_moveDir = 0;
		m_moveHeldTime = 0.0f;
		m_nextRepeatTime = 0.0f;
	} else {
		int dir = (y > 0.0f) ? 1 : -1;
		if (dir != m_moveDir) {
			m_moveDir = dir;
			m_moveHeldTime = 0.0f;
			m_nextRepeatTime = kInitialDelay;
			if (dir > 0) {
				EvalJS("if(typeof menuMoveUp==='function') menuMoveUp();");
			} else {
				EvalJS("if(typeof menuMoveDown==='function') menuMoveDown();");
			}
		}
	}

	// Slider movement
	const float x = a->value.x;
	if (std::abs(x) < kDeadzone) {
		m_moveDirH = 0;
		m_moveHeldTimeH = 0.0f;
		m_nextRepeatTimeH = 0.0f;
	} else {
		int dirH = (x > 0.0f) ? 1 : -1;
		if (dirH != m_moveDirH) {
			m_moveDirH = dirH;
			m_moveHeldTimeH = 0.0f;
			m_nextRepeatTimeH = kInitialDelay;
			if (dirH > 0) {
				EvalJS("if(typeof menuMoveRight==='function') menuMoveRight();");
			} else {
				EvalJS("if(typeof menuMoveLeft==='function') menuMoveLeft();");
			}
		}
	}
}

void MainMenu::OnSelect(const input::Action0D* a) {
	if (!a) {
		return;
	}
	UpdateInputModeFromDevice(a->device);

	if (a->state == input::Action0D::Started) {
		EvalJS("if(typeof menuActivate==='function') menuActivate();");
	} else if (a->state == input::Action0D::Finished) {
		EvalJS("if(typeof menuSelect==='function') menuSelect();");
		// skip cutscene if already playing
		if (m_isPlayingCutscene) {
			HandlePlay();
		}
	}
}

void MainMenu::OnBack(const input::Action0D* a) {
	if (!a || a->state != input::Action0D::Started) {
		return;
	}
	UpdateInputModeFromDevice(a->device);

	// Controls page exits via accept/select (A), not back/cancel (B).
	if (m_isControlsScreen) {
		return;
	}

	EvalJS("if(typeof menuBack==='function') menuBack();");
}

void MainMenu::OnConsoleMessage(const std::string& msg) {
	// First try the SettingsHandler for [Settings] messages
	if (SettingsHandler::ProcessMessage(msg, [this](const std::string& js) {
		EvalJS(js);
	})) {
		return;
	}

	if (msg == "[Credits] init") {
		const auto& s = Stats::stats;
		const long long totalTime = SumNumericValues(s.value("best_time", json_t::object()));
		const long long totalAir = SumNumericValues(s.value("air_time", json_t::object()));
		const long long totalGravity = SumNumericValues(s.value("grav_time", json_t::object()));
		const long long wastedTime = SumNumericValues(s.value("wastedTime", 0));
		const long long maxSpeed = MaxNumericValues(s.value("max_speed", json_t::object()));
		const long long enemiesKilled = SumNumericValues(s.value("enemy_kills", json_t::object()));
		const long long lostHealth = SumNumericValues(s.value("hit_number", json_t::object()));

		json_t payload = {
			{ "time", FormatMilliseconds(totalTime) },
			{ "time_on_air", FormatMilliseconds(totalAir) },
			{ "time_on_gravity", FormatMilliseconds(totalGravity) },
			{ "time_wasted", FormatMilliseconds(wastedTime) },
			{ "maximum_speed", std::to_string(maxSpeed) },
			{ "enemies_killed", std::to_string(enemiesKilled) },
			{ "lost_health", std::to_string(lostHealth) }
		};

		EvalJS(std::format("if(typeof setStats==='function') setStats({});", payload.dump()));
		return;
	}

	if (msg.find("[Menu]") == std::string::npos) {
		return;
	}

	if (const std::string prefix = "[Menu] language:"; msg.rfind(prefix, 0) == 0) {
		const std::string language = msg.substr(prefix.size());
		if (toast::Localization::SetLanguage(language)) {
			game::Stats::stats["language"] = toast::Localization::GetLanguage();
			game::Stats::Save();
		}
		return;
	}

	if (msg == "[Menu] controls:init") {
		m_isControlsScreen = true;
		SyncControlsInputMode();
		return;
	}

	if (msg == "[Menu] controls:exit" || msg == "[Menu] main:init") {
		m_isControlsScreen = false;
		return;
	}

	if (msg == "[Menu] controls:keyboard_mouse") {
		SetInputMode("keyboard_mouse");
		return;
	}

	if (msg == "[Menu] controls:controller") {
		SetInputMode("controller");
		return;
	}

	if (msg.find("play") != std::string::npos) {
		[](MainMenu* m) -> toast::CoroutineTask {
			toast::HtmlView* fade = toast::World::Get<toast::HtmlView>("Fade_View");
			fade->EvalJS("fadeIn()");
			co_await toast::WaitSeconds(1);
			m->m_pendingAction = PendingAction::Play;
			fade->EvalJS("fadeOut()");
		}(this);
	} else if (msg.find("exit") != std::string::npos) {
		m_pendingAction = PendingAction::Exit;
	} else if (msg.find("credits") != std::string::npos) {
		m_pendingAction = PendingAction::Credits;
	} else if (msg.find("form") != std::string::npos) {
		std::string url = "https://docs.google.com/forms/d/e/1FAIpQLSf1eJm459fHgtI7iT7glUC3mfJMUWDAQnVNr7SCdo1pxxKGLA/viewform?usp=header";
#if defined(_WIN32) || defined(_WIN64)
		// Windows
		std::string command = "start ";
		std::system((command + url).c_str());
#elif defined(__APPLE__)
		// macOS
		std::string command = "open ";
		std::system((command + url).c_str());
#else
		// Linux/Unix
		std::string command = "xdg-open ";
		std::system((command + url).c_str());
#endif
	}
}

void MainMenu::HandlePlay() {
	TOAST_INFO("Entering Level From Main Menu");
	m_isPlayingCutscene = false;
	// Switch input layout so the player can aim immediately
	input::SetLayout("player");

	toast::HtmlView* fade = toast::World::Get<toast::HtmlView>("Fade_View");
	fade->EvalJS("fadeIn()");
	[](MainMenu& m) -> toast::CoroutineTask {
		// Load the player scene if not already present
		if (!toast::World::Has("PlayerScene")) {
			auto promise = toast::World::LoadScene("SCENES/PlayerScene.scene");
			co_await toast::WaitSeconds(1);
			promise.wait();
			toast::World::Get(promise.get())->enabled(true);
		}

		// Start game flow
		// event::Send(new toast::LoadLevel(0, 0));
		try {
			switch (game::Stats::stats["current_world"].get<int>()) {
			default:
			case 1:
				toast::World::LoadSceneSync("SCENES/HUB_1.scene");
				break;
			case 2:
				toast::World::LoadSceneSync("SCENES/HUB_2.scene");
				break;
			case 3:
				toast::World::LoadSceneSync("SCENES/HUB_3.scene");
				break;
			}
		} catch(...) {
			// fallback if missing save data
			// toast::World::LoadSceneSync("SCENES/HUB_1.scene");
			// we want to send the player to the first tutorial directly the first time
			event::Send(new toast::LoadLevel(0, 0));
			game::Stats::stats["current_world"] = 1;
		}

		renderer::HUD::HUDLayer::Get()->ExecuteJS("fadeOut()");

		// Disable the scene — triggers OnDisable on children (HtmlView destroys its view)
		// m.enabled(false);
		m.Nuke();
	}(*this);
}

void MainMenu::HandleCredits() {
	toast::World::LoadSceneSync("SCENES/Credits.scene");
	Nuke();
}

void MainMenu::HandleExit() {
	event::Send(new event::WindowClose());
}

void MainMenu::EvalJS(const std::string& script) {
	if (!m_view) {
		return;
	}
	auto ulView = m_view->GetView();
	if (ulView) {
		ulView->EvaluateScript(ultralight::String(script.c_str()));
	}
}

void MainMenu::UpdateInputModeFromDevice(input::Device device) {
	if (device == input::Device::Keyboard || device == input::Device::Mouse) {
		SetInputMode("keyboard_mouse");
		return;
	}

	SetInputMode("controller");
}

void MainMenu::SetInputMode(const std::string& mode) {
	if (mode != "controller" && mode != "keyboard_mouse") {
		return;
	}

	if (m_lastInputMode == mode) {
		return;
	}

	m_lastInputMode = mode;
	SyncControlsInputMode();
}

void MainMenu::SyncControlsInputMode() {
	EvalJS(std::format("if(typeof setControlsInputMode==='function') setControlsInputMode('{}');", m_lastInputMode));
}

}    // game
