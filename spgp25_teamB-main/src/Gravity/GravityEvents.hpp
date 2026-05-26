/// @file GravityEvents.hpp
/// @author Xein
/// @date 22/02/26
///
/// We want to send an event when we start wanting
/// to change gravity rather than having a reference
/// on the player to the GravPointList class, this
/// will make relationships less dependant since we
/// don't really care in the player about what goes
/// on with the gravity system
///
/// Empty events are really funny tho

#pragma once
#include <Toast/Event/Event.hpp>

struct GravityBegin : event::Event<GravityBegin> {
	GravityBegin() = default;
};

struct GravityEnd : event::Event<GravityEnd> {
	GravityEnd() = default;
};
