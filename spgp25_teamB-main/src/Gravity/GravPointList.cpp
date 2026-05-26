#include "GravPointList.hpp"

#include "GravPoint.hpp"
#include "GravityEvents.hpp"
#include "Player/Player.hpp"

#include <Toast/Input/Haptics.hpp>

#include <Toast/Log.hpp>
#include <Toast/Physics/Physics.hpp>
#include <Toast/Renderer/DebugDrawLayer.hpp>
#include <Toast/Renderer/OclussionVolume.hpp>
#include <Toast/World.hpp>
#include <glm/glm.hpp>

#ifdef TOAST_EDITOR
#include <imgui.h>
#endif
#include <limits>

void GravPointList::Init() {
	for (const auto& c : children | std::views::values) {
		if (auto* obj = dynamic_cast<GravPoint*>(c.get())) {
			m.gravityPoints.emplace_back(obj);
		}
	}
}

void GravPointList::Begin() {
	m.player = toast::World::GetFromType<Player>();
	if (not m.player) {
		// Disable the object so it doesnt crash and log it
		this->enabled(false);
		CLIENT_ERROR("[Gravity List] Player not found");
	}

	m.listener.Subscribe<GravityBegin>([this](auto* e) {
		OnGravityBegin();
		return true;
	});

	m.listener.Subscribe<GravityEnd>([this](auto* e) {
		OnGravityEnd();
		return true;
	});
}

void GravPointList::Tick() {
#ifdef TOAST_EDITOR
	if (not m.player) {
		return;
	}
#endif

	float nearest_distance = std::numeric_limits<float>::max();
	glm::vec2 direction = glm::normalize(m.player->GetParameters()->aimingDirection);

	if (not m.isGravityRunning) {
		auto is_on_screen = [](auto* o) -> bool {
			return OclussionVolume::isSphereOnPlanes(o->transform()->position(), o->transform()->scale().x);
		};

		for (auto* o : m.gravityPoints | std::views::filter(is_on_screen)) {
			// @dario wanted to keep this -x
			// glm::vec2 to_object = o->transform()->worldPosition() - m.player->transform()->worldPosition();
			// if (glm::dot(direction, to_object) < 0) {
			//	continue;
			//}

			// float distance = glm::length(to_object);

			// if (distance > nearest_distance) {
			//	continue;
			// }

			// nearest_distance = distance;
			// m.nearestPoint = o;

			float distance = glm::distance(o->transform()->worldPosition(), m.player->transform()->worldPosition());

			if (distance > nearest_distance) {
				continue;
			}

			nearest_distance = distance;
			m.nearestPoint = o;
		}
	} else {
		if (!m.nearestPoint) {
			event::Send(new GravityEnd());
			return;
		}

		auto* player_params = m.player->GetParameters();
		if (not player_params->grounded) {
			glm::vec2 dir = m.nearestPoint->transform()->worldPosition() - m.player->transform()->worldPosition();
			glm::vec2 tan = glm::normalize(dir);
			player_params->groundTangent = -glm::vec2(tan.y, -tan.x);
			// if (player_params->groundTangent.x > 0.0f) {
			// 	player_params->groundTangent *= -1.0f;
			// }
			player_params->gravity = { 6, 6 };
		}

		// auto is_on_screen = [](physics::Rigidbody* rb) -> bool {
		//	glm::vec2 pos = rb->GetPosition();
		//	glm::vec3 pos3 = { pos.x, pos.y, 0.0f };
		//	return OclussionVolume::isSphereOnPlanes(pos3, rb->radius);
		// };
		// for (auto* rb : physics::GetAllRigidbodies() | std::views::filter(is_on_screen)) {
		//	glm::dvec2 distance = glm::dvec2 { m.nearestPoint->transform()->position() } - rb->GetPosition();
		//	glm::dvec2 dir = glm::normalize(distance);
		//	glm::dvec2 force = dir * 0.1;

		//	rb->AddAccel(force);
		//}
	}

	for (const auto p : m.gravityPoints) {
		if (p) {
			p->Disable();
		}
	}

	if (m.nearestPoint) {
		if (!m.isGravityRunning) {
			m.nearestPoint->Enable();
		} else {
			m.nearestPoint->Using();
		}
		if (not OclussionVolume::isSphereOnPlanes(m.nearestPoint->transform()->worldPosition(), m.nearestPoint->transform()->scale().x + 3.f)) {
			m.nearestPoint->Disable();
			m.nearestPoint = nullptr;
		}
	}

	// if (not m.nearestPoint) {
	// 	TOAST_ERROR("NO NEAREST POINT");
	// }

	if (m.nearestPoint) {
		renderer::DebugLine(
		    m.player->transform()->worldPosition(),
		    m.nearestPoint->transform()->position(),
		    m.isGravityRunning ? glm::vec4 { 0.f, 1.f, 0.f, 1.f } : glm::vec4 { 1.0f, 0.0f, 1.0f, 1.0f }
		);
	}

#ifdef TOAST_EDITOR
	// draw on toast editor

#endif
}

void GravPointList::OnGravityBegin() {
	// for (auto* rb : physics::GetAllRigidbodies()) {
	//	rb->hasGravity = false;
	// }

	if (!m.nearestPoint) {
		return;
	}

	haptics::Rumble(0.5f, 1.0f, 200);    // successful grav point attachment

	CLIENT_INFO("Starting gravity gun");

	using namespace physics;
	SetGravityType(GravityType::POINT);
	SetGravityPoint(m.nearestPoint->transform()->worldPosition());
	SetGravityPointScale(9.8f);

	m.nearestPoint->Using();

	m.isGravityRunning = true;

	m.player->GetOnAirParticles()->Play();
	m.player->GetParameters()->isUsingGrav = true;
}

void GravPointList::OnGravityEnd() {
	CLIENT_INFO("Stopping gravity gun");

	// for (auto* rb : physics::GetAllRigidbodies()) {
	//	rb->hasGravity = true;
	// }

	using namespace physics;
	SetGravityType(GravityType::DIRECTION);

	if (m.player) {
		auto* p_params = m.player->GetParameters();
		p_params->groundTangent = { 1.0f, 0.0f };
		p_params->gravity = glm::vec2(0.f, -p_params->gravForce);

		if (m.isGravityRunning) {
			m.player->GetOnAirParticles()->Pause();
		}

		m.player->GetParameters()->isUsingGrav = false;
	}

	for (const auto& p : m.gravityPoints) {
		p->Disable();
	}

	m.isGravityRunning = false;
}

void GravPointList::AddPoint() {
	auto* grav_point = children.Add<GravPoint>();
	m.gravityPoints.emplace_back(grav_point);
	// bro this transform sucks ass -x
	grav_point->transform()->position({ 0.0f, 0.0f, 0.0f });
}

void GravPointList::RemovePoint(GravPoint* point) {
	m.gravityPoints.remove(point);
	children.Remove(point->id());
}

#ifdef TOAST_EDITOR
void GravPointList::Inspector() {
	ImGui::Text("%i gravity points", int(m.gravityPoints.size()));
	ImGui::SameLine();
	if (ImGui::SmallButton("Create")) {
		AddPoint();
	}

	ImGui::Indent(20);
	for (auto* p : m.gravityPoints) {
		ImGui::PushID(p->id());

		ImVec4 color = { 1, 1, 1, 1 };
		if (p == m.nearestPoint) {
			color = { 0, 1, 0, 1 };
		}
		ImGui::TextColored(color, "%i: %s", p->id(), p->name().c_str());

		ImGui::SameLine();
		if (ImGui::SmallButton("x")) {
			RemovePoint(p);
			ImGui::PopID();
			return;    // avoids imgui crash
		}
		ImGui::Spacing();
		ImGui::PopID();
	}
	ImGui::Unindent(20);
}
#endif
