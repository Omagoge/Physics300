/**
 * Game HUD Controller - Manages all UI state and updates
 */

const HUD = {
  // State
  state: {
    health: 3,
    maxHealth: 3,
    ammo: 5,
    maxAmmo: 5,
    selectedWeapon: 1,
    weapons: {},
    timerEnabled: true,
    isHubMode: false,
    // Recharge UI state
    rechargeAnimations: {},
    rechargeQueue: [],
    rechargeBusy: false
  },

  /**
   * Initialize HUD systems
   */
  init() {
    this.cacheElements();
    this.attachEventListeners();
    this.effect = this.effect || document.getElementById('effect');
    if (this.state.isHubMode || !this.state.timerEnabled) {
      this.disableSpeedrunTimer();
    }
  },

  /**
   * Cache DOM elements for performance
   */
  cacheElements() {
    this.slots = document.querySelectorAll('.weapon-slot');
    this.healthSegments = document.querySelectorAll('#health-bar .segment');
    this.ammoSegments = document.querySelectorAll('#ammo-bar .segment');
    
    // Timer iframe
    this.timerFrame = document.getElementById('timer-hud-container');
    this.effect = document.getElementById('effect');
    this.damageVignette = document.getElementById('damage-vignette');

    // Initialize recharge animation map
    this.state.rechargeAnimations = {};

    // Initialize all slots as unequipped
    this.slots.forEach(slot => {
      slot.classList.add('unequipped');
      slot.classList.remove('selected');
    });
  },


  /**
   * Update the HUD timer
   */
  updateTimer(totalMilliseconds) {
    // Do nothing if timer is disabled
    if (!this.state.timerEnabled) return;
    if (!this.timerFrame || !this.timerFrame.contentWindow) return;

    const timerWin = this.timerFrame.contentWindow;
    if (typeof timerWin.updateTimer === 'function') {
      timerWin.updateTimer(totalMilliseconds);
    }
  },

  /**
   * Attach click handlers to weapon slots
   */
  attachEventListeners() {
    this.slots.forEach(slot => {
      slot.addEventListener('click', () => {
        const img = slot.querySelector('.weapon-icon');
        // Only allow selection if weapon has an image
        if (img && img.src && img.src.trim() !== '') {
          const index = parseInt(slot.dataset.index);
          this.setSelectedWeapon(index);
        }
      });
    });
  },

  /**
   * Set weapon slot with icon
   */
  setWeaponSlot(index, iconUrl) {
    const slot = document.getElementById(`slot-${index}`);
    if (!slot) return;

    const img = slot.querySelector('.weapon-icon');
    img.src = iconUrl;
    this.state.weapons[index] = { icon: iconUrl };
  },

  /**
   * Set active weapon and highlight it
   * Pass index 0 to deselect all weapons
   */
  setSelectedWeapon(index) {
    // Deselect all if index is 0
    if (index === 0) {
      this.slots.forEach(slot => {
        slot.classList.remove('selected');
        slot.classList.add('unequipped');
      });
      this.state.selectedWeapon = null;
      return;
    }

    const activeSlot = document.getElementById(`slot-${index}`);
    if (!activeSlot) return;
    
    const img = activeSlot.querySelector('.weapon-icon');
    
    // Don't select if weapon has no image
    if (!img || !img.src || img.src.trim() === '') {
      return;
    }

    this.slots.forEach(slot => {
      slot.classList.remove('selected');
      slot.classList.add('unequipped');
    });
    
    activeSlot.classList.remove('unequipped');
    activeSlot.classList.add('selected');
    this.state.selectedWeapon = index;
  },

  /**
   * Update health bar (0-maxHealth segments)
   */
  setHealth(count) {
    console.log(`HUD.setHealth called. desired=${count} prev=${this.state.health}`);
    count = Math.max(0, Math.min(this.state.maxHealth, count));
    const previousHealth = this.state.health;
    this.state.health = count;

    if (count < previousHealth) {
      this.flashDamageVignette(previousHealth - count);
    }

    this.healthSegments.forEach((seg, i) => {
      const isActive = i < count;

      if (isActive) {
        // Cancel any recharge animation for this segment
        if (this.state.rechargeAnimations && this.state.rechargeAnimations[i]) {
          try { this.state.rechargeAnimations[i].cancel(); } catch (e) {}
          delete this.state.rechargeAnimations[i];
        }
        const fill = seg.querySelector('.recharge-fill');

        // Detect if activation came from a recharge to avoid double pop-in
        const rechargePending = seg.dataset.rechargePending === '1';
        const rechargeCompleted = seg.dataset.rechargeCompleted === '1';

        // Cancel any running animations on the fill; do not force it visible here
        if (fill) {
          try { fill.getAnimations().forEach(a => a.cancel()); } catch (e) {}
          // If this segment is currently pending or just completed via recharge, leave the fill alone so the recharge fade can proceed.
          if (!rechargePending && !rechargeCompleted) {
            fill.style.transform = 'scaleX(0)';
            fill.style.opacity = '0';
          }
        }

        seg.classList.remove('recharging');
        seg.classList.add('active');

        // Newly gained segment: skip pop-in if restored by recharge
        if (i >= previousHealth) {
          if (rechargePending || rechargeCompleted) {
            // Clear flags and fade out the fill so active color is visible
            delete seg.dataset.rechargePending;
            delete seg.dataset.rechargeCompleted;
            if (fill) {
              const fade = fill.animate([{ opacity: 1 }, { opacity: 0 }], { duration: 200, easing: 'linear', fill: 'forwards' });
              fade.onfinish = () => { try { fill.style.opacity = '0'; } catch (e) {} };
            }
          } else {
            // Normal pop-in animation
            seg.animate([
              { transform: 'scaleX(0)', transformOrigin: 'left center', opacity: 0.6 },
              { transform: 'scaleX(0)', transformOrigin: 'left center', opacity: 1 },
              { transform: 'scaleX(1)', transformOrigin: 'left center', opacity: 1 }
            ], {
              duration: 300,
              easing: 'ease-out',
              fill: 'forwards'
            });
          }
        }
      } else {
        // Not active: clear active and any animations
        seg.classList.remove('active');
        seg.getAnimations().forEach(anim => anim.cancel());
        const fill = seg.querySelector('.recharge-fill');
        if (fill) { try { fill.getAnimations().forEach(a => a.cancel()); } catch (e) {} try { if (fill.parentNode) fill.parentNode.removeChild(fill); } catch (e) {} }
        seg.classList.remove('recharging');
        delete seg.dataset.rechargePending;
        delete seg.dataset.rechargeCompleted;
      }
    });
  },

  flashDamageVignette(amountLost = 1) {
    if (!this.damageVignette) return;
    const flashClass = amountLost > 1 ? 'hit-flash-strong' : 'hit-flash';
    this.damageVignette.classList.remove('hit-flash-strong');
    this.damageVignette.classList.remove('hit-flash');
    void this.damageVignette.offsetWidth;
    this.damageVignette.classList.add(flashClass);
  },


  /**
   * Update ammo bar (0-maxAmmo segments)
   */
  setAmmoBar(count) {
    count = Math.max(0, Math.min(this.state.maxAmmo, count));
    this.state.ammo = count;

    this.ammoSegments.forEach((seg, i) => {
      seg.classList.toggle('active', i < count);
    });
  },

  /**
   * Set max health and recreate health segments
   */
  setMaxHealth(max) {
    this.state.maxHealth = Math.max(1, max);
    const healthBar = document.getElementById('health-bar');
    healthBar.innerHTML = '';
    for (let i = 0; i < this.state.maxHealth; i++) {
      const segment = document.createElement('div');
      segment.className = 'segment';
      const fill = document.createElement('div');
      fill.className = 'recharge-fill';
      segment.appendChild(fill);
      healthBar.appendChild(segment);
    }
    this.healthSegments = document.querySelectorAll('#health-bar .segment');
    this.state.rechargeAnimations = {};
    this.state.rechargeQueue = [];
    this.state.rechargeBusy = false;
    this.setHealth(Math.min(this.state.health, this.state.maxHealth));
  },


  /**
   * Set max ammo and recreate ammo segments
   */
  setMaxAmmo(max) {
    this.state.maxAmmo = Math.max(1, max);
    const ammoBar = document.getElementById('ammo-bar');
    ammoBar.innerHTML = '';
    for (let i = 0; i < this.state.maxAmmo; i++) {
      const segment = document.createElement('div');
      segment.className = 'segment';
      ammoBar.appendChild(segment);
    }
    this.ammoSegments = document.querySelectorAll('#ammo-bar .segment');
    this.setAmmoBar(Math.min(this.state.ammo, this.state.maxAmmo));
  },

  /**
   * Fade screen to black (called by C++ on player death)
   */
  fadeIn() {
    if (!this.effect) return;
    this.effect.classList.remove('is-transparent');
    this.effect.classList.add('is-black');
  },

  /**
   * Fade screen from black back to transparent (called by C++ on respawn)
   */
  fadeOut() {
    if (!this.effect) return;
    this.effect.classList.remove('is-black');
    this.effect.classList.add('is-transparent');
  },

  /**
   * When in Hub mode the speedrun timer is disabled and hidden
   */
  setHubMode(isHub) {
    this.state.isHubMode = !!isHub;
    if (this.state.isHubMode) {
      this.disableSpeedrunTimer();
    } else {
      this.enableSpeedrunTimer();
    }
  },

  /**
   * Disable the spedrub timer
   */
  disableSpeedrunTimer() {
    this.state.timerEnabled = false;
    if (this.timerFrame) {
      try { this.timerFrame.style.display = 'none'; } catch (e) {}
    }
  },

  /**
   * Enable the spedrub timer
   */
  enableSpeedrunTimer() {
    this.state.timerEnabled = true;
    if (this.timerFrame) {
      try { this.timerFrame.style.display = ''; } catch (e) {}
    }
  },

  /**
   * Damage player (decrease health)
   */
  takeDamage(amount = 1) {
    this.setHealth(this.state.health - amount);
  },

  /**
   * Heal player
   */
  heal(amount = 1) {
    this.setHealth(this.state.health + amount);
  },

  completeRecharge(index) {
    console.log('HUD.completeRecharge', index);
    index = parseInt(index);
    if (isNaN(index) || index < 0 || index >= this.state.maxHealth) return;
    const seg = this.healthSegments[index];
    if (!seg) return;
    const fill = seg.querySelector('.recharge-fill');
    // mark completed so setHealth can skip pop-in; keep flag until setHealth clears it
    seg.dataset.rechargeCompleted = '1';
    delete seg.dataset.rechargePending;
    seg.classList.remove('recharging');
    if (fill) {
      try { fill.getAnimations().forEach(a => a.cancel()); } catch (e) {}
      fill.style.transform = 'scaleX(1)';
      fill.style.opacity = '1';
      // fade out to reveal active color then remove the fill element to avoid overlay
      const fade = fill.animate([{ opacity: 1 }, { opacity: 0 }], { duration: 200, easing: 'linear', fill: 'forwards' });
      // mark busy while fade is in progress
      this.state.rechargeBusy = true;
      fade.onfinish = () => {
        try { fill.style.opacity = '0'; fill.style.display = 'none'; if (fill.parentNode) fill.parentNode.removeChild(fill); } catch (e) {}
        seg.classList.remove('recharging');
        delete seg.dataset.rechargeCompleted;
        delete seg.dataset.rechargePending;
        // unlock and process queued recharges
        this.state.rechargeBusy = false;
        if (this.state.rechargeQueue && this.state.rechargeQueue.length > 0) {
          const next = this.state.rechargeQueue.shift();
          console.log('HUD: processing queued recharge', next.index, next.durationMs);
          setTimeout(() => { this.startRecharge(next.index, next.durationMs); }, 10);
        }
      };
    }
  },

  startRecharge(index, durationMs) {
    console.log('HUD.startRecharge', index, durationMs);
    index = parseInt(index);
    if (isNaN(index) || index < 0 || index >= this.state.maxHealth) return;

    // If HUD is finalizing a previous recharge, queue this request to avoid races
    if (this.state.rechargeBusy) {
      this.state.rechargeQueue = this.state.rechargeQueue || [];
      // avoid duplicate queue entries for the same index
      if (!this.state.rechargeQueue.some(q => q.index === index)) {
        console.log('HUD.startRecharge - HUD busy, queueing', index);
        this.state.rechargeQueue.push({ index, durationMs });
      } else {
        console.log('HUD.startRecharge - duplicate queued, ignoring', index);
      }
      return;
    }

    const seg = this.healthSegments[index];
    if (!seg) return;

    // ensure recharge-fill exists
    let fill = seg.querySelector('.recharge-fill');
    if (!fill) {
      fill = document.createElement('div');
      fill.className = 'recharge-fill';
      seg.appendChild(fill);
    }

    // cancel existing animation on this index
    if (this.state.rechargeAnimations && this.state.rechargeAnimations[index]) {
      try { this.state.rechargeAnimations[index].cancel(); } catch (e) {}
      delete this.state.rechargeAnimations[index];
    }

    seg.classList.add('recharging');
    seg.dataset.rechargePending = '1';
    fill.style.display = 'block';
    fill.style.transform = 'scaleX(0)';
    fill.style.opacity = '0.6';

    const anim = fill.animate([
      { transform: 'scaleX(0)', opacity: 0.6 },
      { transform: 'scaleX(1)', opacity: 1 }
    ], {
      duration: durationMs,
      easing: 'linear',
      fill: 'forwards'
    });

    this.state.rechargeAnimations = this.state.rechargeAnimations || {};
    this.state.rechargeAnimations[index] = anim;
    anim.onfinish = () => {
      // mark completed (race-safe) and fade out the fill so the active color shows
      seg.dataset.rechargePending = '0';
      seg.dataset.rechargeCompleted = '1';
      // lock HUD to avoid starting another recharge until fade completes
      this.state.rechargeBusy = true;
      const fade = fill.animate([{ opacity: 1 }, { opacity: 0 }], { duration: 200, easing: 'linear', fill: 'forwards' });
      fade.onfinish = () => {
        try { fill.style.opacity = '0'; fill.style.display = 'none'; if (fill.parentNode) fill.parentNode.removeChild(fill); } catch (e) {}
        seg.classList.remove('recharging');
        // allow setHealth to detect the completed flag briefly, then clear it
        setTimeout(() => { delete seg.dataset.rechargeCompleted; }, 50);
        // unlock and process queued startRecharge requests (if any)
        this.state.rechargeBusy = false;
        if (this.state.rechargeQueue && this.state.rechargeQueue.length > 0) {
          const next = this.state.rechargeQueue.shift();
          console.log('HUD: processing queued recharge', next.index, next.durationMs);
          // schedule next start to avoid deep recursion
          setTimeout(() => { this.startRecharge(next.index, next.durationMs); }, 10);
        }
      };
      delete this.state.rechargeAnimations[index];
    };
  },

  cancelRecharge(index) {
    console.log('HUD.cancelRecharge', index);
    // clear any queued recharges and unlock HUD so cancellations take effect immediately
    this.state.rechargeQueue = [];
    this.state.rechargeBusy = false;
    const doCancelForSeg = (segIndex) => {
      const seg = this.healthSegments[segIndex];
      if (!seg) return;
      if (this.state.rechargeAnimations && this.state.rechargeAnimations[segIndex]) {
        try { this.state.rechargeAnimations[segIndex].cancel(); } catch (e) {}
        delete this.state.rechargeAnimations[segIndex];
      }
      seg.classList.remove('recharging');
      delete seg.dataset.rechargePending;
      delete seg.dataset.rechargeCompleted;
      const fill = seg.querySelector('.recharge-fill');
      if (fill) {
        try { fill.getAnimations().forEach(a => a.cancel()); } catch (e) {}
        try { if (fill.parentNode) fill.parentNode.removeChild(fill); } catch (e) {}
      }
    };

    if (typeof index === 'number') {
      doCancelForSeg(index);
      return;
    }

    // cancel all
    for (let i = 0; i < this.healthSegments.length; ++i) {
      doCancelForSeg(i);
    }
  },

  /**
   * Consume ammo
   */
  consumeAmmo(amount = 1) {
    this.setAmmoBar(this.state.ammo - amount);
  },

  /**
   * Set reserve ammo display for a weapon slot
   */
  setReserveAmmo(index, number) {
    const el = document.getElementById(`reserve-ammo-${index}`);
    if (el) {
      el.innerText = number;
    }
  },

  /**
   * Remove weapon image and auto-switch to other weapon if available
   */
  removeWeapon(index) {
    const slot = document.getElementById(`slot-${index}`);
    if (!slot) return;

    const img = slot.querySelector('.weapon-icon');
    if (img) {
      img.src = '';
    }
    slot.classList.remove('selected');
    slot.classList.add('unequipped');

    // Find and equip the other weapon if it has an image
    const otherIndex = index === 1 ? 2 : 1;
    const otherSlot = document.getElementById(`slot-${otherIndex}`);
    const otherImg = otherSlot?.querySelector('.weapon-icon');
    
    if (otherImg && otherImg.src && otherImg.src.trim() !== '') {
      this.setSelectedWeapon(otherIndex);
    } else {
      // No other weapon available, deselect all
      this.state.selectedWeapon = null;
    }
  },
};


function setReserveAmmo(index, number) {
  const slotIndex = index + 1;
  function tryUpdate() {
    if (typeof HUD !== 'undefined' && typeof HUD.setReserveAmmo === 'function') {
      HUD.setReserveAmmo(slotIndex, number);
    } else {
      setTimeout(tryUpdate, 50);
    }
  }
  tryUpdate();
}

function setWeaponSlot(index, iconUrl) {
  function tryUpdate() {
    if (typeof HUD !== 'undefined' && typeof HUD.setWeaponSlot === 'function') {
      HUD.setWeaponSlot(index, iconUrl);
    } else {
      setTimeout(tryUpdate, 50);
    }
  }
  tryUpdate();
}

function updateTimer(totalMilliseconds) {
  function tryUpdate() {
    if (typeof HUD !== 'undefined' && typeof HUD.updateTimer === 'function') {
      HUD.updateTimer(totalMilliseconds);
    } else {
      setTimeout(tryUpdate, 50);
    }
  }
  tryUpdate();
}

function enterHubMode() {
  function tryCall() {
    if (typeof HUD !== 'undefined' && typeof HUD.setHubMode === 'function') {
      HUD.setHubMode(true);
    } else {
      setTimeout(tryCall, 50);
    }
  }
  tryCall();
}

function exitHubMode() {
  function tryCall() {
    if (typeof HUD !== 'undefined' && typeof HUD.setHubMode === 'function') {
      HUD.setHubMode(false);
    } else {
      setTimeout(tryCall, 50);
    }
  }
  tryCall();
}

const menu_fade = document.getElementById('effect');

// function fadeIn() {
//   menu_fade.classList.remove('is-transparent');
//   menu_fade.classList.add('is-black');
// }
//
// function fadeOut() {
//   menu_fade.classList.remove('is-black');
//   menu_fade.classList.add('is-transparent');
// }

// Initialize on DOM ready
document.addEventListener('DOMContentLoaded', () => {
  HUD.init();

  // Example usage (remove when integrating with game)
  // HUD.setWeaponSlot(1, '');
  // HUD.setWeaponSlot(2, '');
  // HUD.setSelectedWeapon(1);
  HUD.setHealth(3);
  HUD.setAmmoBar(3);
});
