#pragma once

namespace editor {

class VisibilityToggles {
public:
	static void Show(bool& show_scenes, bool& show_actors, bool& show_components, bool& show_disabled);

private:
	struct ToggleButton {
		const char* label;
		bool* value;
	};
};

}
