/// @file EndingScreen.cpp
/// @author dario
/// @date 11/03/2026.

#include "EndingScreen.hpp"

#include "SettingsHandler.hpp"
#include "Toast/World.hpp"

void EndingScreen::Init() {
	Scene::Init();

	m_htmlView = children.AddRequired<toast::HtmlView>();

	m_htmlView->SetUrl("file:///assets/UI/menus/ThanksForPlaying.html");

	m_htmlView->SetConsoleCallback([this](const std::string& msg) {
		OnConsoleMsg(msg);
	});
}

void EndingScreen::OnConsoleMsg(const std::string& msg) {
	if (game::SettingsHandler::ProcessMessage(msg, [this](const std::string& js) {
		m_htmlView->EvalJS(js);
	})) {
		return;
	}

	// todo
	if (msg == "[EndMenu] form") {
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

	if (msg == "[EndMenu] continue") {
		toast::World::LoadSceneSync("assets/SCENES/MainMenu.scene");
		toast::World::Get("PlayerScene")->Nuke();
		Nuke();
	}
}
