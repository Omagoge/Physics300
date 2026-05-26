//
// Created by Akaansh on 05/11/2025
//

#ifndef GENERAL_TRIGGER_HPP
#define GENERAL_TRIGGER_HPP

#include <Toast/Objects/Actor.hpp>

namespace game {

enum class TriggerMode {
	OnEnter,
	OnExit
};

class GeneralTrigger : public toast::Actor {
public:
	REGISTER_TYPE(GeneralTrigger);

	void Init() override;
	void Begin() override;

	// --- Serialization ---
	void Load(json_t j, bool force_create = true) override;
	[[nodiscard]]
	json_t Save() const override;

#ifdef TOAST_EDITOR
	void Inspector() override;
#endif

protected:
	// Programmer hook (override in derived classes if you want custom code)
	virtual void OnTriggered(toast::Object* other) { }

private:
	TriggerMode m_mode = TriggerMode::OnEnter;    // Enter or Exit
	bool m_enabled = true;                        // Master enable
	bool m_once = true;                           // Fire only once?
	bool m_playerOnly = true;                     // Limit to Player layer (mask)
	// (Optional) You can keep a string tag for level logic, no event dependency:
	std::string m_tag;    // Optional designer tag

	// --- Runtime ---
	bool m_fired = false;

	// Internals
	void OnTriggerEnter(toast::Object* other);
	void OnTriggerExit(toast::Object* other);
	void Execute(toast::Object* other);
};

}    // namespace game

#endif    // GENERAL_TRIGGER_HPP
