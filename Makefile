.DEFAULT_GOAL := help

BOARD       ?= punkt
BOARDS      := punkt strich
ifeq ($(filter $(BOARD),$(BOARDS)),)
$(error BOARD must be one of: $(BOARDS))
endif

HOST        ?= $(BOARD).local
URL         := http://$(HOST)
CURL        := curl -4 -sS -m 3
JSON        := -H 'Content-Type: application/json'
STATES      := off loading idle working attention done error waiting compacting limit unread glitch
ACTIVITIES  := thinking reading editing shell delegating mcp
ANIMATIONS  := tetris life knight orbit code rain scan pipes pointer pong terminal
SETTINGS    := brightness autoDim nightBrightness city dimTimeoutMin offTimeoutMin clockTimeoutMin nightSleep sleepAt nightOff clockFace splitClock batterySleepMin wakeAt powerSave touchTrigger touchHover debugTouch name
GAUGES      := context usage
comma       := ,
space       := $(subst ,, )

PORT_UPLOAD := $(if $(PORT),--upload-port $(PORT))

# only an explicit HOST pins the hooks to a board, the default must not be baked into settings.json
ifneq ($(filter command line environment,$(origin HOST)),)
HOOKS_ENV := PUNKT_HOST=$(HOST)
endif

.PHONY: help doctor build flash flash-jtag ota monitor gifs blueprint \
        status settings set gauge view preview name reboot forget-wifi \
        hooks-install hooks-uninstall hooks-diff $(STATES)

help: ## list targets
	@grep -E '^[a-zA-Z_-]+:.*## ' $(MAKEFILE_LIST) | awk -F ':.*## ' '{printf "  %-14s %s\n", $$1, $$2}'
	@printf "  %-14s %s\n" "<state>" "set a state: $(STATES)"
	@printf "\nHOST=%s  (override: make status HOST=192.168.1.10)\n" "$(HOST)"
	@printf "BOARD=<%s>  (default punkt, selects the build env and the default HOST, e.g. make flash BOARD=strich)\n" "$(subst $(space),|,$(BOARDS))"
	@printf "PORT=<serial port>  (optional on flash, flash-jtag and monitor, e.g. make flash PORT=/dev/cu.usbmodem1101 or /dev/ttyACM0)\n"
	@printf "S=<session> ACT=<activity> A=<animation>  (optional on a state target, e.g. make working ACT=editing)\n"
	@printf "ACT=%s\n" "$(ACTIVITIES)"
	@printf "A=%s  (overrides the animations setting, e.g. make working A=tetris)\n" "$(ANIMATIONS)"
	@printf "K=%s  (make set K=<setting> V=<value>)\n" "$(SETTINGS)"
	@printf "HOST=<name>.local CLAUDE_HOME=<dir>  (optional on the hooks targets, HOST may list several boards separated by commas, e.g. HOST=punkt.local,strich.local, make hooks-install HOST=punkt-work.local CLAUDE_HOME=\$$HOME/.claude-work)\n"

doctor: ## check the local prerequisites, the board and the installed hooks
	@HOST=$(HOST) tools/doctor.sh

build: ## compile the firmware
	pio run -e $(BOARD)

flash: ## flash over USB, optional PORT=<serial port>
	pio run -e $(BOARD) -t upload $(PORT_UPLOAD)

flash-jtag: ## flash over the built-in USB JTAG with OpenOCD (for USB serial paths that drop bytes), then reset
	pio run -e $(BOARD)-jtag -t upload
	@port="$(PORT)"; \
	test -n "$$port" || port=$$(ls /dev/cu.usbmodem* /dev/ttyACM* 2>/dev/null | head -1); \
	test -n "$$port" || { echo "no serial port found, tap RESET by hand or pass PORT=<port>"; exit 0; }; \
	pio pkg exec -p tool-esptoolpy -- esptool.py --chip esp32s3 --port "$$port" --before default_reset --after hard_reset run

ota: ## flash over Wi-Fi via ArduinoOTA (resolves $(HOST) to IPv4 first, espota cannot)
	@ip=""; \
	if printf '%s' '$(HOST)' | grep -Eq '^([0-9]{1,3}\.){3}[0-9]{1,3}$$'; then \
	  ip='$(HOST)'; \
	elif command -v dscacheutil >/dev/null 2>&1; then \
	  ip=$$(dscacheutil -q host -a name $(HOST) | awk '/ip_address/ {print $$2; exit}'); \
	elif command -v getent >/dev/null 2>&1; then \
	  ip=$$(getent ahostsv4 $(HOST) | awk '{print $$1; exit}'); \
	  test -n "$$ip" || ip=$$(getent hosts $(HOST) | awk '$$1 ~ /^[0-9.]+$$/ {print $$1; exit}'); \
	fi; \
	if [ -z "$$ip" ] && command -v avahi-resolve >/dev/null 2>&1; then \
	  ip=$$(avahi-resolve -4 -n $(HOST) 2>/dev/null | awk '{print $$2; exit}'); \
	fi; \
	test -n "$$ip" || { echo "cannot resolve $(HOST), pass HOST=<ip>"; exit 1; }; \
	echo "OTA to $(HOST) ($$ip)"; \
	pio run -e $(BOARD)-ota -t upload --upload-port "$$ip"

monitor: ## open the serial monitor at 115200 baud, optional PORT=<serial port>
	pio device monitor $(if $(PORT),--port $(PORT))

gifs: ## render the pictures in docs/animations (punkt) and docs/strich with tools/render
	tools/render/render.sh

blueprint: ## draw the README header in docs/blueprint from the case models in hardware
	python3 -I -B tools/blueprint/blueprint.py

status: ## GET /state
	@$(CURL) $(URL)/state; echo

settings: ## GET /settings
	@$(CURL) $(URL)/settings; echo

set: ## change one setting: make set K=brightness V=8, K=sleepAt V=22:00, K=powerSave V=false, K=city V=Europe/Berlin
	@test -n "$(K)" -a -n "$(V)" || { echo "usage: make set K=<setting> V=<value>"; exit 1; }
	@test -n "$(filter $(K),$(SETTINGS))" || { echo "K must be one of: $(SETTINGS)"; exit 1; }
	@case "$(V)" in \
	  true|false) value='$(V)' ;; \
	  *) if printf '%s' '$(V)' | grep -Eq '^-?[0-9]+(\.[0-9]+)?$$'; then value='$(V)'; else value='"$(V)"'; fi ;; \
	esac; \
	$(CURL) -X POST $(URL)/settings $(JSON) -d "{\"$(K)\":$$value}"; echo

gauge: ## set a gauge 0-100: make gauge G=context|usage P=42
	@test -n "$(G)" -a -n "$(P)" || { echo "usage: make gauge G=<context|usage> P=<0-100>"; exit 1; }
	@test -n "$(filter $(G),$(GAUGES))" || { echo "G must be context or usage"; exit 1; }
	@$(CURL) -X POST $(URL)/$(G) $(JSON) -d '{"percent":$(P)}'; echo

view: ## switch the display view: make view V=clock|normal|usage|battery|off|toggle, battery on strich
	@test -n "$(V)" || { echo "usage: make view V=<clock|normal|usage|battery|off|toggle>"; exit 1; }
	@$(CURL) -X POST $(URL)/view $(JSON) -d '{"view":"$(V)"}'; echo

preview: ## play an animation for 10 s: make preview A=<animation>
	@test -n "$(A)" || { echo "usage: make preview A=<animation>"; exit 1; }
	@$(CURL) -X POST $(URL)/preview $(JSON) -d '{"animation":"$(A)"}'; echo

name: ## name the board and restart it so it answers as <name>.local: make name N=punkt-work
	@test -n "$(N)" || { echo "usage: make name N=<name>"; exit 1; }
	@$(CURL) -X POST $(URL)/settings $(JSON) -d '{"name":"$(N)"}'; echo
	@$(CURL) -X POST $(URL)/reboot; echo

reboot: ## POST /reboot
	@$(CURL) -X POST $(URL)/reboot; echo

forget-wifi: ## erase Wi-Fi credentials and restart into the setup portal
	@$(CURL) -X POST $(URL)/forget-wifi; echo

hooks-install: ## install the Claude Code hooks from claude/ into the Claude config directory
	@$(HOOKS_ENV) claude/install.sh install

hooks-uninstall: ## remove the punkt hooks and scripts from the Claude config directory
	@$(HOOKS_ENV) claude/install.sh uninstall

hooks-diff: ## show what hooks-install would change in the Claude config directory
	@$(HOOKS_ENV) claude/install.sh diff

$(STATES): ## set a state, e.g. make working (optional S=<session> ACT=<activity> A=<animation>)
	@$(CURL) -X POST $(URL)/state $(JSON) -d '{"state":"$@"$(if $(S),$(comma)"session":"$(S)")$(if $(ACT),$(comma)"activity":"$(ACT)")$(if $(A),$(comma)"animation":"$(A)")}'; echo
