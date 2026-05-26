//
// Created by xein on 11/3/25 with no will to live.
//

#ifndef MAIN_MENU_H
#define MAIN_MENU_H
#include "Toast/Components/SpineRendererComponent.hpp"
#include "Toast/Event/ListenerComponent.hpp"

#include <Toast/Input/InputListener.hpp>
#include <Toast/Objects/Scene.hpp>
#include <string>

namespace toast {
class Camera;
class HtmlView;
}

namespace game {

class MainMenu : public toast::Scene {
public:
	REGISTER_TYPE(MainMenu);
	void Init() override;
	void Begin() override;
	void Tick() override;

	double wastedTime = 0;

protected:
	void Destroy() override;

private:
	void OnMove(const input::Action2D* a);
	void OnSelect(const input::Action0D* a);
	void OnBack(const input::Action0D* a);

	void OnConsoleMessage(const std::string& msg);
	void HandlePlay();
	void HandleCredits();
	void HandleExit();
	void EvalJS(const std::string& script);
	void UpdateInputModeFromDevice(input::Device device);
	void SetInputMode(const std::string& mode);
	void SyncControlsInputMode();

	enum class PendingAction {
		None,
		Play,
		Credits,
		Exit
	};
	PendingAction m_pendingAction = PendingAction::None;
	bool m_isPlayingCutscene = false;
	bool m_isControlsScreen = false;
	std::string m_lastInputMode = "keyboard_mouse";

	toast::HtmlView* m_view = nullptr;
	input::Listener m_input;

	event::ListenerComponent* m_eventListener = nullptr;

	// Move repeat state
	int m_moveDir = 0;
	float m_moveHeldTime = 0.0f;
	float m_nextRepeatTime = 0.0f;

	int m_moveDirH = 0;
	float m_moveHeldTimeH = 0.0f;
	float m_nextRepeatTimeH = 0.0f;

	SpineRendererComponent* m_cutsceneSpine = nullptr;

	static constexpr float kInitialDelay = 0.35f;
	static constexpr float kRepeatRate = 0.12f;
	static constexpr float kSliderRepeatRate = 0.05f;
	static constexpr float kDeadzone = 0.4f;

	toast::Camera* m_cam = nullptr;
};

}    // game

#endif
