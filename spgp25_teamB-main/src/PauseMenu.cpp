/// @file PauseMenu.cpp
/// @brief Pause menu component implementation

#include "PauseMenu.hpp"

#include "Localization.hpp"
#include "Player/GameplayManager.hpp"
#include "Player/Player.hpp"
#include "SettingsHandler.hpp"
#include "Stats.hpp"
#include "Toast/CoroutineHandler.hpp"
#include "Toast/Event/ListenerComponent.hpp"
#include "Toast/Localization.hpp"
#include "Toast/Objects/Scene.hpp"
#include "Toast/WaitAsync.hpp"

#include <Toast/GameEvents.hpp>
#include <Toast/Log.hpp>
#include <Toast/Renderer/HUD/HUDLayer.hpp>
#include <Toast/Time.hpp>
#include <Toast/Window/WindowEvents.hpp>
#include <Toast/World.hpp>
#include <Toast/Audio/Audio.hpp>

PauseMenu* PauseMenu::s_instance = nullptr;

namespace {
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

PauseMenu* PauseMenu::Get() {
	return s_instance;
}

void PauseMenu::Init() {
	HtmlView::Init();
	s_instance = this;
	EnsureLocalizationReady();

	SetConsoleCallback([this](const std::string& msg) {
		OnConsoleMessage(msg);
	});

	m_input.Subscribe2D("move", [this](auto* a) {
		if (!enabled()) {
			return;
		}
		OnMove(a);
	});
	m_input.Subscribe0D("select", [this](auto* a) {
		if (!enabled()) {
			return;
		}
		OnSelect(a);
	});
	m_input.Subscribe0D("back", [this](auto* a) {
		if (!enabled()) {
			return;
		}
		OnBack(a);
	});
}

void PauseMenu::Destroy() {
	HtmlView::Destroy();
	if (s_instance == this) {
		s_instance = nullptr;
	}
}

void PauseMenu::OnEnable() {
	// Ensure Begin() runs if it was skipped (component started disabled)
	if (!has_run_begin()) {
		RefreshBegin(false);
	}

	HtmlView::OnEnable();

	Time::scale(0.0f);
	input::SetLayout("ui");

	if (auto* player = GameplayManager::GetPlayer()) {
		player->SetPlayerRecivesInput(false);
	}

	m_moveDir = 0;
	m_moveHeldTime = 0.0f;
	m_nextRepeatTime = 0.0f;
	
	toast::Window::GetInstance()->SetShowMouseCursor(true);
}

void PauseMenu::OnDisable() {
	HtmlView::OnDisable();

	Time::scale(1.0f);
	input::SetLayout("player");

	if (auto* player = GameplayManager::GetPlayer()) {
		player->SetPlayerRecivesInput(true);
	}
	
	toast::Window::GetInstance()->SetShowMouseCursor(false);
}

void PauseMenu::Tick() {
	if (!enabled()) {
		return;
	}

	if (m_pendingAction == PendingAction::Continue) {
		m_pendingAction = PendingAction::None;
		HandleContinue();
		return;
	}
	if (m_pendingAction == PendingAction::SkipScene) {
		m_pendingAction = PendingAction::None;
		HandleSkipScene();
		return;
	}
	if (m_pendingAction == PendingAction::MainMenu) {
		m_pendingAction = PendingAction::None;
		HandleMainMenu();
		return;
	}
	if (m_pendingAction == PendingAction::Exit) {
		m_pendingAction = PendingAction::None;
		HandleExit();
		return;
	}

	// Input repeat for held stick/key (use raw_delta since game time is frozen)
	if (m_moveDir != 0) {
		m_moveHeldTime += static_cast<float>(Time::raw_delta());
		if (m_moveHeldTime >= m_nextRepeatTime) {
			if (m_moveDir > 0) {
				EvalJS("if(typeof menuMoveUp==='function') menuMoveUp();");
			} else {
				EvalJS("if(typeof menuMoveDown==='function') menuMoveDown();");
			}
			m_nextRepeatTime = m_moveHeldTime + kRepeatRate;
		}
	}

	// Horizontal input repeat for sliders
	if (m_moveDirH != 0) {
		m_moveHeldTimeH += static_cast<float>(Time::raw_delta());
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

// ── Input handlers ──────────────────────────────────────────────────────────

void PauseMenu::OnMove(const input::Action2D* a) {
	if (!a) {
		return;
	}

	// Vertical movement
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

	// slidder
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

void PauseMenu::OnSelect(const input::Action0D* a) {
	if (!a) {
		return;
	}

	if (a->state == input::Action0D::Started) {
		EvalJS("if(typeof menuActivate==='function') menuActivate();");
	} else if (a->state == input::Action0D::Finished) {
		EvalJS("if(typeof menuSelect==='function') menuSelect();");
	}
}

void PauseMenu::OnBack(const input::Action0D* a) {
	if (!a || a->state != input::Action0D::Started) {
		return;
	}
	EvalJS("if(typeof menuBack==='function') menuBack();");
}

// ── Console message handler ─────────────────────────────────────────────────

void PauseMenu::OnConsoleMessage(const std::string& msg) {
	// First try the SettingsHandler for [Settings] messages
	if (game::SettingsHandler::ProcessMessage(msg, [this](const std::string& js) {
		EvalJS(js);
	})) {
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

	if (msg.find("continue") != std::string::npos) {
		input::SetLayout("null");
		audio::set_param("event:/City", "event:/Paused", 0);
		audio::set_param("event:/Port", "event:/Paused", 0);
		m_pendingAction = PendingAction::Continue;
	} else if (msg.find("skipScene") != std::string::npos) {
		input::SetLayout("null");
		audio::set_param("event:/City", "event:/Paused", 0);
		audio::set_param("event:/Port", "event:/Paused", 0);
		m_pendingAction = PendingAction::SkipScene;
	} else if (msg.find("mainMenuButton") != std::string::npos) {
		input::SetLayout("null");
		[](auto& t) -> toast::CoroutineTask {
			renderer::HUD::HUDLayer::Get()->ExecuteJS("fadeIn()");
			co_await toast::WaitSeconds(0.6f);
			t.m_pendingAction = PendingAction::MainMenu;
			co_await toast::WaitSeconds(0.1f);
			renderer::HUD::HUDLayer::Get()->ExecuteJS("fadeOut()");
			audio::set_param("event:/City", "event:/Paused", 0);
			audio::set_param("event:/Port", "event:/Paused", 0);
			audio::set_param("event:/City", "event:/Level End", 1);
			audio::set_param("event:/Port", "event:/Level End", 1);
		}(*this);
	} else if (msg.find("exit") != std::string::npos) {
		input::SetLayout("null");
		m_pendingAction = PendingAction::Exit;
	} else if (msg.find("retry") != std::string::npos) {
		input::SetLayout("null");
		[](auto& t) -> toast::CoroutineTask {
			renderer::HUD::HUDLayer::Get()->ExecuteJS("fadeIn()");
			co_await toast::WaitSeconds(0.6f);
			t.m_pendingAction = PendingAction::Continue;
			event::Send(new toast::RestartLevel());
			co_await toast::WaitSeconds(0.1f);
			audio::set_param("event:/City", "event:/Paused", 0);
			audio::set_param("event:/Port", "event:/Paused", 0);
			renderer::HUD::HUDLayer::Get()->ExecuteJS("fadeOut()");
		}(*this);
	}
}

// ── Action handlers ─────────────────────────────────────────────────────────

void PauseMenu::HandleContinue() {
	enabled(false);
}

void PauseMenu::HandleSkipScene() {
	enabled(false);
	event::Send(new toast::NextLevel());
}

void PauseMenu::HandleMainMenu() {
	// s_instance = nullptr;
	enabled(false);

	// Reset game flow
	event::Send(new toast::ResetGameFlow());

	// Destroy the player scene
	// if (auto* ps = toast::World::Get("PlayerScene")) {
	// 	ps->Nuke();
	// }

	// Re-enable the main menu scene
	// if (auto* mm = toast::World::GetFromType("MainMenu")) {
	// 	mm->enabled(true);
	// }
	// toast::World::New("MainMenu")->enabled(true);

	try {
		switch (game::Stats::stats["current_world"].get<int>()) {
		case 1:
			toast::World::LoadSceneSync("SCENES/HUB_1.scene");
			break;
		case 2:
			toast::World::LoadSceneSync("SCENES/HUB_2.scene");
			break;
		case 3:
			toast::World::LoadSceneSync("SCENES/HUB_3.scene");
			break;
		default:
			toast::World::LoadSceneSync("SCENES/MainMenu.scene");
			break;
		}
	} catch(...) {
		// fallback if missing save data
		toast::World::LoadSceneSync("SCENES/MainMenu.scene");
	}

	// input::SetLayout("ui");
}

void PauseMenu::HandleExit() {
	Time::scale(1.0f);
	event::Send(new event::WindowClose());
}
