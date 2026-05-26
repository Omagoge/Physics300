const pause_menu = document.getElementById("pauseMenu");
const items = Array.from(pause_menu.querySelectorAll("li"));
let focusIndex = -1;

function t(key, fallback = key) {
	const dict = window.__toastDict || {};
	return Object.prototype.hasOwnProperty.call(dict, key) ? dict[key] : fallback;
}

window.applyLocalization = function() {
	const title = document.querySelector("#pauseTitle .gameTitle");
	if (title) {
		title.textContent = t("menu.pause.title", title.textContent);
	}
	const hint = document.querySelector(".hint");
	if (hint) {
		hint.textContent = t("menu.pause.hint", hint.textContent);
	}

	const byId = {
		continue: "menu.pause.continue",
		skipScene: "menu.pause.skipScene",
		retry: "menu.pause.retry",
		settings: "menu.pause.settings",
		mainMenuButton: "menu.pause.mainMenu",
		exit: "menu.pause.exit"
	};

	for (const [id, key] of Object.entries(byId)) {
		const el = document.getElementById(id);
		if (el) {
			el.textContent = t(key, el.textContent);
		}
	}
};

window.setLocalizationData = function(translations, language) {
	if (translations && typeof translations === "object") {
		window.__toastDict = translations;
	}
	if (typeof language === "string" && language.length > 0) {
		window.__toastLang = language.toLowerCase();
	}
	if (typeof window.applyLocalization === "function") {
		window.applyLocalization();
	}
};

window.onToastLanguageChanged = function() {
	if (typeof window.applyLocalization === "function") {
		window.applyLocalization();
	}
};

function setFocus(index) {
	if (items.length === 0) return;
	items.forEach(li => li.classList.remove("focused"));
	focusIndex = ((index % items.length) + items.length) % items.length;
	items[focusIndex].classList.add("focused");
}

function menuMoveUp() {
	setFocus(focusIndex <= 0 ? items.length - 1 : focusIndex - 1);
}

function menuMoveDown() {
	setFocus(focusIndex < 0 ? 0 : focusIndex + 1);
}

function menuActivate() {
	if (focusIndex < 0) return;
	items[focusIndex].classList.add("active");
}

function menuSelect() {
	if (focusIndex < 0) return;
	items[focusIndex].classList.remove("active");
	activateItem(items[focusIndex]);
}

function menuBack() {
	console.log("[Menu] continue");
}

function activateItem(item) {
	if (item.id === "settings") {
		window.location.href = "Settings.html?from=pause";
	} else {
		console.log("[Menu] " + item.id);
	}
}

pause_menu.addEventListener("click", (e) => {
	const target = e.target.closest("li");
	if (!target) return;
	activateItem(target);
});

items.forEach((li, i) => {
	li.addEventListener("mouseenter", () => setFocus(i));
});

window.applyLocalization();
console.log("[Settings] init");
