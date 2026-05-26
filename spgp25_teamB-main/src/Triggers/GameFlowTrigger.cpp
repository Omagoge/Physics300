#include "GameFlowTrigger.hpp"

#include "Localization.hpp"
#include "MainMenu.hpp"
#include "Player/GameplayManager.hpp"
#include "Player/Player.hpp"
#include "Stats.hpp"
#include "Toast/Components/HtmlView.hpp"
#include "Toast/CoroutineHandler.hpp"
#include "Toast/GameEvents.hpp"
#include "Toast/Log.hpp"
#include "Toast/Objects/Object.hpp"
#include "Toast/Physics/Trigger.hpp"
#include "Toast/Renderer/HUD/HUDLayer.hpp"
#include "Toast/Time.hpp"
#include "Toast/WaitAsync.hpp"
#include "Toast/Window/Window.hpp"
#include "Toast/World.hpp"

#ifdef TOAST_EDITOR
#include "imgui.h"
#endif

namespace game {

void GameFlowTrigger::OnEnter(toast::Object* obj) {
	if (not m.enabled) {
		return;
	}
	if (auto* p = dynamic_cast<Player*>(obj)) {
		TOAST_WARN("NEXT LEVEL TRIGGER");
		input::SetLayout("null");

		p->enabled(false);
		m.enabled = false;    // Logical disable to prevent re-entry

		if (usePath) {
			// do smth else
			m.pending = LoadFromPath;
			return;
		}

		if (Stats::stats["best_time"].contains(scene()->name())) {
			int best_time = Stats::stats["best_time"][scene()->name()];
			if (best_time > static_cast<int>(p->milliseconds * 1000)) {
				Stats::stats["best_time"][scene()->name()] = static_cast<int>(p->milliseconds * 1000);
			}
		} else {
			Stats::stats["best_time"][scene()->name()] = static_cast<int>(p->milliseconds * 1000);
		}
		Stats::Save();

		[](GameFlowTrigger* t, Player* p) -> toast::CoroutineTask {
			auto& end_title = *toast::World::GetChildren().Add<toast::HtmlView>("Title");
			t->m.view = &end_title;

			end_title.SetConsoleCallback([t](const std::string& msg) {
				if (msg.find("[Menu] next-level") != std::string::npos) {
					t->m.pending = NextLevel;
				}

				if (msg.find("[Menu] main-menu") != std::string::npos) {
					t->m.pending = Null;
				}

				if (msg.find("[Menu] restart-level") != std::string::npos) {
					t->m.pending = Reset;
				}
			});

			end_title.SetUrl("file:///assets/UI/end_level_title.html");

			std::string language = "en";
			if (Stats::stats.contains("language") && Stats::stats["language"].is_string()) {
				language = Stats::stats["language"].get<std::string>();
			}
			language = Localization::NormalizeLanguage(language);
			if (!Localization::IsSupportedLanguage(language)) {
				language = "en";
			}
			const auto languageMap = Localization::BuildLanguageMap(language);
			end_title.EvalJS(
			    std::format("if(typeof setLocalizationData==='function') setLocalizationData({}, {});", languageMap.dump(), json_t(language).dump())
			);

			co_await toast::WaitSeconds(0.6);

			end_title.EvalJS(
			    std::format("setTimes({},{})", static_cast<int>(p->milliseconds * 1000), Stats::stats["best_time"][t->scene()->name()].get<int>())
			);
			end_title.EvalJS("fadeTextIn()");
			
			toast::Window::GetInstance()->SetShowMouseCursor(true);
			
			input::SetLayout("ui");
		}(this, p);

		m.input.Subscribe2D("move", [this](auto* a) {
			if (m.view) {
				OnMove(a, *m.view);
			}
		});
		m.input.Subscribe0D("select", [this](auto* a) {
			if (m.view) {
				OnSelect(a, *m.view);
			}
		});
	}
}

void GameFlowTrigger::Tick() {
	physics::Trigger::Tick();

	if (m.pending == LoadFromPath) {
		// Hope This Works
		input::SetState("null");
		m.pending = (GameFlowType)-1;    // Reset
		[](GameFlowTrigger& t) -> toast::CoroutineTask {
			toast::Window::GetInstance()->SetShowMouseCursor(false);
			renderer::HUD::HUDLayer::Get()->ExecuteJS("fadeIn()");
			co_await toast::WaitSeconds(0.75f);
			toast::World::LoadSceneSync(t.path);
			t.scene()->Nuke();
			co_await toast::WaitSeconds(0.25f);
			renderer::HUD::HUDLayer::Get()->ExecuteJS("fadeOut()");
		}(*this);
		return;
	}

	if (m.pending == Reset) {
		toast::Window::GetInstance()->SetShowMouseCursor(false);
		m.pending = (GameFlowType)-1;    // Reset
		input::SetLayout("null");

		m.input.Unsubscribe0D("select");
		m.input.Unsubscribe2D("move");
		[](auto& t) -> toast::CoroutineTask {
			renderer::HUD::HUDLayer::Get()->ExecuteJS("fadeIn()");
			co_await toast::WaitSeconds(0.6f);
			t.m.view->Nuke();
			event::Send(new toast::RestartLevel());
			co_await toast::WaitSeconds(0.1f);
			renderer::HUD::HUDLayer::Get()->ExecuteJS("fadeOut()");
		}(*this);
	}

	if (m.pending == NextLevel) {
		toast::Window::GetInstance()->SetShowMouseCursor(false);
		input::SetLayout("null");

		// dante at least check what copilot is doing
		m.input.Unsubscribe0D("select");
		m.input.Unsubscribe2D("move");
		m.pending = (GameFlowType)-1;    // Reset
		event::Send(new toast::NextLevel());
		m.view->Nuke();
		return;
	}

	if (m.pending == NextWorld) {
		if(Stats::stats["current_world"].get<int>()) {
			Stats::stats["current_world"];
		}
		else {
			Stats::stats["current_world"] = 1;
		}
	}

	if (m.pending == Null && not m.enabled) {
		toast::Window::GetInstance()->SetShowMouseCursor(false);
		m.pending = (GameFlowType)-1;
		input::SetLayout("null");

		m.input.Unsubscribe0D("select");
		m.input.Unsubscribe2D("move");
		event::Send(new toast::ResetGameFlow());
		// if (auto* ps = toast::World::Get("PlayerScene")) {
		// 	ps->Nuke();
		// }
		// toast::World::LoadSceneSync("assets/SCENES/MainMenu.scene");

		try {
			switch (game::Stats::stats["current_world"].get<int>()) {
				case 1: toast::World::LoadSceneSync("SCENES/HUB_1.scene"); break;
				case 2: toast::World::LoadSceneSync("SCENES/HUB_2.scene"); break;
				case 3: toast::World::LoadSceneSync("SCENES/HUB_3.scene"); break;
				default: toast::World::LoadSceneSync("SCENES/MainMenu.scene"); break;
			}
		} catch (...) {
			// fallback if missing save data
			toast::World::LoadSceneSync("SCENES/MainMenu.scene");
		}

		m.view->Nuke();
		return;
	}

	// Handle held-down stick/key repeat
	if (m.movedir != 0) {
		m.moveheldtime += static_cast<float>(Time::delta());
		if (m.view && m.moveheldtime >= m.nextrepeattime) {
			if (m.movedir > 0) {
				m.view->EvalJS("if(typeof menuMoveUp==='function') menuMoveUp();");
			} else {
				m.view->EvalJS("if(typeof menuMoveDown==='function') menuMoveDown();");
			}
			m.nextrepeattime = m.moveheldtime + krepeatrate;
		}
	}
}

void GameFlowTrigger::OnMove(const input::Action2D* a, toast::HtmlView& v) {
	if (!a) {
		return;
	}

	const float y = a->value.y;

	if (std::abs(y) < kdeadzone) {
		m.movedir = 0;
		m.moveheldtime = 0.0f;
		m.nextrepeattime = 0.0f;
		return;
	}

	int dir = (y > 0.0f) ? 1 : -1;

	if (dir != m.movedir) {
		m.movedir = dir;
		m.moveheldtime = 0.0f;
		m.nextrepeattime = kinitialdelay;

		if (dir > 0) {
			v.EvalJS("if(typeof menuMoveUp==='function') menuMoveUp();");
		} else {
			v.EvalJS("if(typeof menuMoveDown==='function') menuMoveDown();");
		}
	}
}

void GameFlowTrigger::OnSelect(const input::Action0D* a, toast::HtmlView& v) {
	if (!a) {
		return;
	}

	if (a->state == input::Action0D::Started) {
		v.EvalJS("if(typeof menuActivate==='function') menuActivate();");
	} else if (a->state == input::Action0D::Finished) {
		v.EvalJS("if(typeof menuSelect==='function') menuSelect();");
	}
}

#ifdef TOAST_EDITOR
void GameFlowTrigger::Inspector() {
	physics::Trigger::Inspector();
	const char* items[] { "Null", "NextLevel", "NextWorld" };
	ImGui::Combo("Weapon Type", (int*)&m.type, items, IM_ARRAYSIZE(items));
}
#endif

void GameFlowTrigger::Load(json_t j, bool force_create) {
	Trigger::Load(j, force_create);
	m.type = j["GameFlowType"];
	m.enabled = true;
}

json_t GameFlowTrigger::Save() const {
	json_t j = Trigger::Save();
	j["GameFlowType"] = m.type;
	return j;
}

}
