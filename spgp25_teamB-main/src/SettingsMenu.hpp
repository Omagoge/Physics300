/**
 * @file SettingsMenu.hpp
 * @date 7 Apr 2026
 * @author Xein
 */

#pragma once

#include <Toast/Input/InputListener.hpp>
#include <Toast/Objects/Scene.hpp>

namespace toast {
class Camera;
class HtmlView;
}

namespace game {

class SettingsMenu : public toast::Scene {
public:
	REGISTER_TYPE(SettingsMenu);
	void Init() override;
	void Begin() override;
	void Tick() override;

protected:
	void Destroy() override;

private:
	void OnMove(const input::Action2D* a);
	void OnMoveHorizontal(const input::Action2D* a);
	void OnSelect(const input::Action0D* a);
	void OnBack(const input::Action0D* a);

	void OnConsoleMessage(const std::string& msg);
	void HandleBack();
	void EvalJS(const std::string& script);

	void PopulateUI();
	void LoadSavedSettingsToUI();

	toast::HtmlView* m_view = nullptr;
	toast::Camera* m_cam = nullptr;
	input::Listener m_input;

	int m_moveDir = 0;
	float m_moveHeldTime = 0.0f;
	float m_nextRepeatTime = 0.0f;

	int m_moveDirH = 0;
	float m_moveHeldTimeH = 0.0f;
	float m_nextRepeatTimeH = 0.0f;

	static constexpr float kInitialDelay = 0.35f;
	static constexpr float kRepeatRate = 0.12f;
	static constexpr float kDeadzone = 0.4f;
	static constexpr float kSliderRepeatRate = 0.05f;

	bool m_uiPopulated = false;
};

}    // namespace game
