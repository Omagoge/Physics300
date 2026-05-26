#pragma once
#include "../ShootingEnemy/ShootingEnemy.hpp"

namespace game {

struct MovingEnemyParameters {
	// Movement Multipliers
	float speed = 10.f;
	float pointRad = 0.25f;

	// Point Logic
	unsigned currentPoint = 0;
	unsigned nextPoint = 1;

	// Direction && Forces
	glm::vec2 nextPointDir = glm::vec2(0.0f);
	bool onPath = true;
	bool canMove = true;

	// Points
	std::vector<glm::vec2> points;
	glm::vec2 newPoint;
};

class Pigeon : public ShootingEnemy {
public:
	REGISTER_TYPE(Pigeon);

	void Init() override;

	void Begin() override;

	void Tick() override;

	void AddPoint(glm::vec2 point);
	void SwapPoints(glm::vec2 lhs, glm::vec2 rhs);
	void DeletePoint(glm::vec2 point);

	auto GetPoints() const -> std::vector<glm::vec2> {
		return m_moveParams.points;
	}

#ifdef TOAST_EDITOR
	void Inspector() override;
#endif

	[[nodiscard]]
	json_t Save() const override;
	void Load(json_t j, bool force_create = true) override;

	MovingEnemyParameters* GetMoveParameters() {
		return &m_moveParams;
	}

	toast::StateMachine<Pigeon>* GetMoveStateMachine() {
		return &m_moveSM;
	}

	void OnDamage(float damage) override;
	void OnDeath() override;

	// Helpers
	void ResetMovementOnBegin();

	// Animation bullshit
	void PlayShootIdleAnim() override;
	void PlayAimAnim() override;
	void PlayFireAnim() override;

	void EndShootIdleAnim() override;
	void EndAimAnim() override;
	void EndFireAnim() override;

	void ResetAnims() override;

protected:
	// Parameters
	MovingEnemyParameters m_moveParams;

	// State
	toast::StateMachine<Pigeon> m_moveSM;
};
}
