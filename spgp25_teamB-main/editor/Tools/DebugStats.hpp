/// @file DebugStats.hpp
/// @author dario
/// @date 20/10/2025.

#pragma once
#include <Toast/Resources/Texture.hpp>

namespace resource {
class ResourceManager;
}

namespace editor {

class DebugStats {
public:
	void Init();
	void Show();

	bool enabled;

private:
	resource::ResourceManager* m_resourceManager = nullptr;
	std::shared_ptr<Texture> m_genericFileIcon;
};
}
