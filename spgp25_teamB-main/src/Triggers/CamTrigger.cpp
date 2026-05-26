//
// Created by akaansh on 02/02/2026.
//
#include "CamTrigger.hpp"

#include "FollowCamera.hpp"

#include <Toast/Objects/Scene.hpp>
#include <Toast/World.hpp>

#ifdef TOAST_EDITOR
#include "imgui.h"
#include "imgui_stdlib.h"
#endif

namespace game {

void CamTrigger::OnEnter(toast::Object* obj) {
	auto* cam = dynamic_cast<FollowCamera*>(toast::World::Get("FollowCamera_1"));
	if (!cam) {
		return;
	}
	cam->SetZoomScale(m_camZoom);
	TOAST_INFO("Setting Cam Zoom Scale: %f", cam->GetZoomScale());
}

#ifdef TOAST_EDITOR
void CamTrigger::Inspector() {
	physics::Trigger::Inspector();
	ImGui::SeparatorText("Camera Settings");
	ImGui::DragFloat("Camera Zoom", &m_camZoom);
}
#endif

void CamTrigger::Load(json_t j, bool force_create) {
	Trigger::Load(j, force_create);
	if (j.contains("camZoom")) {
		m_camZoom = j.at("camZoom").get<float>();
	}
}

json_t CamTrigger::Save() const {
	json_t j = Trigger::Save();
	if (m_camZoom != 1.0f) {
		j["camZoom"] = m_camZoom;
	}
	return j;
}

}
