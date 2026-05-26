/// @file EditorScene.cpp
/// @author dario
/// @date 12/10/2025.

#include "EditorScene.hpp"

#include "EditorCamera.hpp"

#include <Toast/Components/MeshRendererComponent.hpp>
#include <Toast/Components/SpineRendererComponent.hpp>
#include <Toast/Renderer/DebugDrawLayer.hpp>
#include <Toast/Renderer/IRendererBase.hpp>
#include <Toast/Resources/ResourceManager.hpp>

namespace editor {

void EditorScene::Init() {
	Scene::Init();
	m_editorCamera = children.Add<EditorCamera>();
	m_editorCamera->transform()->position({ .1f, .1f, 5 });
}

void EditorScene::Begin() {
	Scene::Begin();
	renderer::IRendererBase::GetInstance()->SetActiveCamera(m_editorCamera);
}

void EditorScene::Tick() {
	Scene::Tick();
	// renderer::DebugDrawLayer::GetInstance()->DrawGrid(20, renderer::IRendererBase::GetInstance()->GetViewProjectionMatrix());
}
}
