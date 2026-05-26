/**
 * @file RayTest.hpp
 * @author Iñaki
 * @date 14/01/2026
 *
 * @brief [TODO: Brief description of the files purpose]
 */

#pragma once
#include "Toast/Objects/Actor.hpp"

class RayTest : public toast::Actor {
public:
	REGISTER_TYPE(RayTest);
	void PhysTick() override;
};
