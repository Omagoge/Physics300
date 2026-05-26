/**
 * @file BaseTypeRegistry.hpp
 * @author Dante Harper
 * @date 27/10/25
 *
 * @brief singleton for handling the base type registry
 * TODO: Xein fix ur shit code
 */

#pragma once

#include <Toast/Objects/Object.hpp>

namespace editor {

class Registry {
public:
	static void Init();

	static const std::multimap<toast::BaseType, std::string>& BaseTypeRegistry() {
		return m_classes;
	}

private:
	static std::multimap<toast::BaseType, std::string> m_classes;
};

}
