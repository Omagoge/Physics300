#include "BaseTypeRegistry.hpp"

namespace editor {

std::multimap<toast::BaseType, std::string> Registry::m_classes;

void Registry::Init() {
	toast::Object::Children c;
	c.parent(nullptr);
	c.scene(nullptr);

	for (auto reg = toast::Object::getRegistry(); const auto& [type, allocator] : reg) {
		// Create a temporary object to query its base type
		auto* o = allocator(c, -1);    // This won't comsume ids now
		m_classes.emplace(o->base_type(), type);
	}
}

}
