#pragma once

#include "Toast/Physics/BoxRigidbody.hpp"

#include <Toast/Objects/Actor.hpp>
#include <Toast/Physics/Collider.hpp>
#include <Toast/Physics/Rigidbody.hpp>

class ColliderTest : public toast::Actor {
public:
	REGISTER_TYPE(ColliderTest)

	void Init() override {
		rb = children.AddRequired<physics::Collider>("Collider");
	}

private:
	physics::Collider* rb;
};

class BoxRbTest : public toast::Actor {
public:
	REGISTER_TYPE(BoxRbTest)

	void Init() override {
		bRb = children.AddRequired<physics::BoxRigidbody>("Box Rigidbody");
	}

private:
	physics::BoxRigidbody* bRb;
};

class RigidbodyTest : public toast::Actor {
public:
	REGISTER_TYPE(RigidbodyTest)

	void Init() override {
		rb = children.AddRequired<physics::Rigidbody>("Rigidbody");
	}

private:
	physics::Rigidbody* rb;
};
