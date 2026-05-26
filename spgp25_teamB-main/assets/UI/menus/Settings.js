document.addEventListener('DOMContentLoaded', () => {
	const menuItems = Array.from(document.querySelectorAll('#settingsMenu > li'));
	const settingsMenu = document.getElementById('settingsMenu');
	const sections = document.querySelectorAll('.settings-section');
	const bgTint = document.getElementById('bgTint');
	const content = document.querySelector('.content');

	// Main menu focus
	let focusIndex = -1;

	// Language tracking
	let selectedLanguage = null;

	// Focus context
	let focusContext = {
		inSection: false,
		sectionId: null,
		rowIndex: 0,
		colIndex: 0,  // For controls table
		inDropdown: false,
		dropdownIndex: 0
	};

	// Auto-repeat for held input
	const INITIAL_DELAY = 350;  // ms before repeat starts
	const REPEAT_RATE = 120;    // ms between repeats
	let repeatTimer = null;

	function startRepeat(action) {
		if (repeatTimer) return;
		action();  // Execute immediately
		repeatTimer = setTimeout(() => {
			repeatTimer = setInterval(action, REPEAT_RATE);
		}, INITIAL_DELAY);
	}

	function stopRepeat() {
		clearTimeout(repeatTimer);
		clearInterval(repeatTimer);
		repeatTimer = null;
	}

	// Staged graphics changes
	let stagedGraphics = {};
	const resolutionOptionsByAspect = {
		'16:9': ['1280x720', '1706x960', '1920x1080', '2560x1440', '3840x2160'],
		'16:10': ['1152x720', '1536x960', '1728x1080', '2304x1440', '3456x2160'],
		'21:9': ['1680x720', '2240x960', '2520x1080', '3360x1440', '5040x2160'],
		'4:3': ['960x720', '1280x960', '1440x1080', '1920x1440', '2880x2160']
	};

	function resolutionLabel(value) {
		return value;
	}

	const localizationState = {
		language: 'en',
		translations: {}
	};

	function t(key, fallback = '') {
		const value = localizationState.translations?.[key];
		if (typeof value === 'string' && value.length > 0) {
			return value;
		}
		return fallback || key;
	}

	function syncDropdownCurrentLabels() {
		document.querySelectorAll('.dropdown').forEach(dropdown => {
			const current = dropdown.querySelector('.dropdown-current');
			if (!current) return;
			const selectedValue = dropdown.getAttribute('data-value');
			const selectedOption = dropdown.querySelector(`.dropdown-options li[data-value="${selectedValue}"]`);
			if (selectedOption) {
				current.textContent = selectedOption.textContent;
			}
		});
	}

	function applyLocalizationToDom() {
		document.querySelectorAll('[data-i18n]').forEach(element => {
			const key = element.getAttribute('data-i18n');
			if (!key) return;
			element.textContent = t(key, element.textContent);
		});
		syncDropdownCurrentLabels();
	}

	window.setLocalizationData = function(translations, language) {
		localizationState.translations = (translations && typeof translations === 'object') ? translations : {};
		if (typeof language === 'string' && language.length > 0) {
			localizationState.language = language.toLowerCase();
		}
		window.__toastDict = localizationState.translations;
		window.__toastLang = localizationState.language;
		applyLocalizationToDom();
		const selectedItem = document.querySelector(`#section-language li[data-lang="${localizationState.language}"]`);
		if (selectedItem) {
			selectLanguage(selectedItem, false);
		} else {
			const fallback = document.querySelector('#section-language li[data-lang="en"]') || document.querySelector('#section-language li[data-lang]');
			if (fallback) {
				localizationState.language = fallback.getAttribute('data-lang') || 'en';
				selectLanguage(fallback, false);
			}
		}
	};

	window.applyLocalization = function() {
		if (window.__toastDict && typeof window.__toastDict === 'object') {
			localizationState.translations = window.__toastDict;
		}
		if (typeof window.__toastLang === 'string' && window.__toastLang.length > 0) {
			localizationState.language = window.__toastLang.toLowerCase();
		}
		applyLocalizationToDom();
		const selectedItem = document.querySelector(`#section-language li[data-lang="${localizationState.language}"]`);
		if (selectedItem) {
			selectLanguage(selectedItem, false);
		} else {
			const fallback = document.querySelector('#section-language li[data-lang="en"]') || document.querySelector('#section-language li[data-lang]');
			if (fallback) {
				localizationState.language = fallback.getAttribute('data-lang') || 'en';
				selectLanguage(fallback, false);
			}
		}
	};

	window.onToastLanguageChanged = function() {
		window.applyLocalization();
	};

	function refreshResolutionOptions(preferredValue = null) {
		const aspectRatioDropdown = document.getElementById('aspectRatio');
		const resolutionDropdown = document.getElementById('resolution');
		const optionsContainer = document.getElementById('resolutionOptions');
		if (!aspectRatioDropdown || !resolutionDropdown || !optionsContainer) return;

		const aspectRatio = aspectRatioDropdown.getAttribute('data-value') || '16:9';
		const options = resolutionOptionsByAspect[aspectRatio] || resolutionOptionsByAspect['16:9'];
		const targetValue = preferredValue || resolutionDropdown.getAttribute('data-value');
		optionsContainer.innerHTML = '';

		options.forEach(value => {
			const li = document.createElement('li');
			li.setAttribute('data-value', value);
			li.textContent = resolutionLabel(value);
			optionsContainer.appendChild(li);
		});

		const fallbackValue = options[Math.min(2, options.length - 1)] || options[0];
		const nextValue = options.includes(targetValue) ? targetValue : fallbackValue;
		if (!nextValue) return;

		resolutionDropdown.setAttribute('data-value', nextValue);
		const resolutionCurrent = resolutionDropdown.querySelector('.dropdown-current');
		if (resolutionCurrent) {
			resolutionCurrent.textContent = resolutionLabel(nextValue);
		}
	}

	function isVsyncEnabled() {
		const vsyncCheckbox = document.getElementById('vsync');
		return vsyncCheckbox?.getAttribute('data-checked') === 'true';
	}

	function syncMaxFpsDisabledState() {
		const maxFpsSlider = document.getElementById('maxFps');
		const maxFpsRow = maxFpsSlider?.closest('.setting-row');
		if (!maxFpsSlider || !maxFpsRow) return;

		const disabled = isVsyncEnabled();
		maxFpsSlider.disabled = disabled;
		maxFpsRow.classList.toggle('disabled', disabled);
		if (disabled) {
			maxFpsRow.classList.remove('active');
		}
	}

	// =========== MAIN MENU FOCUS ===========
	function setFocus(index) {
		if (menuItems.length === 0) return;
		menuItems.forEach(li => li.classList.remove('focused'));
		focusIndex = ((index % menuItems.length) + menuItems.length) % menuItems.length;
		menuItems[focusIndex].classList.add('focused');
	}


	function getCurrentSection() {
		return focusContext.sectionId ? document.getElementById(focusContext.sectionId) : null;
	}

	function getNavigableRows(section) {
		if (!section) return [];
		// Language section uses li items, others use setting-row divs
		if (section.id === 'section-language') {
			return Array.from(section.querySelectorAll('li[data-lang]'));
		}
		return Array.from(section.querySelectorAll('.setting-row[data-nav-row]'));
	}

	function setRowFocus(index) {
		const section = getCurrentSection();
		if (!section) return;
		const rows = getNavigableRows(section);
		if (rows.length === 0) return;

		rows.forEach(r => {
			r.classList.remove('focused');
			r.querySelector('.apply-btn')?.classList.remove('focused');
		});
		focusContext.rowIndex = ((index % rows.length) + rows.length) % rows.length;
		const focusedRow = rows[focusContext.rowIndex];
		focusedRow.classList.add('focused');
		focusedRow.querySelector('.apply-btn')?.classList.add('focused');

		// Close any open dropdowns when changing rows
		if (!focusContext.inDropdown) {
			section.querySelectorAll('.dropdown.open').forEach(d => d.classList.remove('open'));
		}
	}

	function openSection(sectionId) {
		sections.forEach(s => s.classList.remove('active'));
		const section = document.getElementById(sectionId);
		if (section) {
			section.classList.add('active');
			focusContext.inSection = true;
			focusContext.sectionId = sectionId;
			focusContext.rowIndex = 0;
			focusContext.colIndex = 0;  // Reset column for controls table
			focusContext.inDropdown = false;
			focusContext.dropdownIndex = 0;
			
			setRowFocus(0);
		}
	}

	function closeSection() {
		sections.forEach(s => s.classList.remove('active'));
		const section = getCurrentSection();
		if (section) {
			section.querySelectorAll('.focused').forEach(el => el.classList.remove('focused'));
			section.querySelectorAll('.apply-btn.focused').forEach(el => el.classList.remove('focused'));
			section.querySelectorAll('.dropdown.open').forEach(d => d.classList.remove('open'));
		}
		focusContext.inSection = false;
		focusContext.sectionId = null;
		focusContext.rowIndex = 0;
		focusContext.colIndex = 0;
		focusContext.inDropdown = false;
		focusContext.dropdownIndex = 0;
		settingsMenu.classList.remove('shifted');
		menuItems.forEach(i => i.classList.remove('selected'));
	}

	function selectMenuItem(item) {
		if (!item) return;
		const index = menuItems.indexOf(item);
		if (index >= 0) {
			setFocus(index);
		}
		menuItems.forEach(i => {
			i.classList.remove('selected');
			i.classList.remove('active');
		});
		item.classList.add('selected');
		settingsMenu.classList.add('shifted');
	}

	// =========== DROPDOWN HANDLING ===========
	function openDropdown(dropdown) {
		dropdown.classList.add('open');
		focusContext.inDropdown = true;
		const options = Array.from(dropdown.querySelectorAll('.dropdown-options li'));
		const currentValue = dropdown.getAttribute('data-value');
		focusContext.dropdownIndex = options.findIndex(o => o.getAttribute('data-value') === currentValue);
		if (focusContext.dropdownIndex < 0) focusContext.dropdownIndex = 0;
		setDropdownOptionFocus(dropdown, focusContext.dropdownIndex);
	}

	function closeDropdown(dropdown) {
		dropdown.classList.remove('open');
		dropdown.querySelectorAll('.dropdown-options li').forEach(li => li.classList.remove('focused'));
		focusContext.inDropdown = false;
		focusContext.dropdownIndex = 0;
	}

	function setDropdownOptionFocus(dropdown, index) {
		const options = Array.from(dropdown.querySelectorAll('.dropdown-options li'));
		if (options.length === 0) return;
		options.forEach(o => o.classList.remove('focused'));
		focusContext.dropdownIndex = ((index % options.length) + options.length) % options.length;
		options[focusContext.dropdownIndex].classList.add('focused');
	}

	function selectDropdownOption(dropdown, option) {
		const value = option.getAttribute('data-value');
		const text = option.textContent;
		dropdown.setAttribute('data-value', value);
		dropdown.querySelector('.dropdown-current').textContent = text;
		closeDropdown(dropdown);

		// Handle preset changes - apply preset values to other settings
		const dropdownId = dropdown.id;
		if (dropdownId === 'graphicsPreset') {
			applyPreset(value);
		}
		if (dropdownId === 'aspectRatio') {
			refreshResolutionOptions();
			return;
		}

		// Send audio changes immediately, stage graphics changes
		if (dropdownId === 'audioMode') {
			console.log(`[Settings] audio.mode ${value}`);
		} else {
			// Stage for graphics apply
			stagedGraphics[dropdownId] = value;
		}
	}

	// Preset definitions
	const presets = {
		low: {
			resolutionScale: 50,
			lightResolutionScale: 50,
			lightQuality: 'mid',
			physicsQuality: 'low',
		},
		high: {
			resolutionScale: 100,
			lightResolutionScale: 75,
			lightQuality: 'high',
			physicsQuality: 'mid',
		},
		ultra: {
			resolutionScale: 100,
			lightResolutionScale: 100,
			lightQuality: 'ultra',
			physicsQuality: 'high',
		}
	};

	function applyPreset(presetName) {
		const preset = presets[presetName];
		if (!preset) return;

		// Resolution Scale
		const resScaleSlider = document.getElementById('resolutionScale');
		if (resScaleSlider && preset.resolutionScale !== undefined) {
			resScaleSlider.value = preset.resolutionScale;
			updateSliderDisplay(resScaleSlider);
			stagedGraphics['resolutionScale'] = preset.resolutionScale;
		}

		// Light Resolution Scale
		const lightResSlider = document.getElementById('lightResolutionScale');
		if (lightResSlider && preset.lightResolutionScale !== undefined) {
			lightResSlider.value = preset.lightResolutionScale;
			updateSliderDisplay(lightResSlider);
			stagedGraphics['lightResolutionScale'] = preset.lightResolutionScale;
		}

		// Light Quality
		const lightQualityDropdown = document.getElementById('lightQuality');
		if (lightQualityDropdown && preset.lightQuality) {
			lightQualityDropdown.setAttribute('data-value', preset.lightQuality);
			const option = lightQualityDropdown.querySelector(`[data-value="${preset.lightQuality}"]`);
			if (option) {
				lightQualityDropdown.querySelector('.dropdown-current').textContent = option.textContent;
			}
			stagedGraphics['lightQuality'] = preset.lightQuality;
		}

		// Physics Quality
		const physicsDropdown = document.getElementById('physicsQuality');
		if (physicsDropdown && preset.physicsQuality) {
			physicsDropdown.setAttribute('data-value', preset.physicsQuality);
			const option = physicsDropdown.querySelector(`[data-value="${preset.physicsQuality}"]`);
			if (option) {
				physicsDropdown.querySelector('.dropdown-current').textContent = option.textContent;
			}
			stagedGraphics['physicsQuality'] = preset.physicsQuality;
		}
	}

	function updateSliderDisplay(slider) {
		const container = slider.closest('.slider-container');
		if (!container) return;

		const valueSpan = container.querySelector('.slider-value');
		const fill = container.querySelector('.slider-fill');
		
		const min = parseFloat(slider.min);
		const max = parseFloat(slider.max);
		const val = parseFloat(slider.value);
		const percent = ((val - min) / (max - min)) * 100;

		if (fill) {
			fill.style.width = percent + '%';
		}

		if (valueSpan) {
			if (slider.id === 'maxFps') {
				valueSpan.textContent = slider.value === '241' ? '∞' : slider.value;
			} else {
				valueSpan.textContent = slider.value + '%';
			}
		}
	}

	function adjustSlider(slider, delta) {
		if (slider.disabled) return;

		const min = parseFloat(slider.min);
		const max = parseFloat(slider.max);
		const step = parseFloat(slider.step) || 1;
		let newValue = parseFloat(slider.value) + (delta * step);
		newValue = Math.max(min, Math.min(max, newValue));
		slider.value = newValue;
		updateSliderDisplay(slider);

		// Send audio changes immediately
		if (slider.id === 'musicVolume' || slider.id === 'effectsVolume') {
			const normalizedValue = (newValue / 100).toFixed(2);
			console.log(`[Settings] audio.${slider.id} ${normalizedValue}`);
		} else {
			stagedGraphics[slider.id] = newValue;
		}
	}

	function toggleCheckbox(checkbox) {
		const isChecked = checkbox.getAttribute('data-checked') === 'true';
		checkbox.setAttribute('data-checked', (!isChecked).toString());
		stagedGraphics[checkbox.id] = !isChecked;
		if (checkbox.id === 'vsync') {
			syncMaxFpsDisabledState();
		}
	}

	function selectLanguage(langItem, notifyRuntime = true) {
		const langCode = langItem.getAttribute('data-lang');
		if (!langCode) return;

		const allLangItems = document.querySelectorAll('#section-language li[data-lang]');
		allLangItems.forEach(li => li.classList.remove('selected'));

		langItem.classList.add('selected');
		selectedLanguage = langCode;
		localizationState.language = langCode;

		if (notifyRuntime) {
			console.log(`[Settings] language ${langCode}`);
		}
	}

	function applyGraphics() {
		// Collect all current graphics settings
		const graphicsSection = document.getElementById('section-graphics');
		if (!graphicsSection) return;
		const vsyncEnabled = isVsyncEnabled();

		// Sliders
		graphicsSection.querySelectorAll('input[type="range"]').forEach(slider => {
			if (slider.id === 'maxFps' && vsyncEnabled) return;
			const value = slider.id === 'maxFps' && slider.value === '241' ? 'unlimited' : slider.value;
			console.log(`[Settings] graphics.${slider.id} ${value}`);
		});

		// Dropdowns
		graphicsSection.querySelectorAll('.dropdown').forEach(dropdown => {
			if (dropdown.id === 'aspectRatio') return;
			console.log(`[Settings] graphics.${dropdown.id} ${dropdown.getAttribute('data-value')}`);
		});

		// Checkboxes
		graphicsSection.querySelectorAll('.checkbox').forEach(checkbox => {
			console.log(`[Settings] graphics.${checkbox.id} ${checkbox.getAttribute('data-checked')}`);
		});

		console.log('[Settings] graphics.apply');
		stagedGraphics = {};
	}

	let fineControl = false;
	window.setFineControl = function(enabled) {
		fineControl = enabled;
	};

	window.menuMoveUp = function() {
		
		if (focusContext.inDropdown) {
			const section = getCurrentSection();
			const row = getNavigableRows(section)[focusContext.rowIndex];
			const dropdown = row?.querySelector('.dropdown.open');
			if (dropdown) {
				setDropdownOptionFocus(dropdown, focusContext.dropdownIndex - 1);
			}
		} else if (focusContext.inSection) {
			{
				setRowFocus(focusContext.rowIndex - 1);
			}
		} else {
			setFocus(focusIndex <= 0 ? menuItems.length - 1 : focusIndex - 1);
		}
	};

	window.menuMoveDown = function() {
		
		if (focusContext.inDropdown) {
			const section = getCurrentSection();
			const row = getNavigableRows(section)[focusContext.rowIndex];
			const dropdown = row?.querySelector('.dropdown.open');
			if (dropdown) {
				setDropdownOptionFocus(dropdown, focusContext.dropdownIndex + 1);
			}
		} else if (focusContext.inSection) {
			{
				setRowFocus(focusContext.rowIndex + 1);
			}
		} else {
			setFocus(focusIndex < 0 ? 0 : focusIndex + 1);
		}
	};

	window.menuMoveLeft = function() {
		if (focusContext.inDropdown) return;
		if (!focusContext.inSection) return;

		const section = getCurrentSection();
		
		const rows = getNavigableRows(section);
		const row = rows[focusContext.rowIndex];
		if (!row) return;

		const slider = row.querySelector('input[type="range"]');
		if (slider) {
			const multiplier = fineControl ? 0.5 : 1;
			adjustSlider(slider, -multiplier);
		}
	};

	window.menuMoveRight = function() {
		if (focusContext.inDropdown) return;
		if (!focusContext.inSection) return;

		const section = getCurrentSection();
		
		const rows = getNavigableRows(section);
		const row = rows[focusContext.rowIndex];
		if (!row) return;

		const slider = row.querySelector('input[type="range"]');
		if (slider) {
			const multiplier = fineControl ? 0.5 : 1;
			adjustSlider(slider, multiplier);
		}
	};

	// Autorwpeat wrappers for held input
	window.menuMoveUpStart = function() { startRepeat(window.menuMoveUp); };
	window.menuMoveDownStart = function() { startRepeat(window.menuMoveDown); };
	window.menuMoveLeftStart = function() { startRepeat(window.menuMoveLeft); };
	window.menuMoveRightStart = function() { startRepeat(window.menuMoveRight); };
	window.menuMoveStop = function() { stopRepeat(); };

	window.menuActivate = function() {
		
		if (focusContext.inSection) {
			const section = getCurrentSection();
			const rows = getNavigableRows(section);
			const row = rows[focusContext.rowIndex];
			if (row) row.classList.add('active');
		} else if (focusIndex >= 0) {
			menuItems[focusIndex].classList.add('active');
		}
	};

	window.menuSelect = function() {
		if (focusContext.inDropdown) {
			// Select dropdown option
			const section = getCurrentSection();
			const row = getNavigableRows(section)[focusContext.rowIndex];
			const dropdown = row?.querySelector('.dropdown.open');
			if (dropdown) {
				const options = Array.from(dropdown.querySelectorAll('.dropdown-options li'));
				if (options[focusContext.dropdownIndex]) {
					selectDropdownOption(dropdown, options[focusContext.dropdownIndex]);
				}
			}
		} else if (focusContext.inSection) {
			const section = getCurrentSection();
			const rows = getNavigableRows(section);
			const row = rows[focusContext.rowIndex];
			if (!row) return;

			row.classList.remove('active');
			row.querySelector('.apply-btn')?.classList.remove('active');

			// Language item
			if (row.hasAttribute('data-lang')) {
				selectLanguage(row);
				return;
			}

			// Dropdown
			const dropdown = row.querySelector('.dropdown');
			if (dropdown) {
				if (dropdown.classList.contains('open')) {
					closeDropdown(dropdown);
				} else {
					openDropdown(dropdown);
				}
				return;
			}

			// Checkbox
			const checkbox = row.querySelector('.checkbox');
			if (checkbox) {
				toggleCheckbox(checkbox);
				return;
			}

			// Apply buttons
			const applyBtn = row.querySelector('.apply-btn');
			if (applyBtn) {
				if (applyBtn.id === 'applyGraphics') {
					applyGraphics();
				}
				return;
			}
		} else if (focusIndex >= 0) {
			menuItems[focusIndex].classList.remove('active');
			const item = menuItems[focusIndex];

			if (item.id === 'back') {
				navigateBack();
				return;
			}

			// Open section
			selectMenuItem(item);

			const sectionId = item.getAttribute('data-section');
			if (sectionId) {
				openSection(sectionId);
			}
		}
	};

	window.menuBack = function() {
		if (focusContext.inDropdown) {
			const section = getCurrentSection();
			const row = getNavigableRows(section)[focusContext.rowIndex];
			const dropdown = row?.querySelector('.dropdown.open');
			if (dropdown) {
				closeDropdown(dropdown);
			}
		} else if (focusContext.inSection) {
			closeSection();
		} else {
			navigateBack();
		}
	};

	function navigateBack() {
		const params = new URLSearchParams(window.location.search);
		const from = params.get('from');
		const backUrl = (from === 'pause') ? 'PauseMenu.html' : 'MainMenu.html';

		content.classList.add('exit-anim');
		bgTint.style.animation = "fadeOut 0.4s ease-in forwards";
		setTimeout(() => {
			window.location.href = backUrl;
		}, 400);
	}

	menuItems.forEach((item, i) => {
		item.addEventListener('click', () => {
			if (item.id === 'back') {
				navigateBack();
				return;
			}

			selectMenuItem(item);

			const sectionId = item.getAttribute('data-section');
			if (sectionId) {
				openSection(sectionId);
			}
		});
		item.addEventListener('mouseenter', () => {
			if (!focusContext.inSection) setFocus(i);
		});
	});

	// Language click handlers
	document.querySelectorAll('#section-language li[data-lang]').forEach((li, i) => {
		li.addEventListener('click', () => selectLanguage(li));
		li.addEventListener('mouseenter', () => {
			if (focusContext.sectionId === 'section-language') {
				setRowFocus(i);
			}
		});
	});

	// Dropdown click handlers
	document.querySelectorAll('.dropdown').forEach(dropdown => {
		dropdown.addEventListener('click', (e) => {
			e.stopPropagation();
			const option = e.target.closest('.dropdown-options li');
			if (option) {
				selectDropdownOption(dropdown, option);
				return;
			}
			if (dropdown.classList.contains('open')) {
				closeDropdown(dropdown);
			} else {
				// Close other dropdowns first
				document.querySelectorAll('.dropdown.open').forEach(d => closeDropdown(d));
				openDropdown(dropdown);
			}
		});

		dropdown.addEventListener('mouseover', (e) => {
			if (!focusContext.inDropdown) return;
			const option = e.target.closest('.dropdown-options li');
			if (!option) return;
			const options = Array.from(dropdown.querySelectorAll('.dropdown-options li'));
			const index = options.indexOf(option);
			if (index >= 0) {
				setDropdownOptionFocus(dropdown, index);
			}
		});
	});

	// Slider input handlers
	document.querySelectorAll('input[type="range"]').forEach(slider => {
		slider.addEventListener('input', () => {
			if (slider.disabled) return;
			updateSliderDisplay(slider);

			// Audio sliders send immediately
			if (slider.id === 'musicVolume' || slider.id === 'effectsVolume') {
				const normalizedValue = (parseFloat(slider.value) / 100).toFixed(2);
				console.log(`[Settings] audio.${slider.id} ${normalizedValue}`);
			} else {
				stagedGraphics[slider.id] = slider.value;
			}
		});
	});

	// Checkbox click handlers
	document.querySelectorAll('.checkbox').forEach(checkbox => {
		checkbox.addEventListener('click', () => toggleCheckbox(checkbox));
	});

	// Apply button click handlers
	document.getElementById('applyGraphics')?.addEventListener('click', applyGraphics);

	// Setting row mouse hover
	document.querySelectorAll('.setting-row[data-nav-row]').forEach((row, i) => {
		row.addEventListener('mouseenter', () => {
			if (focusContext.inSection && !focusContext.inDropdown) {
				const section = getCurrentSection();
				const rows = getNavigableRows(section);
				const idx = rows.indexOf(row);
				if (idx >= 0) setRowFocus(idx);
			}
		});
	});

	// Slider drag support
	document.querySelectorAll('input[type="range"]').forEach(slider => {
		const row = slider.closest('.setting-row[data-nav-row]');
		
		slider.addEventListener('mousedown', () => {
			if (row) row.classList.add('dragging');
		});
		
		slider.addEventListener('mouseup', () => {
			if (row) row.classList.remove('dragging');
		});
		
		slider.addEventListener('mouseleave', () => {
			if (row) row.classList.remove('dragging');
		});
	});

	document.addEventListener('click', () => {
		document.querySelectorAll('.dropdown.open').forEach(d => closeDropdown(d));
	});

	document.querySelectorAll('input[type="range"]').forEach(updateSliderDisplay);
	refreshResolutionOptions();
	syncMaxFpsDisabledState();
	syncDropdownCurrentLabels();
	window.refreshResolutionOptions = refreshResolutionOptions;
	window.applyLocalization();

	console.log('[Settings] init');
});
