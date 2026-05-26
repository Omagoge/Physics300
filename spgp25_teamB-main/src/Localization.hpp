#pragma once

#include "Toast/ISerializable.hpp"

#include <string>
#include <vector>

namespace game {

class Localization {
public:
	static json_t BuildLanguageMap(const std::string& language);
	static std::string NormalizeLanguage(const std::string& language);
	static bool IsSupportedLanguage(const std::string& language);
	static std::vector<std::string> GetSupportedLanguages();
	static void Reload();

private:
	static void LoadIfNeeded();
	static std::string ResolveTranslation(const std::string& key, const json_t& entry, const std::string& language);

	static bool s_loaded;
	static json_t s_dictionary;
	static std::vector<std::string> s_languages;
};

}    // namespace game
