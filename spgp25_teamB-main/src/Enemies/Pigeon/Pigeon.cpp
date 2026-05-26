#define GLM_ENABLE_EXPERIMENTAL

#include "Pigeon.hpp"

#include "Toast/GlmJson.hpp"
#include "Toast/Physics/Rigidbody.hpp"
#ifdef TOAST_EDITOR
#include "imgui.h"
#endif
#include "Idle.hpp"
#include "Returning.hpp"
#include "Toast/CoroutineHandler.hpp"
#include "Toast/Renderer/OclussionVolume.hpp"
#include "Toast/WaitAsync.hpp"
#include "Weapons/PigeonGun.hpp"

using namespace moveEnemy_states;

void game::Pigeon::Init() {
	ShootingEnemy::Init();

	// State thingamajiggies
	m_moveSM.SetParent(this);
	m_moveSM.AddState("Idle", std::make_unique<Idle>());
	m_moveSM.AddState("Returning", std::make_unique<Returning>());

	// Rigid body stuff
	m_rb->gravityScale = { 0.f, 0.f };    // Flying enemies aren't affected by gravity
	m_rb->radius = 1.25f;                 // Flying enemies are slightly easier to hit

	// Spine
	// Stupid spine bs
	m_spineJsonPath = "CHARS/ENEMY3_PIDGEON/ANIMATIONS/CH_pidgeon.json";
	m_spineAtlasPath = "CHARS/ENEMY3_PIDGEON/ANIMATIONS/CH_pidgeon.atlas";
	auto enemy_atlas = resource::LoadResource<SpineAtlas>(m_spineAtlasPath);
	auto enemy_skeleton = resource::LoadResource<SpineSkeletonData>(m_spineJsonPath, enemy_atlas);
	m_spine->SetSkeletonData(enemy_skeleton);
	m_spine->scale(glm::vec3(.5f, .5f, 1.f));
	m_spine->position(glm::vec3(0, -0.5, 0));
	m_spine->enabled(true);

	// Weapon
	m_heldWeapon = new PigeonGun();
}

void game::Pigeon::Begin() {
	// Base Begin
	ShootingEnemy::Begin();

	// Pigeon State Defaults
	m_spine->StopAnimation(0);
	m_spine->StopAnimation(1);
	m_spine->StopAnimation(2);
	m_moveSM.SetState("Idle");    // set inital state to Idle

	// Set idle anim
	m_spine->PlayAnimation("an_pidgeon_idle", true, 0);

	// Defaults
	ResetMovementOnBegin();
}

void game::Pigeon::Tick() {
	// Sanity? (Is this even fucking necessary... I think my brain is just rotting?)
	if (Time::delta() == 0.0f || !m_moveParams.canMove) {
		return;
	};

	// Shooting logic
	ShootingEnemy::Tick();

	// Movement stuff
	m_moveSM.Tick();
}

#pragma region Helper Functions

void game::Pigeon::ResetMovementOnBegin() {
	// Defaults
	m_shootParams.currentDir = glm::vec3(0, -1, 0);
	m_moveParams.currentPoint = 0;
	m_moveParams.nextPoint = 1;
	m_rb->mass = 100.f;
	m_moveParams.canMove = true;
	ManageGunAnims();

	// Defaults if we have at least 2 points
	if (m_moveParams.points.size() > 1) {
		m_moveParams.nextPointDir = m_moveParams.points[m_moveParams.nextPoint] - m_moveParams.points[m_moveParams.currentPoint];
		if (m_moveParams.nextPointDir.x != 0 || m_moveParams.nextPointDir.y != 0) {
			m_moveParams.nextPointDir = normalize(m_moveParams.nextPointDir);
		}
		transform()->position(glm::vec3(m_moveParams.points[m_moveParams.currentPoint], transform()->position().z));
	}
}

void game::Pigeon::OnDamage(float damage) {
	// TODO: hit animation?
}

void game::Pigeon::OnDeath() {
	// If we are ready to die, just die...
	if (m_deathAllowed) {
		enabled(false);
		return;
	}

	++scene()->enemy_kills;

	// Else, disable shooting and play death particles
	m_shootParams.shootPlayer = false;
	m_shootParams.shootChild = false;
	m_moveParams.canMove = false;
	m_spine->StopAnimation(0);
	m_spine->StopAnimation(1);
	m_spine->ResetSkeletonToSetupPose();
	m_spine->PlayAnimation("an_pidgeon_dead", false, 2);
	m_awaitingDeath = true;

	// Coroutine for waiting for animation to end...
	[](Pigeon* pigeon) -> toast::CoroutineTask {
		co_await toast::WaitSeconds(0.9f);
		pigeon->AllowDeath(true);
		//pigeon->GetSpineRenderer()->StopAnimation(2);
		//pigeon->GetSpineRenderer()->ResetSkeletonToSetupPose();
		pigeon->OnDeath();
	}(this);
}

#pragma endregion

#pragma region Inspector/Points/Save/Load

#ifdef TOAST_EDITOR
void game::Pigeon::Inspector() {
	ShootingEnemy::Inspector();

	ImGui::PushID(this);

	if (ImGui::CollapsingHeader("Forces and Debug")) {
		ImGui::Spacing();
		ImGui::DragFloat("Speed", &m_moveParams.speed, 0.05);
		ImGui::DragFloat("Point Rad", &m_moveParams.pointRad, 0.05);
		ImGui::Text("Current Point: %u", m_moveParams.currentPoint);
		ImGui::Text("Next Point: %u", m_moveParams.nextPoint);
		ImGui::Checkbox("On Path", &m_moveParams.onPath);
		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Points")) {
		ImGui::Spacing();

		if (ImGui::Button("Add Current Position")) {
			AddPoint(glm::vec2(transform()->position()));
		}

		ImGui::Spacing();

		if (ImGui::Button("Add")) {
			AddPoint(m_moveParams.newPoint);
		}

		ImGui::SameLine();
		ImGui::DragFloat2("Position", &m_moveParams.newPoint.x);

		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Indent(10);

		int idx = 0;
		for (auto it = m_moveParams.points.begin(); it != m_moveParams.points.end();) {
			ImGui::PushID(idx);

			bool moved = false;

			// Up: only if not first
			if (it != m_moveParams.points.begin() && ImGui::SmallButton("U")) {
				auto prev_it = std::prev(it);
				std::iter_swap(it, prev_it);
				it = prev_it;
				moved = true;
			}
			ImGui::SameLine();

			// Down: only if not last
			auto next_it = std::next(it);
			if (next_it != m_moveParams.points.end() && ImGui::SmallButton("D")) {
				std::iter_swap(it, next_it);
				it = next_it;
				moved = true;
			}
			ImGui::SameLine();

			ImGui::Text("Point %d", idx + 1);
			ImGui::SameLine();

			if (ImGui::SmallButton("X")) {
				DeletePoint(*it);
				ImGui::PopID();
				ImGui::Separator();
				ImGui::Spacing();
				return;
			}

			std::string label = std::format("Position##pt{}", idx);
			ImGui::DragFloat2(label.c_str(), &it->x);

			ImGui::PopID();
			ImGui::Separator();
			ImGui::Spacing();

			++it;
			++idx;
		}

		ImGui::Unindent(10);

		ImGui::Spacing();
	}

	ImGui::PopID();
}

#endif

void game::Pigeon::AddPoint(glm::vec2 point) {
	m_moveParams.points.emplace_back(point);
}

void game::Pigeon::SwapPoints(glm::vec2 lhs, glm::vec2 rhs) {
	auto lhs_it = std::ranges::find(m_moveParams.points, lhs);
	auto rhs_it = std::ranges::find(m_moveParams.points, rhs);
	std::swap(lhs_it, rhs_it);
}

void game::Pigeon::DeletePoint(glm::vec2 point) {
	auto it = std::ranges::find(m_moveParams.points, point);
	m_moveParams.points.erase(it);
}

[[nodiscard]]
json_t game::Pigeon::Save() const {
	json_t j = ShootingEnemy::Save();

	// Necessary Variables
	j["Speed"] = m_moveParams.speed;
	j["Point Rad"] = m_moveParams.pointRad;

	// Points
	for (const auto& p : m_moveParams.points) {
		j["points"].push_back(p);
	}

	return j;
}

void game::Pigeon::Load(json_t j, bool force_create) {
	ShootingEnemy::Load(j, force_create);

	// Points
	if (j.contains("points")) {
		// we need to clear the points before loading so we don't have duplicates
		if (!m_moveParams.points.empty()) {
			m_moveParams.points.clear();
		}

		for (const auto& p : j["points"]) {
			AddPoint(p);
		}
	}

	// Necessary Variables
	if (j.contains("Speed")) {
		m_moveParams.speed = j["Speed"].get<float>();
	}
	if (j.contains("Point Rad")) {
		m_moveParams.pointRad = j["Point Rad"].get<float>();
	}
}

#pragma endregion

#pragma region Animations

void game::Pigeon::ResetAnims() {
	// Nothing here...
}

void game::Pigeon::PlayShootIdleAnim() {
	// Nothing for now
}

void game::Pigeon::PlayAimAnim() {
	// No because the aiming one is the same as idle
	// m_spine->PlayAnimation("an_pidgeon_aim", true, 0);

	m_spine->PlayAnimation("an_pidgeon_shoot", true, 1);    // Because the minigun spin here and not in aim
}

void game::Pigeon::PlayFireAnim() {
	// Nothing, this is handled by the muzzle effect
}

void game::Pigeon::EndShootIdleAnim() {
	// Nothing to do
}

void game::Pigeon::EndAimAnim() {
	// No because the aiming one is the same as idle
	// m_spine->StopAnimation(0);

	m_spine->StopAnimation(1);    // Because the minigun spins here and not in aim
}

void game::Pigeon::EndFireAnim() {
	// Nothing, this is handled by the muzzle effect
}

#pragma endregion
