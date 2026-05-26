#pragma once
#include "Toast/Input/Action.hpp"

#include <Toast/Input/InputListener.hpp>
#include <Toast/Objects/Actor.hpp>

class TestInput : public toast::Actor {
public:
	REGISTER_TYPE(TestInput);

	void Init() override {
		Actor::Init();

		// Input Callbacks
		m.listener.Subscribe2D("2D", [](auto* a) {
			CLIENT_INFO("THE 2D ACTION STATE IS: {0}, Value: {1} x, {2} y", static_cast<int>(a->state), a->value.x, a->value.y);
		});

		m.listener.Subscribe1D("1D", [](auto* a) {
			CLIENT_INFO("THE 1D ACTION STATE IS: {0}, Value: {1}", static_cast<int>(a->state), a->value);
		});

		m.listener.Subscribe0D("0D", [](auto* a) {
			CLIENT_INFO("THE 0D ACTION STATE IS: {0}, Value: {1}", static_cast<int>(a->state), a->value);
		});
	}

	void Begin() override {
		Actor::Begin();
		input::SetLayout("inputTest");
	}

private:
	struct {
		input::Listener listener;
	} m;
};
