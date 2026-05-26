#pragma once

#include <Toast/Input/InputListener.hpp>
#include <Toast/Objects/Scene.hpp>

namespace toast {
class HtmlView;
}

namespace game {

class CreditsScene : public toast::Scene {
public:
	REGISTER_TYPE(CreditsScene);
	void Init() override;
	void Begin() override;
	void Tick() override;

protected:
	void Destroy() override;

private:
	void OnSelect(const input::Action0D* a);
	void OnBack(const input::Action0D* a);
	void OnConsoleMessage(const std::string& msg);
	void PushStatsToView();
	void EvalJS(const std::string& script) const;

	toast::HtmlView* m_view = nullptr;
	input::Listener m_input;
	bool m_pendingReturnToMenu = false;
};

}    // namespace game
