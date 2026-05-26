/// @file EditorScene.hpp
/// @author dario
/// @date 12/10/2025.

#pragma once
#include "EditorCamera.hpp"

#include <Toast/Objects/Scene.hpp>

namespace editor {

class EditorScene : public toast::Scene {
public:
	REGISTER_TYPE(EditorScene);
	EditorScene() = default;
	~EditorScene() override = default;

	void Init() override;
	void Begin() override;
	void Tick() override;

private:
	EditorCamera* m_editorCamera = nullptr;
};
}
