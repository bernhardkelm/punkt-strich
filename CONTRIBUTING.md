# Contributing

Issues and pull requests are welcome. The firmware lives in `src/`, the Claude
Code side in `claude/`. The [README](README.md) covers what the board is and how
it is set up, [docs/hardware.md](docs/hardware.md) the parts and the wiring,
[docs/usage.md](docs/usage.md) the day to day use and
[docs/reference.md](docs/reference.md) the API, the settings and the make
targets. [docs/technical.md](docs/technical.md) describes how the firmware
behaves in depth, with the timings, the rules and the log lines behind every
picture on the matrix.

## Building and testing

`make doctor` checks that the tools every target needs are installed. `make
build` compiles the firmware, `make flash` uploads it over USB and
`make monitor` opens the serial console at 115200 baud. `make help` lists every
target. `make gifs` renders the pictures in `docs/animations/` and
`docs/strich/` again with the host harnesses in `tools/render/`, run it after
changing an animation or a strich clock face.
For the Claude Code side, `make hooks-diff` prints what an install
would change in `~/.claude/settings.json` without writing anything, and
`CLAUDE_HOME=<dir> claude/install.sh install` runs the installer against a copy
of the settings. `PUNKT_DRY_RUN=1` makes `claude/punkt.sh` print each request to
stderr instead of sending it:

```sh
echo '{"session_id":"abc","hook_event_name":"Stop"}' \
  | PUNKT_DRY_RUN=1 claude/punkt.sh hook
```

The CI workflow in `.github/workflows/build.yml` builds the `punkt` and
`strich` envs and runs `shellcheck -S warning` over the shell scripts on every
push and pull request. Run both locally before you open a pull request:

```sh
pio run -e punkt -e strich
shellcheck -S warning claude/*.sh tools/doctor.sh tools/render/render.sh
```

## Code conventions

- Braces on every body, also for a single statement and for an early return.
- A JSDoc-style block on every function, describing what it does, its
  parameters and its return value.
- Comments only for a real technical constraint the code cannot show, in
  English, as is everything else in the repository.

## Project layout

```
src/
  main.cpp                   boot, module wiring, loop
  version.h                  firmware version string
  nameTable.h                lookup and parse helpers for the enumerator name tables
  Animation.h                base class every animation implements
  BrightnessRamp.h/.cpp      linear ramps between the 16 hardware brightness levels
  Button.h/.cpp              debounced BOOT button with hold timing
  CoinInput.h/.cpp           strich's two coins: gestures under the overlay's ripple, the swipe disc
  coinActions.h/.cpp         what a strich coin gesture and a swipe do
  Commands.h/.cpp            state, settings and flag queues from the HTTP task to loop()
  Display.h/.cpp             thin wrapper around the Adafruit matrix driver, brightness and standby
  DisplayLayers.cpp          canvas, viewport, overlay and reveal layers the display composes
  Frame.h                    one bit per matrix pixel, shared by Display and the overlay
  inputActions.h/.cpp        what a button release and a punkt touch gesture do
  ResetHold.h/.cpp           10 s button or touch hold to the factory reset
  SerialCommands.h/.cpp      serial debug console
  StatusLed.h/.cpp           keeps the onboard WS2812 dark
  TapMemory.h                what the last tap did, so double tap or confirm can finish it
  app/
    CommandDrain.h/.cpp      applies the queued commands in loop(), restarts after the grace period
    SettingsApplier.h/.cpp   hands the stored settings to the modules loop() runs
    TouchDispatch.h/.cpp     ticks punkt's coin or strich's two coins, feeds the overlay and the reset hold
    otaProgress.h/.cpp       starts the OTA service with the callbacks that show the progress spiral
    patchSettings.h/.cpp     writes a validated settings patch, every setter persists to NVS
  animations/
    Attention.h/.cpp         rings blooming out of the centre, a flash and a dark pause
    Clock.h/.cpp             digital clock, hours over minutes as 3x4 digits
    CodeLines.h/.cpp         the code animation, lines typed from the top row down
    Compacting.h/.cpp        rings collapsing from the border into the centre
    Done.h/.cpp              rings bloom, the matrix pulses and drains row by row
    Error.h/.cpp             skull with three short glitch bursts per loop
    Glitch.h/.cpp            glitch burst then a dim X held at half brightness
    IdleGauge.h/.cpp         dark viewport with only the context gauge, idle on strich
    Knight.h/.cpp            the knight animation, a 2x2 block sweeping with a trail
    Life.h/.cpp              the life animation, Conway's Game of Life on a wrapping grid
    Limit.h/.cpp             hourglass with sand drifting into the bottom half
    MatrixRain.h/.cpp        the rain animation, one falling streak per column
    Off.h/.cpp               dark matrix with a dim heartbeat pixel in the corner
    Orbit.h/.cpp             comet running around the border, loading and the orbit animation
    OtaSpiral.h/.cpp         inward spiral fill with the share of an OTA image received
    Pipes.h/.cpp             the pipes animation, a pipe keeping a dark pixel to every other run and crossing them
    Pointer.h/.cpp           the pointer animation, a three pixel L gliding to a spot, clicking with a ripple or dragging a rectangle
    Pong.h/.cpp              the pong animation, two tracking paddles and a ball between them
    ResetFill.h/.cpp         bottom-up fill while the factory reset hold runs
    Scan.h/.cpp              the scan animation, a cursor reading a page of code line by line
    Terminal.h/.cpp          the terminal animation, a shell session typing and scrolling up out of the bottom row
    Tetris.h/.cpp            the tetris animation, falling blocks that stack and collapse
    Unread.h/.cpp            dark matrix with a softly blinking corner marker
    UsageView.h/.cpp         the 5 hour usage limit as a gauge filling the matrix
    Waiting.h/.cpp           single pixel drifting along a Lissajous-like path
    WifiLost.h/.cpp          Wi-Fi symbol blinking while the link is down
    WifiSearch.h/.cpp        Wi-Fi symbol whose arcs light up one after another, the setup portal
    blink.h/.cpp             period based blink helpers for the markers and gauges
    bottomFill.h/.cpp        bottom-up, left-to-right fill helper
    contextGauge.h/.cpp      bottom-row context gauge and the rows left for the animation
    cursorTarget.h/.cpp      random targets and drag corners the pointer L fits on, one step towards them
    digitFont.h/.cpp         3x4 digit glyphs
    glitchFrame.h/.cpp       the thick X rows and the glitch treatment applied to any frame
    linePlan.h/.cpp          the code line planner shared by CodeLines and Scan
    marker.h/.cpp            blinking corner marker over a dark matrix
    rectOutline.h/.cpp       rectangle border between two opposite corners
    rings.h/.cpp             centred square ring outlines
    ripple.h/.cpp            circular ring of one radius, the click ripple
    skullFrame.h/.cpp        the skull rows, the error face
    spiralFill.h/.cpp        inward spiral fill helper, the 64 positions in fill order
    wifiSymbol.h/.cpp        the Wi-Fi dot and its quarter arcs
  board/
    boardConfig.h            includes the header of the board the env builds
    punkt.h                  pins, canvas and setting defaults of the 8x8 punkt
    strich.h                 pins, canvas and setting defaults of the 16x8 strich
  clockFaces/
    BatteryFace.h/.cpp       battery view of strich: outline, fill and percentage, a bolt on USB
    CanvasWipe.h/.cpp        column wipe into and out of the battery view
    LayoutSlide.h/.cpp       steps between the full clock face and a split picture
    LowBatteryDot.h/.cpp     the blinking low-battery dot in column 7 of the bottom row
    SideClock.h/.cpp         split clock in the left half, in the splitClock style
    WideClock.h/.cpp         full-width clock across the 16x8 canvas, staircase or tall face
    batteryDigits.h/.cpp     small percentage digits of the battery view
    batteryFill.h/.cpp       battery inside lit from the left in proportion to the charge
    batteryOutline.h/.cpp    battery outline across the canvas
    canvasFrame.h/.cpp       draws a whole frame onto the canvas or the viewport
    chargeBolt.h/.cpp        lightning bolt the battery view shows on USB
    clockFrame.h/.cpp        full face frames by digit height and minute shift, the squash and the slide
    columnRun.h              row bits of a run of adjacent columns
    digitPairs.h/.cpp        3x4 and 3x3 digit pairs as frames
    frameUnion.h             pixels lit in either of two frames
    glyphFrame.h/.cpp        one glyph placed into a frame
    minuteBar.h/.cpp         minute bar of the bar split clock
    minuteSquares.h/.cpp     two counting squares of the squares split clock
    scaledDigit.h/.cpp       3-column digit stretched to a height, the tall face
    smallDigit.h/.cpp        3x3 digit glyphs
    splitClockFrame.h/.cpp   the split clock picture of each style
    splitPath.h/.cpp         slide path from the full face into each split clock style
    squaresRoll.h/.cpp       squares minute change, draining one pixel per step
  settings/
    Settings.h/.cpp          NVS backed settings
    cities.h/.cpp            the city table: IANA name, label, region, POSIX rule, coordinates
    workingAnimationNames.h/.cpp  working animation names, same order as the enum
    activityNames.h/.cpp     activity names, same order as the enum
    clockFaceNames.h/.cpp    clock face names, strich
    splitClockNames.h/.cpp   split clock style names, strich
    clockMinutes.h/.cpp      HH:MM parse and format
  state/
    State.h                  PunktState, View and the StateMachine declaration
    State.cpp                the tick, the brightness ceiling and the activity time
    StateBattery.cpp         battery view timeout and wipe, the low-battery flag
    StateInput.cpp           what the touch and the button feed in: acknowledge, idle, activity
    StateOff.cpp             off view, off reasons, the off timeout
    StatePicture.cpp         which picture wins: link and OTA pictures, preview, view, clock face
    StatePreview.cpp         animation preview and the animation name lookup
    StateReports.cpp         the context and usage percentages with their age
    StateSessions.cpp        state requests from the API and the serial console, the dismiss
    StateShow.cpp            starts the chosen picture, the side clock and the unread marker
    StateSlide.cpp           the layout slide between the full face and a split picture
    StateView.cpp            view changes, the no-session rule, the clock timeout
    ReportedPercent.h        a percentage with the time it arrived, so it can go stale
    SessionTable.h/.cpp      up to 8 sessions and their watchdog timestamps
    lowBattery.h/.cpp        low-battery hysteresis, below 15 percent on, from 18 off
    statePriority.h/.cpp     which session is shown, and which states count as busy
    stateAnimations.cpp      the animation each state and working animation maps to
    stateNames.h/.cpp        state names
  network/
    WifiSetup.h/.cpp         WiFiManager wrapper, the setup portal, the link watchdog and state
    BootCounter.h/.cpp       reset reason and the fast power cycles that request a factory reset
    Ota.h/.cpp               ArduinoOTA wrapper
    TimeSync.h/.cpp          SNTP wrapper, applies the POSIX rule of the chosen city
  power/
    Power.h/.cpp             CPU clock, Wi-Fi sleep mode, chip temperature
    BatterySleep.h/.cpp      deep sleep on battery, the left coin and the 30 s USB check wake
    NightSleep.h/.cpp        deep sleep inside the night window, double tap wake
    NightOff.h/.cpp          scheduled screen off inside the night window
    VbusSense.h/.cpp         USB presence and the smoothed charge estimate from the VBUS pin
    batteryEstimate.h/.cpp   VBUS voltage to charge along the curve measured on one cell
    deepSleep.h/.cpp         shared sleep entry: dark matrix, one touch pad scanned, the kind in RTC memory
    nightWindow.h/.cpp       pure night window check
    wakeDelay.h/.cpp         seconds from a local time to the next wake time
  dimming/
    AutoDim.h/.cpp           sun elevation to brightness ceiling, the 30 s tick
    BrightnessCeiling.h/.cpp  hands the state machine the ceiling from the sun level and the rest flag
    RestDim.h/.cpp           resting clock dim flag
    restCeiling.h/.cpp       pure min rule for the ceiling
    solarElevation.h/.cpp    NOAA solar elevation for a place and a moment
  touch/
    Touch.h/.cpp             capacitive electrode on GPIO4, its reading and hover level
    TouchCalibration.cpp     baseline calibration and the frozen sensor restart
    TouchDrift.cpp           baseline drift, the snap-down and the stuck touch release
    TouchGesture.cpp         debounce and the tap, double tap and hold recognition
    TouchBaseline.h/.cpp     untouched reference level and its drift tracking
    TouchEvent.h             the gestures the electrode reports
    TouchProximity.h/.cpp    hover level EMA, thresholds and hysteresis on the raw reading
    touchEventNames.h/.cpp   touch event names
    Coin.h                   the two coins of strich
    coinNames.h/.cpp         coin names for the log
    CoinSignal.h/.cpp        one coin's smoothed delta, drifting baseline, strength and dome level
    ChordHold.h/.cpp         two-finger hold over both coins
    FingerTracker.h/.cpp     one finger over strich's two coins, its phases and gestures
    FingerTrackerGesture.cpp  press, swipe and release handling of the finger tracker
    FingerSensor.h/.cpp      samples both coins into strengths and the filtered finger column
    FingerColumn.h/.cpp      position estimate, slew limit and One Euro filter
    FingerPress.h/.cpp       the metrics and the tap, hold and swipe rules of one press
    FingerEvent.h            the gestures the finger tracker reports
    FingerEventQueue.h/.cpp  hands out the gestures of one sample one per call
    GestureLog.h/.cpp        debug gesture lines, repeated while idle
    GestureSettle.h/.cpp     quiet period after a released gesture
    OneEuroFilter.h/.cpp     speed adaptive low pass
    SwipeDisc.h/.cpp         radius of the disc a swipe grows out of its coin's dome
    TapWindow.h/.cpp         double tap windows of both coins
    domeLevels.h/.cpp        dome level and radius from a coin's strength
    fingerPosition.h/.cpp    coin centres, the noise floor and the position model
  overlay/
    TouchOverlay.h/.cpp      draws the touch proximity domes, one per coin
    TouchOverlayReveal.cpp   the ripple and the swipe disc that reveal the next picture
    discMask.h/.cpp          disc, ring and keyline pixel masks for the overlay
  web/
    HttpServer.h/.cpp        async web server on port 80 and the route registration
    apiCommon.h/.cpp         shared JSON body limits, API logging and error answers
    stateRoutes.h/.cpp       GET and POST /state
    settingsRoutes.h/.cpp    GET and POST /settings
    settingsDoc.h/.cpp       the full settings object, overlaid with a pending patch
    settingsFields.h/.cpp    readers of the settings fields that need more than jsonFields
    settingsPatch.h/.cpp     validates a POST /settings body into a patch
    patchDetail.h/.cpp       the fields of a patch for the log line
    percentRoutes.h/.cpp     POST /context and POST /usage
    viewRoutes.h/.cpp        POST /view
    previewRoutes.h/.cpp     GET and POST /preview
    cityRoutes.h/.cpp        GET /cities
    statusBody.h/.cpp        the GET /state body
    jsonFields.h/.cpp        field readers and the messages they reject with
    settingsPage.h           the settings page markup
claude/
  punkt.sh                   hook mapping script, installed to ~/.claude/hooks
  punkt-statusline.sh        status line wrapper feeding the context gauge and usage view
  hooks.json                 hook registrations merged into ~/.claude/settings.json
  install.sh                 behind make hooks-install, hooks-uninstall, hooks-diff
docs/
  hardware.md                parts, wiring, coins, battery, case and assembly
  usage.md                   what the matrix shows and how to use the board
  reference.md               HTTP API, settings, event mapping, serial commands, make targets
  technical.md               the firmware behaviour in depth
  logo.svg, logo-dark.svg    the project logo, light and dark
  blueprint/                 the README header views and the social preview, written by make blueprint
  animations/                one GIF and one still PNG per punkt picture, written by make gifs
  strich/                    the strich pictures, written by make gifs
  photos/                    photos of the built devices, metadata stripped
  screenshots/               settings page section pictures used by the docs
  wiring/                    wiring diagrams of punkt and strich, light and dark
hardware/
  punkt_case.3mf             punkt case and screen plate, 3MF in mm
  strich_case.3mf            strich case and screen plate, 3MF in mm
  esp_solder_jig.3mf         optional jig that holds the Super Mini while soldering
tools/
  doctor.sh                  checks the local prerequisites, behind make doctor
  blueprint/                 projects the hardware/ models into the header views, behind make blueprint
  render/
    main.cpp                 punkt host harness, runs one picture and prints its frames
    render.sh                builds both harnesses and writes docs/animations and docs/strich, behind make gifs
    gif.py                   turns a frame dump into the animated GIF and the still PNG
    strich/
      main.cpp               strich host harness, drives one picture and prints its 16x8 frames
      SplitPicture.h/.cpp    the split clock beside a Claude picture, run like the state machine runs it
      SlideRoundTrip.h/.cpp  the layout slide from the full face into the split picture and back
      hostDisplay.cpp        host definitions of the Display members, a push prints the canvas
      hostServices.h/.cpp    fixed time, settings and VBUS state the clock faces read
    stub/
      Adafruit_LEDBackpack.h  one HT16K33 backpack, dumps its pushes instead of sending them
      Animation.h            copy of src/Animation.h, shadows the include on the host
      Arduino.h              the millis() and random() the animations call
      Display.h              host matrix, records every frame it is shown
      Preferences.h          in-memory stand-in for the NVS store
      network/
        TimeSync.h           one fixed local time, so the clock picture is stable
.github/workflows/
  build.yml                  CI: builds the punkt and strich envs, runs shellcheck on the scripts
.vscode/extensions.json      recommends the PlatformIO IDE extension
Makefile                     every build, flash, board and hooks target, make help lists them
platformio.ini               the punkt and strich envs with their -ota and -jtag variants
```

`Display` is the only code that includes the Adafruit library. Animations draw
through its five calls: `clear()`, `setPixel(x, y, on)`, `show()`,
`setBrightness(0-15)` and `setBrightnessNow()`.

`WifiSetup.cpp` is the only file including `WiFiManager.h`, and only the
`.cpp` files in `src/web/` include `ESPAsyncWebServer.h`, never a header. The
two libraries define clashing `HTTP_*` enumerators and must stay in separate
translation units.

HTTP handlers run on the AsyncTCP task, not in `loop()`. They only read
snapshots and enqueue work in `Commands`, which `loop()` drains once per
iteration. A settings change travels the same way: the handler validates it
and queues a patch, `loop()` applies it and does the NVS writes. The session
table is one such snapshot:
`SessionTable::copyTo()` and every write that moves a whole entry run under a
spinlock, so a reader never catches an entry half moved.

## Adding an animation

1. Create `src/animations/Sparkle.h` and `Sparkle.cpp` with a class deriving
   from `Animation`:

```cpp
#pragma once

#include "Animation.h"

class Sparkle : public Animation {
public:
  const char* name() const override;
  void begin(Display& display) override;
  void tick(Display& display, uint32_t nowMs) override;
};
```

2. Implement the three methods. `begin()` receives a cleared matrix at the
   brightness ceiling in effect and must draw the first frame. `tick()` is called from
   `loop()` as fast as possible, so keep your own timestamps and only call
   `show()` when the frame actually changed. Never call `delay()`. Scale
   brightness against `maxBrightness()`, it can change while the animation
   runs. Override `finished()` if the animation plays a burst once and then
   holds an end frame, as `Glitch` does: `true` is what falls `done` back to
   `idle` and what lets a one-shot preview hand the matrix back early, the
   animation keeps drawing the held frame.

3. Hook it up in one of two ways:

   - **As a state animation**: add the member to `StateMachine` in
     `src/state/State.h`, add it to the `all_` list, raise `kAnimationCount`
     by one (a list that does not match it fails to compile) and return it
     from `StateMachine::animationFor()` in
     `src/state/stateAnimations.cpp`. A new state additionally needs an
     enumerator in `PunktState`, its name in `src/state/stateNames.cpp`, a
     case in `statePriority()` and, unless the state should never time out on
     its own, a case in `isBusyState()`, both in
     `src/state/statePriority.cpp`.
   - **As a working animation**: add the enumerator to `WorkingAnimation` in
     `src/settings/Settings.h`, bump `kWorkingAnimationCount` (the BOOT
     button cycles through exactly that many, see `handleButtonRelease()` in
     `src/inputActions.cpp`), its name in `src/settings/workingAnimationNames.cpp`
     (same order as the enum), the member and the
     `StateMachine::workingAnimation()` case in `src/state/State.h` and
     `src/state/stateAnimations.cpp`, an `<option>` in every `anim-` select of
     `src/web/settingsPage.h` and the name in `kWorkingAnimationError` in
     `src/web/jsonFields.h`, the one message both the `animations` object
     on `POST /settings` and the `animation` field on `POST /state` answer
     with. The `ANIMATIONS` list in the `Makefile` names them for
     `make working A=<name>`.

4. Give it a picture for the docs: add a branch for its name to
   `makePicture()` in `tools/render/main.cpp` and a row to the `PICTURES` list
   in `tools/render/render.sh` (name, duration, seed, percent, window start,
   budget end, still time and loop mode). `make gifs` then writes
   `docs/animations/<name>.gif` and `<name>.png`.

The string returned by `name()` is what shows up in the logs.

## Hook script internals

The Claude Code integration lives in `claude/` and is managed from this
repository. Edit the files there and install again, never the copies under
`~/.claude/hooks/`. The command line forms of both scripts are in
[The hook scripts](docs/reference.md#the-hook-scripts).

### The installer

`hooks-install` copies `claude/punkt.sh` and `claude/punkt-statusline.sh` to
`~/.claude/hooks/`, keeps an existing `~/.claude/settings.json` as
`settings.json.bak` unless it already holds punkt hooks, and
merges `claude/hooks.json` into it with `jq`: every hook whose command runs a
punkt script is removed first, then the entries from `hooks.json` are appended.
Other hooks and settings stay untouched and a second run changes nothing.
`statusLine` is pointed at the wrapper when it is unset, already runs a punkt
script or runs `~/.claude/statusline-command.sh` directly, any other status line
is left alone with a note. The `statusLine` from before the first install is
recorded in `~/.claude/hooks/punkt-statusline.prev.json`, as `"none"` when there
was none. `hooks-uninstall` removes the same entries, puts exactly that recorded
value back, or drops `statusLine` again when the record says `none`, and deletes
the record and the copied scripts. Without a record it falls back to pointing
`statusLine` at `statusline-command.sh` when that script exists. An unreadable
`settings.json` aborts before anything is written.

Claude Code's file watcher normally picks up the changed hooks, running
sessions included. `CLAUDE_HOME`, or `CLAUDE_CONFIG_DIR`, points the installer
at another directory, for a dry run against a copy of the settings, and
`PUNKT_HOST` writes the board host into every installed command instead of
leaving it on the `punkt.local` default. The hooks targets pass both on, see
[Several boards and Claude
directories](docs/usage.md#several-boards-and-claude-directories). A leading `~` in
the directory is expanded by the installer as a safety net.

### `punkt.sh`

Every event in `hooks.json` runs `punkt.sh hook` without a matcher. The script
reads `session_id`, `hook_event_name`, `tool_name`, `notification_type`,
`error`, `trigger`, `source` and `agent_id` from the payload in one `jq` call
and picks the state itself, so the whole mapping sits in one `case` block and
two overlapping matchers can never post conflicting states for the same event.
Events that map to nothing post nothing. `context` reads
`context_window.used_percentage` and `rate_limits.five_hour.used_percentage`,
rounds each, clamps it to 0-100 and posts them to `/context` and `/usage`. A
field the payload lacks, such as the usage on an API key account, posts
nothing.

The script never writes to stdout and always exits `0`. Every `curl`
(`-4 -s -m 5`) runs backgrounded and detached, so an unreachable board never
delays Claude Code. Without `jq` or with empty stdin nothing is posted.

Every state post carries `ts`, the local clock in milliseconds sampled as the
script's first step (`perl` with `Time::HiRes`, whole seconds from `date` as
the fallback). Each hook posts from its own background process, so two events
a few milliseconds apart, such as `PreToolUse` and `PermissionRequest`, can
reach the board in either order. The board drops the one that was overtaken,
see [`POST /state`](docs/reference.md#post-state).

Every `PreToolUse` posts, whatever the tool. The watchdog counts only the
requests of that session, so a long stretch of reads, searches or MCP calls
would otherwise let it fall back to `idle` in the middle of a turn, see
[Sessions and the watchdog](docs/technical.md#sessions-and-the-watchdog). A tool
with no activity of its own repeats the activity the session sent last, kept in
`$TMPDIR/punkt/<session>.activity` next to the permission marker and removed
with it on `UserPromptSubmit`, `Stop`, `StopFailure` and `SessionEnd`:
alternating between an activity and none would restart the running working
animation on every tool switch. A turn whose tools carried no activity yet
posts `working` without one, which the board works in as `thinking`.
`PostToolUse` posts only for the tools listed under
[Event mapping](docs/reference.md#event-mapping) and after a permission dialog, one
post per tool call is enough to keep the watchdog quiet.

`PermissionRequest` fires the moment Claude Code asks for permission, the
`permission_prompt` notification only once the dialog has waited about six
seconds; it stays mapped because it is the only signal for a sandboxed
command's network request. `PermissionRequest` is registered without `async`
and with a 5 s timeout, since it is a decision event. The script returns no
decision, so the dialog appears as usual.

`idle_prompt` fires once Claude finished about a minute ago and nothing was
typed since. In the `normal` view the clock that followed `done` then gives
way to the `unread` marker until the next prompt.

Background helper agents keep working after the main turn ended, so `Stop`
defers its `done` while one of them runs: `SubagentStart` writes an empty file
named after the payload's `agent_id` to `$TMPDIR/punkt/<session>.agents/`, and
`SubagentStop` removes it again. Whether a turn is running at all is
`$TMPDIR/punkt/<session>.turn`, written by `UserPromptSubmit` and removed by
`Stop`, `StopFailure` and `SessionEnd`: only an agent that starts or stops
inside a turn posts a state at all. When `Stop` finds a file younger than 30
minutes in the agents directory it posts `working` with the activity
`delegating` instead and leaves `$TMPDIR/punkt/<session>.deferred` behind; the
`SubagentStop` that removes the last agent file then posts the held back
`done`. Older files belong to an agent whose stop never fired and are swept
away, and `UserPromptSubmit`, `StopFailure` and `SessionEnd` drop the deferred
marker again, since the regular flow takes over from there.

Answering a permission dialog fires no event of its own, so the attention
posts leave a marker, the event's `ts` in `$TMPDIR/punkt/<session>.asked`. An
approved call ends in `PostToolUse`, which consumes a marker older than itself
and posts `working`. A denied call is followed by the next tool call, whose
`PreToolUse` consumes a marker older than 1 s; a younger one belongs to the
very call the dialog is about. `UserPromptSubmit`, `PostToolUseFailure`,
`Stop`, `StopFailure` and `SessionEnd` remove the marker.

The board takes care of the rest on its own: `done` falls back to `idle` after
its burst, a `glitch` returns to the state it interrupted 2.1 s after it was
posted, and the watchdog falls back to `idle` and then `off` when no hook
fires for a while.

### `punkt-statusline.sh`

Reads the status line payload once, feeds it to `punkt.sh context`, which
posts the context fill and the usage, and then to
`~/.claude/statusline-command.sh` when that script exists, passing its output
and exit code through unchanged.
