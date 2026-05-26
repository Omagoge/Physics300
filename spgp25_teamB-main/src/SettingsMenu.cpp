#include "SettingsMenu.hpp"
#include "SettingsHandler.hpp"

#include <Toast/Components/HtmlView.hpp>
#include <Toast/Log.hpp>
#include <Toast/Renderer/Camera.hpp>
#include <Toast/Renderer/IRendererBase.hpp>
#include <Toast/Time.hpp>
#include <Toast/World.hpp>
#include <Ultralight/Ultralight.h>

namespace game {

void SettingsMenu::Init() {
	Scene::Init();

	m_view = children.Get<toast::HtmlView>();
	if (!m_view) {
		m_view = children.AddRequired<toast::HtmlView>("SettingsView");
	}

	if (m_view) {
		m_view->SetConsoleCallback([this](const std::string& msg) {
			OnConsoleMessage(msg);
		});
		m_view->SetUrl("file:///assets/UI/menus/Settings.html");
	}

	m_cam = scene()->children.AddRequired<toast::Camera>();
	m_cam->transform()->position(glm::vec3(0.0f, 0.0f, 5.25f));
}

void SettingsMenu::Begin() {
	Scene::Begin();

	input::SetLayout("ui");

	m_input.Subscribe2D("move", [this](auto* a) {
		OnMove(a);
		OnMoveHorizontal(a);
	});
	m_input.Subscribe0D("select", [this](auto* a) {
		OnSelect(a);
	});
	m_input.Subscribe0D("back", [this](auto* a) {
		OnBack(a);
	});

	renderer::IRendererBase::GetInstance()->SetActiveCamera(m_cam);

	m_uiPopulated = false;
}

void SettingsMenu::Tick() {
	Scene::Tick();

	if (!m_uiPopulated && m_view) {
		PopulateUI();
		m_uiPopulated = true;
	}

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

void SettingsMenu::Destroy() {
	m_view = nullptr;
}

void SettingsMenu::OnMove(const input::Action2D* a) {
	if (!a) return;

	const float y = a->value.y;

	if (std::abs(y) < kDeadzone) {
		m_moveDir = 0;
		m_moveHeldTime = 0.0f;
		m_nextRepeatTime = 0.0f;
		return;
	}

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

void SettingsMenu::OnMoveHorizontal(const input::Action2D* a) {
	if (!a) return;

	const float x = a->value.x;

	if (std::abs(x) < kDeadzone) {
		m_moveDirH = 0;
		m_moveHeldTimeH = 0.0f;
		m_nextRepeatTimeH = 0.0f;
		return;
	}

	int dir = (x > 0.0f) ? 1 : -1;

	if (dir != m_moveDirH) {
		m_moveDirH = dir;
		m_moveHeldTimeH = 0.0f;
		m_nextRepeatTimeH = kInitialDelay;

		if (dir > 0) {
			EvalJS("if(typeof menuMoveRight==='function') menuMoveRight();");
		} else {
			EvalJS("if(typeof menuMoveLeft==='function') menuMoveLeft();");
		}
	}
}

void SettingsMenu::OnSelect(const input::Action0D* a) {
	if (!a) return;

	if (a->state == input::Action0D::Started) {
		EvalJS("if(typeof menuActivate==='function') menuActivate();");
	} else if (a->state == input::Action0D::Finished) {
		EvalJS("if(typeof menuSelect==='function') menuSelect();");
	}
}

void SettingsMenu::OnBack(const input::Action0D* a) {
	if (!a || a->state != input::Action0D::Started) return;
	EvalJS("if(typeof menuBack==='function') menuBack();");
}

void SettingsMenu::OnConsoleMessage(const std::string& msg) {
	// First try the SettingsHandler for [Settings] messages
	if (SettingsHandler::ProcessMessage(msg, [this](const std::string& js) {
		EvalJS(js);
	})) {
		return;
	}

	// Handle navigation messages
	if (msg.find("[Menu]") != std::string::npos) {
		if (msg.find("back") != std::string::npos) {
			HandleBack();
		}
	}
}

void SettingsMenu::HandleBack() {}

void SettingsMenu::EvalJS(const std::string& script) {
	if (!m_view) return;
	auto ulView = m_view->GetView();
	if (ulView) {
		ulView->EvaluateScript(ultralight::String(script.c_str()));
	}
}

void SettingsMenu::PopulateUI() {
	// Populate resolution dropdown
	SettingsHandler::PopulateResolutionOptions([this](const std::string& js) {
		EvalJS(js);
	});

	// Load saved settings values into UI
	LoadSavedSettingsToUI();

	TOAST_INFO("Settings UI populated");
}

void SettingsMenu::LoadSavedSettingsToUI() {
}

}    // namespace game
