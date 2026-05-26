/// @file EndingScreen.hpp
/// @author dario
/// @date 11/03/2026.

#pragma once
#include "Toast/Components/HtmlView.hpp"
#include "Toast/Objects/Scene.hpp"

class EndingScreen : public toast::Scene {
public:
	REGISTER_TYPE(EndingScreen);

private:
	void Init() override;

	void OnConsoleMsg(const std::string& msg);

	toast::HtmlView* m_htmlView;
};
