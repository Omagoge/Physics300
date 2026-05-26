const root = document.getElementById("controlsRoot");
const keyboardImage = document.getElementById("controlsKeyboard");
const controllerImage = document.getElementById("controlsController");
const hint = document.querySelector(".hint");
let backTriggered = false;

let currentMode = "keyboard_mouse";

function t(key, fallback = key) {
	const dict = window.__toastDict || {};
	return Object.prototype.hasOwnProperty.call(dict, key) ? dict[key] : fallback;
}

function applyLocalization() {
	if (hint) {
		hint.textContent = t("menu.controls.backHint", hint.textContent);
	}
}

window.setLocalizationData = function(translations, language) {
	if (translations && typeof translations === "object") {
		window.__toastDict = translations;
	}
	if (typeof language === "string" && language.length > 0) {
		window.__toastLang = language.toLowerCase();
	}
	applyLocalization();
};

window.onToastLanguageChanged = function() {
	applyLocalization();
};

function showMode(mode) {
	currentMode = (mode === "controller") ? "controller" : "keyboard_mouse";
	root.dataset.mode = currentMode;
	keyboardImage.classList.toggle("visible", currentMode !== "controller");
	controllerImage.classList.toggle("visible", currentMode === "controller");
}

window.setControlsInputMode = function(mode) {
	showMode(mode);
};

function goBack() {
	if (backTriggered) {
		return;
	}
	backTriggered = true;
	console.log("[Menu] controls:exit");
	window.location.href = "MainMenu.html";
}

window.menuMoveUp = function() {};
window.menuMoveDown = function() {};
window.menuMoveLeft = function() {};
window.menuMoveRight = function() {};
window.menuActivate = function() {};
window.menuSelect = function() {
	goBack();
};

window.menuBack = function() {};

function markKeyboardMouseInput() {
	if (currentMode === "keyboard_mouse") {
		return;
	}
	showMode("keyboard_mouse");
	console.log("[Menu] controls:keyboard_mouse");
}

window.addEventListener("keydown", () => {
	markKeyboardMouseInput();
});

window.addEventListener("mousedown", () => {
	markKeyboardMouseInput();
});

window.addEventListener("mousemove", () => {
	markKeyboardMouseInput();
});

showMode("keyboard_mouse");
applyLocalization();
console.log("[Settings] init");
console.log("[Menu] controls:init");
