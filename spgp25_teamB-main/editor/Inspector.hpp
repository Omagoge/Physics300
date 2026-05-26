#pragma once

namespace toast {
class Object;
}

namespace editor {

class Inspector {
public:
	void Show(toast::Object* selected_object);
	void ShowChildren(toast::Object* selected_object);

private:
	bool m_wasEnabled = true;
	bool m_showScene = false;
	bool m_showActor = true;
	bool m_showComponent = true;
	bool m_showDisabled = false;
};

}
