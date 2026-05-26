#include "CreditsScene.hpp"

#include "SettingsHandler.hpp"
#include "Stats.hpp"

#include <Toast/Components/HtmlView.hpp>
#include <Toast/World.hpp>
#include <Ultralight/Ultralight.h>

namespace game {

namespace {
auto SumNumericValues(const json_t& value) -> long long {
	if (value.is_number_integer()) {
		return value.get<long long>();
	}
	if (value.is_number_float()) {
		return static_cast<long long>(value.get<double>());
	}
	if (!value.is_object()) {
		return 0;
	}

	long long total = 0;
	for (const auto& item : value.items()) {
		const auto& v = item.value();
		if (v.is_number_integer()) {
			total += v.get<long long>();
		} else if (v.is_number_float()) {
			total += static_cast<long long>(v.get<double>());
		}
	}
	return total;
}

auto MaxNumericValues(const json_t& value) -> long long {
	if (value.is_number_integer()) {
		return value.get<long long>();
	}
	if (value.is_number_float()) {
		return static_cast<long long>(value.get<double>());
	}
	if (!value.is_object()) {
		return 0;
	}

	long long currentMax = 0;
	for (const auto& item : value.items()) {
		const auto& v = item.value();
		if (v.is_number_integer()) {
			currentMax = std::max(currentMax, v.get<long long>());
		} else if (v.is_number_float()) {
			currentMax = std::max(currentMax, static_cast<long long>(v.get<double>()));
		}
	}
	return currentMax;
}

auto FormatMilliseconds(long long ms) -> std::string {
	const long long totalSeconds = std::max(0LL, ms / 1000);
	const long long hours = totalSeconds / 3600;
	const long long minutes = (totalSeconds % 3600) / 60;
	const long long seconds = totalSeconds % 60;
	return std::format("{:02}:{:02}:{:02}", hours, minutes, seconds);
}
}    // namespace

void CreditsScene::Init() {
	Scene::Init();

	m_view = children.Get<toast::HtmlView>();
	if (!m_view) {
		m_view = children.AddRequired<toast::HtmlView>("CreditsView");
	}
	if (!m_view) {
		return;
	}

	m_view->SetConsoleCallback([this](const std::string& msg) {
		OnConsoleMessage(msg);
	});
	m_view->SetUrl("file:///assets/UI/credits/credits.html");
}

void CreditsScene::Begin() {
	Scene::Begin();

	m_pendingReturnToMenu = false;
	input::SetLayout("ui");

	m_input.Subscribe0D("select", [this](const input::Action0D* a) {
		OnSelect(a);
	});
	m_input.Subscribe0D("back", [this](const input::Action0D* a) {
		OnBack(a);
	});
}

void CreditsScene::Tick() {
	Scene::Tick();

	if (!m_pendingReturnToMenu) {
		return;
	}

	m_pendingReturnToMenu = false;
	toast::World::LoadSceneSync("SCENES/MainMenu.scene");
	Nuke();
}

void CreditsScene::Destroy() {
	m_view = nullptr;
}

void CreditsScene::OnSelect(const input::Action0D* a) {
	if (!a || a->state != input::Action0D::Finished) {
		return;
	}
	EvalJS("if(typeof onUiSelect==='function') onUiSelect();");
}

void CreditsScene::OnBack(const input::Action0D* a) {
	if (!a || a->state != input::Action0D::Started) {
		return;
	}
	EvalJS("if(typeof onUiSelect==='function') onUiSelect();");
}

void CreditsScene::OnConsoleMessage(const std::string& msg) {
	if (SettingsHandler::ProcessMessage(msg, [this](const std::string& js) {
		EvalJS(js);
	})) {
		return;
	}

	if (msg == "[Credits] init") {
		PushStatsToView();
		return;
	}

	if (msg == "[Credits] return-main") {
		m_pendingReturnToMenu = true;
	}
}

void CreditsScene::PushStatsToView() {
	const auto& s = Stats::stats;

	const long long totalTime = SumNumericValues(s.value("best_time", json_t::object()));
	const long long totalAir = SumNumericValues(s.value("air_time", json_t::object()));
	const long long totalGravity = SumNumericValues(s.value("grav_time", json_t::object()));
	const long long wastedTime = SumNumericValues(s.value("wastedTime", 0));
	const long long maxSpeed = MaxNumericValues(s.value("max_speed", json_t::object()));
	const long long enemiesKilled = SumNumericValues(s.value("enemy_kills", json_t::object()));
	const long long lostHealth = SumNumericValues(s.value("hit_number", json_t::object()));

	json_t payload = {
		{ "time", FormatMilliseconds(totalTime) },
		{ "time_on_air", FormatMilliseconds(totalAir) },
		{ "time_on_gravity", FormatMilliseconds(totalGravity) },
		{ "time_wasted", FormatMilliseconds(wastedTime) },
		{ "maximum_speed", std::to_string(maxSpeed) },
		{ "enemies_killed", std::to_string(enemiesKilled) },
		{ "lost_health", std::to_string(lostHealth) }
	};

	EvalJS(std::format("if(typeof setStats==='function') setStats({});", payload.dump()));
}

void CreditsScene::EvalJS(const std::string& script) const {
	if (!m_view) {
		return;
	}
	if (auto view = m_view->GetView()) {
		view->EvaluateScript(ultralight::String(script.c_str()));
	}
}

}    // namespace game
