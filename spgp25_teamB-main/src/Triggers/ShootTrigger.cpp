#include "ShootTrigger.hpp"

#include "Toast/Objects/Actor.hpp"
#ifdef TOAST_EDITOR
#include "imgui.h"
#endif

namespace game {

#ifdef TOAST_EDITOR
void ShootTrigger::Inspector() {
	toast::Actor::Inspector();
	ImGui::InputInt("World", &m.world);
	ImGui::InputInt("Level", &m.level);
}
#endif

}
