//
// Created by akaan on 19/11/2025.
//

#pragma once
#include "glm/vec3.hpp"

#include <Toast/Components/Component.hpp>
#include <Toast/Components/TransformComponent.hpp>

namespace game {

class InterpMovementComponent : public toast::Component {
public:
	REGISTER_TYPE(InterpMovementComponent);

	void Init() override {
		Component::Init();
	}

	void Begin() override;

	void EarlyTick() override { }

	void Tick() override;

	void LateTick() override { }

	void Destroy() override { }

#ifdef TOAST_EDITOR
	void Inspector() override;
#endif

	json_t Save() const override;
	void Load(json_t, bool force_create) override;

private:
	glm::vec3 m_offset = glm::vec3(0, 0, 0);
	glm::vec3 m_origin = glm::vec3(0, 0, 0);
	float m_speed = 1.0f;
	float m_waitTime = 0.0f;

	float m_t = 0.0f;
	int m_direction = 1;
	float m_waitTimer = 0.0f;
	bool m_isWaiting = false;

	toast::TransformComponent* m_transform = nullptr;
};
}
