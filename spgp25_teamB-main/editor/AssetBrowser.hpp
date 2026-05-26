/// @file AssetBrowser.hpp
/// @author dario
/// @date 18/10/2025.

#pragma once
#include "Tools/TextModal.hpp"
#include "Windows/MaterialEditor.hpp"

#include <Toast/Resources/ResourceManager.hpp>

namespace editor {
class AssetBrowser {
public:
	AssetBrowser();

	void Init();
	void Show();

private:
	void RescanDirectory();

	void RightClickEntry(const ResourceSlot::Entry& entry);

	MaterialEditor m_materialEditor;

	TextModal m_renameTextModal;

	std::filesystem::path m_currentDir;

	// Cached entries for the current directory
	std::vector<editor::ResourceSlot::Entry> m_entries;
	bool m_needsRescan = true;
};
}
