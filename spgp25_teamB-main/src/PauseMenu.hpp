/// @file PauseMenu.hpp
/// @brief Pause menu component — extends HtmlView with pause/unpause logic

#pragma once

#include <Toast/Components/HtmlView.hpp>
#include <Toast/Event/ListenerComponent.hpp>
#include <Toast/Input/InputListener.hpp>

class PauseMenu : public toast::HtmlView {
public:
	REGISTER_TYPE(PauseMenu);

	void Init() override;
	void Tick() override;

	static PauseMenu* Get();

protected:
	void Destroy() override;
	void OnEnable() override;
	void OnDisable() override;

private:
	void OnMove(const input::Action2D* a);
	void OnSelect(const input::Action0D* a);
	void OnBack(const input::Action0D* a);
	void OnConsoleMessage(const std::string& msg);

	void HandleContinue();
	void HandleSkipScene();
	void HandleMainMenu();
	void HandleExit();

	enum class PendingAction {
		None,
		Continue,
		SkipScene,
		MainMenu,
		Exit
	};
	PendingAction m_pendingAction = PendingAction::None;

	input::Listener m_input;
	event::ListenerComponent* m_eventListener = nullptr;

	int m_moveDir = 0;
	float m_moveHeldTime = 0.0f;
	float m_nextRepeatTime = 0.0f;

	int m_moveDirH = 0;
	float m_moveHeldTimeH = 0.0f;
	float m_nextRepeatTimeH = 0.0f;

	static constexpr float kInitialDelay = 0.35f;
	static constexpr float kRepeatRate = 0.12f;
	static constexpr float kSliderRepeatRate = 0.05f;
	static constexpr float kDeadzone = 0.4f;

	static PauseMenu* s_instance;
};
