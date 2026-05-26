/**
 * @file RayTest.hpp
 * @author Iñaki
 * @date 14/01/2026
 *
 * @brief [TODO: Brief description of the files purpose]
 */
#include "RayTest.hpp"

#include "Toast/Physics/Raycast.hpp"
using namespace physics;

void RayTest::PhysTick() {
	RayCast(transform()->position(), glm::vec2(1, 0));
}
