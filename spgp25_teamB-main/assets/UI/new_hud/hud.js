const SPRITES = {
  heartDead: "_0002_Heart_dead.png",
  heartAlive: "_0001_Heart_alive.png",
  heartOutline: "_0000_Heart_outline.png",
  pistolIcon: "_0005_Pistol_icon.png",
  shotgunIcon: "_0003_Shotgun_icon.png",
  ammo: {
    pistol: {
      filled: "_0020_Bullet.png",
      empty: "_0017_Bullet_empty.png"
    },
    shotgun: {
      filled: "_0019_Bullet_big.png",
      empty: "_0018_Big_bullet_empty.png"
    }
  }
};

const HUD = {
  state: {
    health: 3,
    maxHealth: 3,
    ammo: 5,
    maxAmmo: 5,
    selectedWeapon: 1,
    weapons: {},
    timerEnabled: true,
    isHubMode: false,
    rechargeAnimations: {},
    rechargeQueue: [],
    rechargeBusy: false
  },

  init() {
    this.cacheElements();
    this.attachEventListeners();
    if (this.state.isHubMode || !this.state.timerEnabled) {
      this.disableSpeedrunTimer();
    }
    this.refreshAmmoSprites();
  },

  cacheElements() {
    this.slots = document.querySelectorAll(".weapon-slot");
    this.healthBar = document.getElementById("health-bar");
    this.ammoBar = document.getElementById("ammo-bar");
    this.healthSegments = this.healthBar.querySelectorAll(".heart-segment");
    this.ammoSegments = this.ammoBar.querySelectorAll(".ammo-segment");
    this.timerFrame = document.getElementById("timer-hud-container");
    this.effect = document.getElementById("effect");
    this.damageVignette = document.getElementById("damage-vignette");
    this.state.rechargeAnimations = {};

    this.slots.forEach(slot => {
      slot.classList.add("unequipped");
      slot.classList.remove("selected");
    });
  },

  attachEventListeners() {
    this.slots.forEach(slot => {
      slot.addEventListener("click", () => {
        const img = slot.querySelector(".weapon-icon");
        if (img && img.src && img.src.trim() !== "") {
          const index = parseInt(slot.dataset.index, 10);
          this.setSelectedWeapon(index);
        }
      });
    });
  },

  updateTimer(totalMilliseconds) {
    if (!this.state.timerEnabled) return;
    if (!this.timerFrame || !this.timerFrame.contentWindow) return;

    const timerWin = this.timerFrame.contentWindow;
    if (typeof timerWin.updateTimer === "function") {
      timerWin.updateTimer(totalMilliseconds);
    }
  },

  setWeaponSlot(index, iconUrl) {
    const slot = document.getElementById(`slot-${index}`);
    if (!slot) return;

    const img = slot.querySelector(".weapon-icon");
    if (!img) return;
    img.src = iconUrl;
    this.state.weapons[index] = { icon: iconUrl };

    if (this.state.selectedWeapon === index) {
      this.refreshAmmoSprites();
    }
  },

  setSelectedWeapon(index) {
    if (index === 0) {
      this.slots.forEach(slot => {
        slot.classList.remove("selected");
        slot.classList.add("unequipped");
      });
      this.state.selectedWeapon = null;
      this.refreshAmmoSprites();
      return;
    }

    const activeSlot = document.getElementById(`slot-${index}`);
    if (!activeSlot) return;
    const img = activeSlot.querySelector(".weapon-icon");
    if (!img || !img.src || img.src.trim() === "") return;

    this.slots.forEach(slot => {
      slot.classList.remove("selected");
      slot.classList.add("unequipped");
    });

    activeSlot.classList.remove("unequipped");
    activeSlot.classList.add("selected");
    this.state.selectedWeapon = index;
    this.refreshAmmoSprites();
  },

  setHeartFill(segment, value) {
    const mask = segment.querySelector(".heart-alive-mask");
    if (!mask) return;
    const normalized = Math.max(0, Math.min(1, Number(value) || 0));
    mask.style.width = `${normalized * 100}%`;
  },

  animateHeartFill(segment, duration) {
    const mask = segment.querySelector(".heart-alive-mask");
    if (!mask) return;
    try { mask.getAnimations().forEach(anim => anim.cancel()); } catch (e) {}
    mask.animate(
      [
        { width: "0%" },
        { width: "100%" }
      ],
      {
        duration,
        easing: "ease-out",
        fill: "forwards"
      }
    );
  },

  setHealth(count) {
    count = Math.max(0, Math.min(this.state.maxHealth, count));
    const previousHealth = this.state.health;
    this.state.health = count;

    if (count < previousHealth) {
      this.flashDamageVignette(previousHealth - count);
    }

    this.healthSegments.forEach((seg, i) => {
      const isActive = i < count;
      const rechargePending = seg.dataset.rechargePending === "1";
      const rechargeCompleted = seg.dataset.rechargeCompleted === "1";

      if (isActive) {
        seg.classList.add("active");

        if (!rechargePending && !rechargeCompleted) {
          if (i >= previousHealth) {
            this.animateHeartFill(seg, 300);
          } else {
            this.setHeartFill(seg, 1);
          }
        }

        if (rechargeCompleted) {
          delete seg.dataset.rechargeCompleted;
        }
      } else {
        seg.classList.remove("active");
        this.stopRechargeForIndex(i);
        this.setHeartFill(seg, 0);
        delete seg.dataset.rechargePending;
        delete seg.dataset.rechargeCompleted;
      }
    });
  },

  flashDamageVignette(amountLost = 1) {
    if (!this.damageVignette) return;
    const flashClass = amountLost > 1 ? "hit-flash-strong" : "hit-flash";
    this.damageVignette.classList.remove("hit-flash-strong");
    this.damageVignette.classList.remove("hit-flash");
    void this.damageVignette.offsetWidth;
    this.damageVignette.classList.add(flashClass);
  },

  createHeartSegment() {
    const segment = document.createElement("div");
    segment.className = "heart-segment";
    segment.innerHTML = `
      <img class="heart-layer heart-dead" src="${SPRITES.heartDead}" alt="">
      <div class="heart-alive-mask">
        <img class="heart-layer heart-alive" src="${SPRITES.heartAlive}" alt="">
      </div>
      <img class="heart-layer heart-outline" src="${SPRITES.heartOutline}" alt="">
    `;
    return segment;
  },

  setMaxHealth(max) {
    this.state.maxHealth = Math.max(1, max);
    this.healthBar.innerHTML = "";
    for (let i = 0; i < this.state.maxHealth; i++) {
      this.healthBar.appendChild(this.createHeartSegment());
    }

    this.healthSegments = this.healthBar.querySelectorAll(".heart-segment");
    this.state.rechargeAnimations = {};
    this.state.rechargeQueue = [];
    this.state.rechargeBusy = false;
    this.setHealth(Math.min(this.state.health, this.state.maxHealth));
  },

  createAmmoSegment() {
    const segment = document.createElement("div");
    segment.className = "ammo-segment";
    const img = document.createElement("img");
    img.className = "ammo-pip";
    img.alt = "";
    segment.appendChild(img);
    return segment;
  },

  setMaxAmmo(max) {
    this.state.maxAmmo = Math.max(1, max);
    this.ammoBar.innerHTML = "";
    for (let i = 0; i < this.state.maxAmmo; i++) {
      this.ammoBar.appendChild(this.createAmmoSegment());
    }
    this.ammoSegments = this.ammoBar.querySelectorAll(".ammo-segment");
    this.setAmmoBar(Math.min(this.state.ammo, this.state.maxAmmo));
  },

  isShotgunIcon(iconPath) {
    const path = (iconPath || "").toLowerCase();
    return path.includes("shotgun");
  },

  getSelectedWeaponType() {
    if (!this.state.selectedWeapon) return "pistol";
    const selected = this.state.weapons[this.state.selectedWeapon];
    if (!selected || !selected.icon) return "pistol";
    return this.isShotgunIcon(selected.icon) ? "shotgun" : "pistol";
  },

  refreshAmmoSprites() {
    const type = this.getSelectedWeaponType();
    const ammoSprites = type === "shotgun" ? SPRITES.ammo.shotgun : SPRITES.ammo.pistol;
    if (this.ammoBar) {
      this.ammoBar.classList.toggle("shotgun-ammo", type === "shotgun");
      this.ammoBar.classList.toggle("pistol-ammo", type !== "shotgun");
    }

    this.ammoSegments.forEach((seg, i) => {
      const img = seg.querySelector(".ammo-pip");
      if (!img) return;
      img.src = i < this.state.ammo ? ammoSprites.filled : ammoSprites.empty;
    });
  },

  setAmmoBar(count) {
    count = Math.max(0, Math.min(this.state.maxAmmo, count));
    this.state.ammo = count;
    this.refreshAmmoSprites();
  },

  setHubMode(isHub) {
    this.state.isHubMode = !!isHub;
    if (this.state.isHubMode) {
      this.disableSpeedrunTimer();
    } else {
      this.enableSpeedrunTimer();
    }
  },

  disableSpeedrunTimer() {
    this.state.timerEnabled = false;
    if (this.timerFrame) {
      this.timerFrame.style.display = "none";
    }
  },

  enableSpeedrunTimer() {
    this.state.timerEnabled = true;
    if (this.timerFrame) {
      this.timerFrame.style.display = "";
    }
  },

  fadeIn() {
    if (!this.effect) return;
    this.effect.classList.remove("is-transparent");
    this.effect.classList.add("is-black");
  },

  fadeOut() {
    if (!this.effect) return;
    this.effect.classList.remove("is-black");
    this.effect.classList.add("is-transparent");
  },

  completeRecharge(index) {
    index = parseInt(index, 10);
    if (isNaN(index) || index < 0 || index >= this.state.maxHealth) return;

    const seg = this.healthSegments[index];
    if (!seg) return;

    this.stopRechargeForIndex(index);
    seg.dataset.rechargeCompleted = "1";
    delete seg.dataset.rechargePending;
    seg.classList.remove("recharging");
    this.setHeartFill(seg, 1);

    this.state.rechargeBusy = false;
    this.processRechargeQueue();
  },

  processRechargeQueue() {
    if (this.state.rechargeBusy) return;
    if (!this.state.rechargeQueue || this.state.rechargeQueue.length === 0) return;
    const next = this.state.rechargeQueue.shift();
    setTimeout(() => this.startRecharge(next.index, next.durationMs), 10);
  },

  startRecharge(index, durationMs) {
    index = parseInt(index, 10);
    if (isNaN(index) || index < 0 || index >= this.state.maxHealth) return;

    if (this.state.rechargeBusy) {
      if (!this.state.rechargeQueue.some(item => item.index === index)) {
        this.state.rechargeQueue.push({ index, durationMs });
      }
      return;
    }

    const seg = this.healthSegments[index];
    if (!seg) return;

    const mask = seg.querySelector(".heart-alive-mask");
    if (!mask) return;

    this.stopRechargeForIndex(index);
    this.state.rechargeBusy = true;
    seg.classList.add("recharging");
    seg.dataset.rechargePending = "1";
    delete seg.dataset.rechargeCompleted;
    this.setHeartFill(seg, 0);

    const anim = mask.animate(
      [
        { width: "0%" },
        { width: "100%" }
      ],
      {
        duration: Math.max(1, Number(durationMs) || 1),
        easing: "linear",
        fill: "forwards"
      }
    );

    this.state.rechargeAnimations[index] = anim;
    anim.onfinish = () => {
      delete this.state.rechargeAnimations[index];
      seg.dataset.rechargePending = "0";
      seg.dataset.rechargeCompleted = "1";
      this.state.rechargeBusy = false;
      this.processRechargeQueue();
    };
    anim.oncancel = () => {
      delete this.state.rechargeAnimations[index];
      this.state.rechargeBusy = false;
      this.processRechargeQueue();
    };
  },

  stopRechargeForIndex(index) {
    if (this.state.rechargeAnimations[index]) {
      try { this.state.rechargeAnimations[index].cancel(); } catch (e) {}
      delete this.state.rechargeAnimations[index];
    }

    const seg = this.healthSegments[index];
    if (!seg) return;
    const mask = seg.querySelector(".heart-alive-mask");
    if (mask) {
      try { mask.getAnimations().forEach(a => a.cancel()); } catch (e) {}
    }
  },

  cancelRecharge(index) {
    this.state.rechargeQueue = [];
    this.state.rechargeBusy = false;

    const cancelOne = segIndex => {
      this.stopRechargeForIndex(segIndex);
      const seg = this.healthSegments[segIndex];
      if (!seg) return;
      seg.classList.remove("recharging");
      delete seg.dataset.rechargePending;
      delete seg.dataset.rechargeCompleted;
      this.setHeartFill(seg, segIndex < this.state.health ? 1 : 0);
    };

    if (typeof index === "number") {
      cancelOne(index);
      return;
    }

    for (let i = 0; i < this.healthSegments.length; i++) {
      cancelOne(i);
    }
  },

  consumeAmmo(amount = 1) {
    this.setAmmoBar(this.state.ammo - amount);
  },

  setReserveAmmo(index, number) {
    const el = document.getElementById(`reserve-ammo-${index}`);
    if (el) {
      el.innerText = number;
    }
  },

  removeWeapon(index) {
    const slot = document.getElementById(`slot-${index}`);
    if (!slot) return;

    const img = slot.querySelector(".weapon-icon");
    if (img) {
      img.src = "";
    }

    delete this.state.weapons[index];
    slot.classList.remove("selected");
    slot.classList.add("unequipped");

    const otherIndex = index === 1 ? 2 : 1;
    const otherSlot = document.getElementById(`slot-${otherIndex}`);
    const otherImg = otherSlot ? otherSlot.querySelector(".weapon-icon") : null;

    if (otherImg && otherImg.src && otherImg.src.trim() !== "") {
      this.setSelectedWeapon(otherIndex);
    } else {
      this.state.selectedWeapon = null;
      this.refreshAmmoSprites();
    }
  }
};

function setReserveAmmo(index, number) {
  const slotIndex = index + 1;
  function tryUpdate() {
    if (typeof HUD !== "undefined" && typeof HUD.setReserveAmmo === "function") {
      HUD.setReserveAmmo(slotIndex, number);
    } else {
      setTimeout(tryUpdate, 50);
    }
  }
  tryUpdate();
}

function setWeaponSlot(index, iconUrl) {
  function tryUpdate() {
    if (typeof HUD !== "undefined" && typeof HUD.setWeaponSlot === "function") {
      HUD.setWeaponSlot(index, iconUrl);
    } else {
      setTimeout(tryUpdate, 50);
    }
  }
  tryUpdate();
}

function updateTimer(totalMilliseconds) {
  function tryUpdate() {
    if (typeof HUD !== "undefined" && typeof HUD.updateTimer === "function") {
      HUD.updateTimer(totalMilliseconds);
    } else {
      setTimeout(tryUpdate, 50);
    }
  }
  tryUpdate();
}

function enterHubMode() {
  function tryCall() {
    if (typeof HUD !== "undefined" && typeof HUD.setHubMode === "function") {
      HUD.setHubMode(true);
    } else {
      setTimeout(tryCall, 50);
    }
  }
  tryCall();
}

function exitHubMode() {
  function tryCall() {
    if (typeof HUD !== "undefined" && typeof HUD.setHubMode === "function") {
      HUD.setHubMode(false);
    } else {
      setTimeout(tryCall, 50);
    }
  }
  tryCall();
}

function fadeIn() {
  if (typeof HUD !== "undefined" && typeof HUD.fadeIn === "function") {
    HUD.fadeIn();
  }
}

function fadeOut() {
  if (typeof HUD !== "undefined" && typeof HUD.fadeOut === "function") {
    HUD.fadeOut();
  }
}

document.addEventListener("DOMContentLoaded", () => {
  HUD.init();
  HUD.setHealth(3);
  HUD.setAmmoBar(3);
});
