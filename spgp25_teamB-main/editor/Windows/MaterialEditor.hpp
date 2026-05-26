/// @file MaterialEditor.hpp
/// @author dario
/// @date 17/11/2025.

#pragma once
#include <Toast/Renderer/Material.hpp>

namespace editor {
class MaterialEditor {
public:
	void Show();

	void SetMaterial(std::string_view path);

	bool enabled = false;
	std::shared_ptr<renderer::Material> m_material = nullptr;
};
}
