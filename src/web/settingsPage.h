#pragma once

#include <pgmspace.h>

/** Settings page served for GET /, no external assets so it works without an internet uplink. */
const char kSettingsPage[] PROGMEM = R"HTML(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>punkt</title>
<meta name="color-scheme" content="light dark">
<!-- data: URI can't read CSS custom properties, the dots carry the light accent as a literal -->
<link rel="icon" href="data:image/svg+xml,<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 20 20'><circle cx='6' cy='6' r='3' fill='%232563eb'/><circle cx='14' cy='6' r='3' fill='%232563eb'/><circle cx='6' cy='14' r='3' fill='%232563eb'/></svg>">
<style>
:root {
  --bg: #f4f4f5;
  --card: #ffffff;
  --fg: #18181b;
  --muted: #71717a;
  --line: #d4d4d8;
  --accent: #2563eb;
  --danger: #b91c1c;
  --on-accent: #ffffff;
  --space: 8px;
  --radius: 10px;
}
@media (prefers-color-scheme: dark) {
  :root {
    --bg: #09090b;
    --card: #18181b;
    --fg: #f4f4f5;
    --muted: #a1a1aa;
    --line: #71717a;
    --accent: #2563eb;
    --danger: #b91c1c;
  }
}
* { box-sizing: border-box; }
body { margin: 0; padding: calc(var(--space) * 2); background: var(--bg); color: var(--fg); font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
main { max-width: 420px; margin: 0 auto; }
h1 { font-size: 1.4rem; margin: 0 0 calc(var(--space) * 2); }
.logo { height: 1em; width: 1em; margin-right: var(--space); vertical-align: -0.125em; }
section { background: var(--card); border: 1px solid var(--line); border-radius: var(--radius); padding: calc(var(--space) * 1.5); margin-bottom: calc(var(--space) * 1.5); }
h2 { font-size: 0.75rem; text-transform: uppercase; letter-spacing: 0.08em; color: var(--muted); margin: 0 0 var(--space); }
dl { display: grid; grid-template-columns: auto 1fr; gap: calc(var(--space) / 2) var(--space); margin: 0; font-size: 0.9rem; }
dt { color: var(--muted); }
dd { margin: 0; min-width: 0; text-align: right; font-variant-numeric: tabular-nums; overflow-wrap: anywhere; white-space: pre-line; }
label { display: block; margin: var(--space) 0 calc(var(--space) / 2); font-size: 0.85rem; color: var(--muted); }
input, select, button { width: 100%; font: inherit; min-height: 44px; padding: var(--space); border: 1px solid var(--line); border-radius: var(--radius); background: var(--card); color: var(--fg); }
input[type=range] { padding: 0; border: 0; background: none; }
/* iOS sizes time inputs by their content and ignores width until the native appearance is off */
input[type=time] { -webkit-appearance: none; appearance: none; display: block; min-width: 0; max-width: 100%; text-align: left; }
label.check { display: flex; align-items: center; gap: var(--space); color: var(--fg); }
label.check input { width: auto; min-height: 0; }
label:has(+ :disabled), :disabled, .dim { opacity: 0.5; }
button { margin-top: var(--space); background: var(--accent); color: var(--on-accent); border: 0; font-weight: 600; }
button.danger { background: var(--danger); }
:focus-visible { outline: 3px solid var(--accent); outline-offset: 2px; }
#log { min-height: 1.2rem; margin: 0; text-align: center; font-size: 0.85rem; color: var(--muted); }
p.note { margin: calc(var(--space) / 2) 0 0; font-size: 0.8rem; color: var(--muted); }
</style>
</head>
<body>
<main>
<h1><svg class="logo" viewBox="0 0 20 20" aria-hidden="true" focusable="false"><circle cx="6" cy="6" r="3" fill="currentColor"/><circle cx="14" cy="6" r="3" fill="currentColor"/><circle cx="6" cy="14" r="3" fill="currentColor"/></svg><span id="board-name">punkt</span></h1>
<section>
<h2>Status</h2>
<dl>
<dt>state</dt><dd id="v-state">-</dd>
<dt>view</dt><dd id="v-view">-</dd>
<dt>brightness</dt><dd id="v-brightness">-</dd>
<dt>sessions</dt><dd id="v-sessions">-</dd>
<dt>context</dt><dd id="v-context">-</dd>
<dt>usage</dt><dd id="v-usage">-</dd>
<dt>uptime</dt><dd id="v-uptime">-</dd>
<dt>ip</dt><dd id="v-ip">-</dd>
<dt>version</dt><dd id="v-version">-</dd>
<dt>temperature</dt><dd id="v-temp">-</dd>
<dt>touch</dt><dd id="v-touch">-</dd>
</dl>
</section>
<section>
<h2>Brightness</h2>
<label for="brightness">Brightness, 0 to 15: <span id="brightness-value">-</span></label>
<input type="range" id="brightness" min="0" max="15" step="1">
<label for="nightbrightness">Dimmed brightness, 0 to 15: <span id="nightbrightness-value">-</span></label>
<input type="range" id="nightbrightness" min="0" max="15" step="1">
<p class="note">Level 0 is the dimmest step, not off.</p>
<label class="check"><input type="checkbox" id="autodim"> Use it at night, following the sun</label>
<p class="note" id="city-note">-</p>
<label for="dim">Use it when the clock rests for (minutes)</label>
<input type="number" id="dim" min="0" max="1440">
<p class="note">0 never dims.</p>
</section>
<section>
<h2>Location and clock</h2>
<label for="city">City (timezone and sun position)</label>
<select id="city"></select>
<button type="button" id="city-device">Take the city from this browser's timezone</button>
<p class="note">Uses the timezone of the phone or laptop showing this page, the board has no location of its own.</p>
<label for="clockface" id="clockface-label">Clock face</label>
<select id="clockface"><option value="staircase">Staircase</option><option value="tall">Tall</option></select>
<label for="splitclock" id="splitclock-label">Split clock</label>
<select id="splitclock"><option value="digits">3x3 digits</option><option value="squares">Squares</option><option value="stagger">Stagger</option><option value="bar">Bar</option></select>
</section>
<section>
<h2>Screen off</h2>
<label for="clock">Back to clock after (minutes)</label>
<input type="number" id="clock" min="0" max="1440">
<label for="off">Screen off after a further (minutes)</label>
<input type="number" id="off" min="0" max="1440">
<p class="note">0 disables a timeout. A Claude hook or a tap lights the screen again, a double tap switches it off until the next double tap.</p>
<label class="check"><input type="checkbox" id="nightoff"> Off at night</label>
<label for="sleepat">From</label>
<input type="time" id="sleepat">
<label for="wakeat">To</label>
<input type="time" id="wakeat">
<label class="check"><input type="checkbox" id="nightsleep"> Deep sleep during the night (saves power, only a double tap or the end time wakes it)</label>
<label for="batterysleep" id="batterysleep-label">Sleep on battery after (min, 0 = never)</label>
<input type="number" id="batterysleep" min="0" max="240">
</section>
<section>
<h2>Animations</h2>
<label for="anim-thinking">Thinking</label>
<select id="anim-thinking"><option>tetris</option><option>life</option><option>knight</option><option>orbit</option><option>code</option><option>rain</option><option>scan</option><option>pipes</option><option>pointer</option><option>pong</option><option>terminal</option></select>
<label for="anim-reading">Reading</label>
<select id="anim-reading"><option>tetris</option><option>life</option><option>knight</option><option>orbit</option><option>code</option><option>rain</option><option>scan</option><option>pipes</option><option>pointer</option><option>pong</option><option>terminal</option></select>
<label for="anim-editing">Editing</label>
<select id="anim-editing"><option>tetris</option><option>life</option><option>knight</option><option>orbit</option><option>code</option><option>rain</option><option>scan</option><option>pipes</option><option>pointer</option><option>pong</option><option>terminal</option></select>
<label for="anim-shell">Shell</label>
<select id="anim-shell"><option>tetris</option><option>life</option><option>knight</option><option>orbit</option><option>code</option><option>rain</option><option>scan</option><option>pipes</option><option>pointer</option><option>pong</option><option>terminal</option></select>
<label for="anim-delegating">Delegating</label>
<select id="anim-delegating"><option>tetris</option><option>life</option><option>knight</option><option>orbit</option><option>code</option><option>rain</option><option>scan</option><option>pipes</option><option>pointer</option><option>pong</option><option>terminal</option></select>
<label for="anim-mcp">MCP</label>
<select id="anim-mcp"><option>tetris</option><option>life</option><option>knight</option><option>orbit</option><option>code</option><option>rain</option><option>scan</option><option>pipes</option><option>pointer</option><option>pong</option><option>terminal</option></select>
</section>
<section>
<h2>Touch</h2>
<label for="touch">Trigger, percent above the baseline, 5 to 250: <span id="touch-value">-</span></label>
<input type="range" id="touch" min="5" max="250" step="1">
<label for="hover">Hover, where the dome starts as percent of the trigger, 1 to 50, 0 turns it off: <span id="hover-value">-</span></label>
<input type="range" id="hover" min="0" max="50" step="1">
<label class="check"><input type="checkbox" id="debugtouch"> Touch debug stream on serial</label>
</section>
<section>
<h2>Try it</h2>
<label for="preview">Show animation</label>
<select id="preview"></select>
<button id="showpreview">Show for 10 s</button>
</section>
<section>
<h2>Device</h2>
<label class="check"><input type="checkbox" id="powersave"> Power save (80 MHz, Wi-Fi modem sleep)</label>
<label for="name">Name</label>
<input type="text" id="name" maxlength="24" spellcheck="false">
<p class="note">Hostname on the network, &lt;name&gt;.local, and the setup network &lt;name&gt;-setup. Applies after a restart.</p>
<button id="reboot">Restart</button>
<button id="forget" class="danger">Forget Wi-Fi</button>
</section>
<p id="log" role="status"></p>
</main>
<script>
/**
 * Returns the element carrying the given id.
 * @param {string} id - element id
 * @returns {HTMLElement} - the element, null when no element carries the id
 */
const el = id => document.getElementById(id);

/**
 * Writes one line into the status line below the form.
 * @param {string} text - message to show
 * @returns {void}
 */
const say = text => {
  el('log').textContent = text;
};

/**
 * Posts a JSON body and reports the outcome in the status line.
 * @param {string} path - endpoint path
 * @param {object} body - body to send, an empty object when omitted
 * @returns {Promise<object>} - parsed answer, null when the request failed
 */
async function post(path, body) {
  try {
    const response = await fetch(path, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(body || {})
    });
    const data = await response.json().catch(() => ({}));
    say(response.ok ? 'ok' : (data.error || 'HTTP ' + response.status));
    return response.ok ? data : null;
  } catch (error) {
    say('no answer');
    return null;
  }
}

/**
 * Formats a second count as seconds, minutes, hours or days.
 * @param {number} seconds - uptime in seconds
 * @returns {string} - shortest readable form
 */
function uptime(seconds) {
  if (seconds < 60) {
    return seconds + ' s';
  }
  const minutes = Math.floor(seconds / 60);
  if (minutes < 60) {
    return minutes + ' min ' + (seconds % 60) + ' s';
  }
  const hours = Math.floor(minutes / 60);
  if (hours < 24) {
    return hours + ' h ' + (minutes % 60) + ' min';
  }
  return Math.floor(hours / 24) + ' d ' + (hours % 24) + ' h';
}

let boardName = 'punkt';

/**
 * Shows the product name of the board in the title, the heading and the restart prompt.
 * @param {string|undefined} name - product name from the state body
 * @returns {void}
 */
function applyBoard(name) {
  if (!name) {
    return;
  }
  boardName = name;
  document.title = name;
  el('board-name').textContent = name;
}

/**
 * Reads the board status and fills the status list.
 * @returns {Promise<void>} - resolves once the list was written
 */
async function loadStatus() {
  try {
    const data = await (await fetch('/state')).json();
    applyBoard(data.board);
    el('v-state').textContent = data.state;
    el('v-uptime').textContent = uptime(data.uptimeS);
    el('v-ip').textContent = data.name + '.local ' + data.ip + ' (' + data.rssi + ' dBm)';
    el('v-version').textContent = data.version;
    el('v-view').textContent = data.view + ' (' + data.animation + ')'
      + (data.offReason === 'none' ? '' : ', ' + data.offReason);
    el('v-brightness').textContent = data.brightness + (data.dimmed ? ' (dimmed)' : '');
    el('v-context').textContent = data.contextAgeS === null
      ? 'none'
      : data.context + ' %' + (data.contextStale ? ' (stale)' : ', ' + data.contextAgeS + ' s ago');
    el('v-usage').textContent = data.usageAgeS === null
      ? 'none'
      : data.usage + ' %' + (data.usageStale ? ' (stale)' : ', ' + data.usageAgeS + ' s ago');
    const sessions = data.sessions || [];
    el('v-sessions').textContent = sessions.length
      ? sessions.map(s => (s.id ? s.id.slice(0, 8) : 'anon') + ' ' + s.state).join('\n')
      : 'none';
    const touch = data.touch;
    el('v-touch').textContent = (touch.touched ? 'down' : 'up') + ' ' + touch.raw + '/' + touch.baseline
      + ' L' + touch.proximity
      + (touch.lastEvent === 'none' ? '' : ', ' + touch.lastEvent + ' ' + touch.lastEventAgeS + ' s ago');
    el('v-temp').textContent = data.temperatureC === null ? '-' : data.temperatureC.toFixed(1) + ' C';
  } catch (error) {
    say('no answer');
  }
}

/** City table from GET /cities, filled once before the settings load. */
let cities = [];

/** Activities the board maps to an animation, each one has an anim- select of the same name. */
const activities = ['thinking', 'reading', 'editing', 'shell', 'delegating', 'mcp'];

/**
 * Reads the stored settings and fills every control with them.
 * @returns {Promise<void>} - resolves once the controls were written
 */
async function loadSettings() {
  try {
    const data = await (await fetch('/settings')).json();
    el('name').value = data.name;
    el('brightness').value = data.brightness;
    el('brightness-value').textContent = data.brightness;
    activities.forEach(activity => {
      el('anim-' + activity).value = data.animations[activity];
    });
    el('clock').value = data.clockTimeoutMin;
    el('autodim').checked = data.autoDim;
    el('nightbrightness').value = data.nightBrightness;
    el('nightbrightness-value').textContent = data.nightBrightness;
    el('dim').value = data.dimTimeoutMin;
    el('off').value = data.offTimeoutMin;
    el('city').value = data.city;
    showCityNote(data.city);
    const hasClockFace = data.clockFace !== undefined;
    el('clockface-label').hidden = !hasClockFace;
    el('clockface').hidden = !hasClockFace;
    if (hasClockFace) {
      el('clockface').value = data.clockFace;
    }
    const hasSplitClock = data.splitClock !== undefined;
    el('splitclock-label').hidden = !hasSplitClock;
    el('splitclock').hidden = !hasSplitClock;
    if (hasSplitClock) {
      el('splitclock').value = data.splitClock;
    }
    const hasBatterySleep = data.batterySleepMin !== undefined;
    el('batterysleep-label').hidden = !hasBatterySleep;
    el('batterysleep').hidden = !hasBatterySleep;
    if (hasBatterySleep) {
      el('batterysleep').value = data.batterySleepMin;
    }
    el('nightoff').checked = data.nightOff;
    el('nightsleep').checked = data.nightSleep;
    el('sleepat').value = data.sleepAt;
    el('wakeat').value = data.wakeAt;
    el('powersave').checked = data.powerSave;
    el('touch').value = data.touchTrigger;
    el('touch-value').textContent = data.touchTrigger + ' %';
    el('hover').value = data.touchHover;
    el('hover-value').textContent = data.touchHover + ' %';
    el('debugtouch').checked = data.debugTouch;
    syncDependents();
  } catch (error) {
    say('no answer');
  }
}

/**
 * Greys out the controls whose switch is off, so the page shows what applies.
 * @returns {void}
 */
function syncDependents() {
  const autoDim = el('autodim').checked;
  const nightOff = el('nightoff').checked;
  el('city-note').classList.toggle('dim', !autoDim);
  el('sleepat').disabled = !nightOff;
  el('wakeat').disabled = !nightOff;
  el('nightsleep').disabled = !nightOff;
}

/**
 * Posts a settings body and reloads the settings, which shows what the board kept.
 * @param {object} body - settings fields to store
 * @returns {Promise<void>} - resolves once the controls were reloaded
 */
async function saveBody(body) {
  await post('/settings', body);
  await loadSettings();
}

/**
 * Posts one settings key and reloads the settings, which shows what the board kept.
 * @param {string} key - settings key
 * @param {*} value - value to store under the key
 * @returns {Promise<void>} - resolves once the controls were reloaded
 */
function saveSetting(key, value) {
  return saveBody({ [key]: value });
}

/**
 * Binds the select of one activity, posting the animation it maps to whenever it changes.
 * @param {string} activity - activity name, the select carries it as its anim- id suffix
 * @returns {void}
 */
function bindAnimation(activity) {
  el('anim-' + activity).addEventListener('change', () => {
    saveBody({ animations: { [activity]: el('anim-' + activity).value } });
  });
}

/**
 * Binds a control to one settings key, posting its value whenever it changes.
 * @param {string} id - element id
 * @param {string} key - settings key
 * @param {function} read - returns the value to post from the element
 * @returns {void}
 */
function bindSetting(id, key, read) {
  el(id).addEventListener('change', () => {
    const value = read(el(id));
    if (value === null) {
      loadSettings();
      return;
    }
    saveSetting(key, value);
  });
}

/**
 * Reads a number field, an emptied field reads as null so nothing is posted.
 * @param {HTMLInputElement} input - number input
 * @returns {number} - the value, null while the field is empty
 */
const numberOf = input => input.value === '' ? null : Number(input.value);

/**
 * Binds a slider to one settings key, updating its value label live and posting debounced.
 * @param {string} id - element id, the value label is the same id with a -value suffix
 * @param {string} key - settings key
 * @param {string} unit - suffix appended to the value label
 * @returns {void}
 */
function bindSlider(id, key, unit) {
  let timer = 0;
  el(id).addEventListener('input', () => {
    el(id + '-value').textContent = el(id).value + unit;
    clearTimeout(timer);
    timer = setTimeout(() => saveSetting(key, Number(el(id).value)), 100);
  });
}

/**
 * Fills the city select from the board's table, one optgroup per region in table order.
 * @returns {Promise<void>} - resolves once the options were written
 */
async function loadCities() {
  try {
    cities = await (await fetch('/cities')).json();
  } catch (error) {
    say('no answer');
    return;
  }
  const select = el('city');
  select.replaceChildren();
  let group = null;
  cities.forEach(city => {
    if (!group || group.label !== city.region) {
      group = document.createElement('optgroup');
      group.label = city.region;
      select.appendChild(group);
    }
    const option = document.createElement('option');
    option.value = city.name;
    option.textContent = city.label;
    group.appendChild(option);
  });
}

/**
 * Fills the preview select from the animations the state machine owns.
 * @returns {Promise<void>} - resolves once the options were written
 */
async function loadPreview() {
  let animations = [];
  try {
    animations = (await (await fetch('/preview')).json()).animations;
  } catch (error) {
    say('no answer');
    return;
  }
  const select = el('preview');
  select.replaceChildren();
  animations.forEach(name => {
    const option = document.createElement('option');
    option.value = name;
    option.textContent = name;
    select.appendChild(option);
  });
}

/**
 * Writes the coordinates the sun position is computed for under the auto dim card.
 * @param {string} name - IANA name of the chosen city
 * @returns {void}
 */
function showCityNote(name) {
  const city = cities.find(entry => entry.name === name);
  if (!city) {
    el('city-note').textContent = '-';
    return;
  }
  const lat = Math.abs(city.latitude).toFixed(2) + (city.latitude < 0 ? ' S' : ' N');
  const lon = Math.abs(city.longitude).toFixed(2) + (city.longitude < 0 ? ' W' : ' E');
  el('city-note').textContent = 'Sun position for ' + city.label + ', ' + lat + ' ' + lon;
}

/**
 * Selects the city the browser reports and posts it, or says that the list has no entry for it.
 * @returns {Promise<void>} - resolves once the city was posted, at once when the list misses it
 */
function useDeviceCity() {
  const name = Intl.DateTimeFormat().resolvedOptions().timeZone;
  const match = Array.from(el('city').options).find(option => option.value === name);
  if (!match) {
    say('this browser reports ' + name + ', not in the list, pick the nearest city');
    return Promise.resolve();
  }
  match.selected = true;
  showCityNote(name);
  return saveSetting('city', name);
}

bindSlider('brightness', 'brightness', '');
activities.forEach(activity => bindAnimation(activity));
bindSetting('clock', 'clockTimeoutMin', numberOf);
bindSetting('autodim', 'autoDim', input => input.checked);
bindSlider('nightbrightness', 'nightBrightness', '');
bindSetting('dim', 'dimTimeoutMin', numberOf);
bindSetting('off', 'offTimeoutMin', numberOf);
bindSetting('city', 'city', input => input.value);
bindSetting('clockface', 'clockFace', input => input.value);
bindSetting('splitclock', 'splitClock', input => input.value);
bindSetting('batterysleep', 'batterySleepMin', numberOf);
bindSetting('nightoff', 'nightOff', input => input.checked);
bindSetting('nightsleep', 'nightSleep', input => input.checked);
bindSetting('sleepat', 'sleepAt', input => input.value === '' ? null : input.value);
bindSetting('wakeat', 'wakeAt', input => input.value === '' ? null : input.value);
bindSetting('powersave', 'powerSave', input => input.checked);
bindSlider('touch', 'touchTrigger', ' %');
bindSlider('hover', 'touchHover', ' %');
bindSetting('debugtouch', 'debugTouch', input => input.checked);
bindSetting('name', 'name', input => input.value);

el('city').addEventListener('change', () => {
  showCityNote(el('city').value);
});

['autodim', 'nightoff'].forEach(id => {
  el(id).addEventListener('change', syncDependents);
});

el('city-device').addEventListener('click', () => {
  useDeviceCity();
});

el('showpreview').addEventListener('click', async () => {
  await post('/preview', { animation: el('preview').value });
  loadStatus();
});

el('reboot').addEventListener('click', () => {
  if (confirm('Restart ' + boardName + '?')) {
    post('/reboot');
  }
});

el('forget').addEventListener('click', () => {
  if (confirm('Erase the Wi-Fi credentials and restart?')) {
    post('/forget-wifi');
  }
});

loadStatus();
loadCities().then(loadSettings);
loadPreview();
setInterval(loadStatus, 5000);
</script>
</body>
</html>
)HTML";
