const thanks_menu = document.getElementById("thanksMenu");
const items = Array.from(thanks_menu.querySelectorAll("li"));
const title = document.querySelector(".title");
let focusIndex = -1;

function t(key, fallback = key) {
	const dict = window.__toastDict || {};
	return Object.prototype.hasOwnProperty.call(dict, key) ? dict[key] : fallback;
}

function applyLocalization() {
	if (title) {
		title.textContent = t("credits.thankYouForPlaying", title.textContent);
	}
	const form = document.getElementById("form");
	if (form) {
		form.textContent = t("menu.main.form", form.textContent);
	}
	const mainMenu = document.getElementById("main-menu");
	if (mainMenu) {
		mainMenu.textContent = t("menu.pause.mainMenu", mainMenu.textContent);
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
	console.log("[EndMenu] continue");
}

function activateItem(item) {
	if (item.id === "form") {
		console.log("[EndMenu] form");
	} else {
		console.log("[EndMenu] continue");
	}
}

thanks_menu.addEventListener("click", (e) => {
	const target = e.target.closest("li");
	if (!target) return;
	activateItem(target);
});

items.forEach((li, i) => {
	li.addEventListener("mouseenter", () => setFocus(i));
});

applyLocalization();
console.log("[Settings] init");
