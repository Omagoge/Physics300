/**
* @file SettingsHandler.hpp
 * @date 7 Apr 2026
 * @author Xein
 */

#pragma once

#include <string>
#include <functional>
#include <map>

namespace game {

class SettingsHandler {
public:
	using EvalJSFunc = std::function<void(const std::string&)>;
	static bool ProcessMessage(const std::string& msg, EvalJSFunc evalJS = nullptr);
	static void LoadSavedSettings();
	static int GetPhysicsTPS(const std::string& quality);
	static void PopulateResolutionOptions(EvalJSFunc evalJS);
	static void ApplyGraphicsSettings();

private:
	static void HandleLanguage(const std::string& value, EvalJSFunc evalJS);
	static void HandleAudio(const std::string& setting, const std::string& value);
	static void HandleGraphics(const std::string& setting, const std::string& value);
	static void HandleInit(EvalJSFunc evalJS);
	static void PushLocalization(EvalJSFunc evalJS, const std::string& language);
	static std::map<std::string, std::string> s_stagedGraphics;
};

}    // namespace game
