#include "Localization.hpp"

#include <Toast/Log.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <vector>

namespace game {

bool Localization::s_loaded = false;
json_t Localization::s_dictionary = json_t::object();
std::vector<std::string> Localization::s_languages = {};

namespace {

std::vector<std::string> ParseCsvLine(const std::string& line) {
	std::vector<std::string> columns;
	std::string current;
	bool inQuotes = false;

	for (size_t i = 0; i < line.size(); ++i) {
		const char c = line[i];
		if (c == '"') {
			if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
				current.push_back('"');
				++i;
			} else {
				inQuotes = !inQuotes;
			}
			continue;
		}
		if (c == ',' && !inQuotes) {
			columns.push_back(current);
			current.clear();
			continue;
		}
		current.push_back(c);
	}

	columns.push_back(current);
	return columns;
}

void StripBom(std::string& value) {
	if (value.size() >= 3 && static_cast<unsigned char>(value[0]) == 0xEF && static_cast<unsigned char>(value[1]) == 0xBB &&
	    static_cast<unsigned char>(value[2]) == 0xBF) {
		value.erase(0, 3);
	}
}

}    // namespace

std::string Localization::NormalizeLanguage(const std::string& language) {
	if (language.empty()) {
		return "en";
	}
	std::string normalized = language;
	std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) {
		return static_cast<char>(std::tolower(c));
	});
	return normalized;
}

void Localization::Reload() {
	s_loaded = false;
	s_dictionary = json_t::object();
	s_languages.clear();
	LoadIfNeeded();
}

void Localization::LoadIfNeeded() {
	if (s_loaded) {
		return;
	}
	s_loaded = true;
	s_dictionary = json_t::object();
	s_languages.clear();

	std::ifstream in("assets/localization/translations.csv");
	if (!in.is_open()) {
		TOAST_WARN("Localization CSV not found at assets/localization/translations.csv");
		return;
	}

	std::string headerLine;
	if (!std::getline(in, headerLine)) {
		TOAST_WARN("Localization CSV is empty");
		return;
	}

	auto headers = ParseCsvLine(headerLine);
	if (headers.empty()) {
		TOAST_WARN("Localization CSV header is invalid");
		return;
	}
	StripBom(headers[0]);

	std::vector<std::string> languages;
	languages.reserve(headers.size() > 1 ? headers.size() - 1 : 0);
	for (size_t i = 1; i < headers.size(); ++i) {
		languages.push_back(NormalizeLanguage(headers[i]));
	}
	s_languages = languages;

	std::string line;
	while (std::getline(in, line)) {
		if (line.empty()) {
			continue;
		}

		auto columns = ParseCsvLine(line);
		if (columns.empty()) {
			continue;
		}

		std::string key = columns[0];
		if (key.empty()) {
			continue;
		}

		auto& entry = s_dictionary[key];
		if (!entry.is_object()) {
			entry = json_t::object();
		}

		const size_t maxColumns = std::min(columns.size(), languages.size() + 1);
		for (size_t i = 1; i < maxColumns; ++i) {
			entry[languages[i - 1]] = columns[i];
		}
	}

	TOAST_INFO("Localization loaded: {} keys", s_dictionary.size());
}

std::vector<std::string> Localization::GetSupportedLanguages() {
	LoadIfNeeded();
	return s_languages;
}

bool Localization::IsSupportedLanguage(const std::string& language) {
	const std::string normalized = NormalizeLanguage(language);
	for (const auto& entry : GetSupportedLanguages()) {
		if (entry == normalized) {
			return true;
		}
	}
	return false;
}

std::string Localization::ResolveTranslation(const std::string& key, const json_t& entry, const std::string& language) {
	const auto fetchIfPresent = [&entry](const std::string& lang) -> std::string {
		if (!entry.contains(lang) || !entry[lang].is_string()) {
			return {};
		}
		const auto value = entry[lang].get<std::string>();
		return value.empty() ? std::string {} : value;
	};

	const std::string normalizedLanguage = NormalizeLanguage(language);
	std::string resolved = fetchIfPresent(normalizedLanguage);
	if (!resolved.empty()) {
		return resolved;
	}

	resolved = fetchIfPresent("en");
	if (!resolved.empty()) {
		return resolved;
	}

	for (auto it = entry.begin(); it != entry.end(); ++it) {
		if (!it.value().is_string()) {
			continue;
		}
		const auto value = it.value().get<std::string>();
		if (!value.empty()) {
			return value;
		}
	}

	return key;
}

json_t Localization::BuildLanguageMap(const std::string& language) {
	LoadIfNeeded();

	json_t languageMap = json_t::object();
	for (auto it = s_dictionary.begin(); it != s_dictionary.end(); ++it) {
		if (!it.value().is_object()) {
			continue;
		}
		languageMap[it.key()] = ResolveTranslation(it.key(), it.value(), language);
	}

	return languageMap;
}

}    // namespace game
