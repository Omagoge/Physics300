const main_menu = document.getElementById("mainMenu");
const items = Array.from(main_menu.querySelectorAll("li"));
const content = document.querySelector(".content");
let focusIndex = -1;
let layoutFramePending = false;

function t(key, fallback = key) {
	const dict = window.__toastDict || {};
	return Object.prototype.hasOwnProperty.call(dict, key) ? dict[key] : fallback;
}

function hasCjkGlyphs(value) {
	return /[\u3400-\u9FFF\uF900-\uFAFF]/.test(String(value ?? ""));
}

const cjkFontStack = "'Resource Han Rounded', 'Microsoft YaHei UI', 'Microsoft YaHei', 'DengXian', 'SimHei', 'SimSun', 'Arial Unicode MS', sans-serif";

function formatLocalizedTitle(value) {
	const normalized = String(value ?? "").replace(/\\n/g, "\n");
	const lines = normalized.split(/\r?\n/).map(line => line.trim()).filter(Boolean);
	if (lines.length === 0) {
		return "";
	}
	return lines.join("<br/>");
}

function isMenuOverflowing() {
	if (!main_menu) return false;
	const rect = main_menu.getBoundingClientRect();
	return rect.bottom > window.innerHeight;
}

function applyMenuSafetyLayout() {
	if (!content) return;
	content.classList.remove("compact-1", "compact-2", "compact-3");

	if (!isMenuOverflowing()) {
		return;
	}

	const compactLevels = ["compact-1", "compact-2", "compact-3"];
	for (const level of compactLevels) {
		content.classList.add(level);
		if (!isMenuOverflowing()) {
			break;
		}
	}
}

function scheduleMenuSafetyLayout() {
	if (layoutFramePending) {
		return;
	}
	layoutFramePending = true;
	window.requestAnimationFrame(() => {
		layoutFramePending = false;
		applyMenuSafetyLayout();
	});
}

window.applyLocalization = function() {
	const title = document.querySelector("#mainTitle .gameTitle");
	if (title) {
		const fallbackTitle = title.textContent.replace(/\n/g, "\\n");
		const localizedTitle = t("menu.main.title", fallbackTitle);
		title.innerHTML = formatLocalizedTitle(localizedTitle);
		title.style.fontFamily = hasCjkGlyphs(localizedTitle) ? cjkFontStack : "";
	}

	const byId = {
		play: "menu.main.play",
		settings: "menu.main.settings",
		controls: "menu.main.controls",
		credits: "menu.main.credits",
		form: "menu.main.form",
		exit: "menu.main.exit"
	};

	for (const [id, key] of Object.entries(byId)) {
		const el = document.getElementById(id);
		if (el) {
			const localized = t(key, el.textContent);
			el.textContent = localized;
			el.style.fontFamily = hasCjkGlyphs(localized) ? cjkFontStack : "";
		}
	}

	scheduleMenuSafetyLayout();
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
	scheduleMenuSafetyLayout();
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
	console.log("[Menu] back");
}

function menuGetFocusedId() {
	if (focusIndex < 0) return "";
	return items[focusIndex].id;
}

function activateItem(item) {
	if (item.id === "settings") {
		window.location.href = "Settings.html";
	} else if (item.id === "controls") {
		window.location.href = "Controls.html";
	} else if (item.id === "credits") {
		window.location.href = "../credits/credits.html";
	} else {
		console.log("[Menu] " + item.id);
	}
}

function markKeyboardMouseInput() {
	console.log("[Menu] controls:keyboard_mouse");
}

main_menu.addEventListener("click", (e) => {
	const target = e.target.closest("li");
	if (!target) return;
	markKeyboardMouseInput();
	activateItem(target);
});

// Mouse hover sets focus too
items.forEach((li, i) => {
	li.addEventListener("mouseenter", () => {
		markKeyboardMouseInput();
		setFocus(i);
	});
});

window.addEventListener("keydown", () => {
	markKeyboardMouseInput();
});

window.addEventListener("resize", () => {
	scheduleMenuSafetyLayout();
});

window.applyLocalization();
scheduleMenuSafetyLayout();
console.log("[Settings] init");
console.log("[Menu] main:init");
