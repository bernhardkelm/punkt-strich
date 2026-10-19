#!/bin/bash
# Checks the local prerequisites of the Makefile targets and reports what is missing.
# Usage: doctor.sh            run from the repository root, behind make doctor
# Environment: HOST (default punkt.local) is the board to probe, CLAUDE_HOME or CLAUDE_CONFIG_DIR the Claude directory
set -u

host="${HOST:-punkt.local}"
claude_home="${CLAUDE_HOME:-${CLAUDE_CONFIG_DIR:-$HOME/.claude}}"
missing=0

ok() {
  printf '  ok       %-12s %s\n' "$1" "$2"
}

warn() {
  printf '  missing  %-12s %s\n' "$1" "$2"
}

fail() {
  printf '  MISSING  %-12s %s\n' "$1" "$2"
  missing=$((missing + 1))
}

# $1 command, $2 what needs it, $3 required|optional, $4 install hint
check_command() {
  local path

  if path=$(command -v "$1" 2>/dev/null); then
    ok "$1" "$path"
    return
  fi

  if [ "$3" = "required" ]; then
    fail "$1" "$2, $4"
  else
    warn "$1" "$2, $4"
  fi
}

echo "Firmware"
check_command pio "build, flash, ota and monitor" required "install PlatformIO Core: https://docs.platformio.org/en/latest/core/"
check_command "${CXX:-c++}" "make gifs" optional "install the Xcode command line tools or a C++ compiler"

if command -v python3 >/dev/null 2>&1; then
  if python3 -c 'import PIL' 2>/dev/null; then
    ok "python3" "$(command -v python3), Pillow present"
  else
    warn "Pillow" "make gifs, pip3 install pillow"
  fi
else
  warn "python3" "make gifs, install Python 3 with Pillow"
fi

if command -v dscacheutil >/dev/null 2>&1 || command -v getent >/dev/null 2>&1 || command -v avahi-resolve >/dev/null 2>&1; then
  ok "resolver" "make ota can resolve $host"
else
  warn "resolver" "make ota needs dscacheutil, getent or avahi-resolve, else pass HOST=<ip>"
fi

if [ "$(uname)" = "Linux" ]; then
  if command -v avahi-daemon >/dev/null 2>&1 || [ -e /usr/lib/libnss_mdns4_minimal.so.2 ] || [ -e /lib/x86_64-linux-gnu/libnss_mdns4_minimal.so.2 ]; then
    ok "mdns" "avahi or nss-mdns present, .local names can resolve"
  else
    warn "mdns" "install avahi-daemon and libnss-mdns for .local names, or pass HOST=<ip>"
  fi
fi

ports=$(ls /dev/cu.usbmodem* /dev/ttyACM* 2>/dev/null | tr '\n' ' ')

if [ -n "$ports" ]; then
  ok "usb" "$ports"
else
  warn "usb" "no /dev/cu.usbmodem* or /dev/ttyACM* port, plug the board in for make flash and make monitor"
fi

echo "Board and API"
check_command curl "every target that talks to the board and the hook scripts" required "install curl"
check_command jq "the hook scripts and the installer" required "install it with your package manager, e.g. brew install jq or apt install jq"

if command -v curl >/dev/null 2>&1; then
  IFS=',' read -r -a probe_hosts <<<"$host"

  for probe in "${probe_hosts[@]}"; do
    state=$(curl -4 -s -m 3 "http://$probe/state" 2>/dev/null)

    if [ -n "$state" ] && command -v jq >/dev/null 2>&1; then
      ok "board" "$probe answers, version $(printf '%s' "$state" | jq -r '.version'), ip $(printf '%s' "$state" | jq -r '.ip')"
    elif [ -n "$state" ]; then
      ok "board" "$probe answers"
    else
      warn "board" "$probe does not answer, check the network or pass HOST=<ip>"
    fi
  done
fi

echo "Claude Code"
check_command claude "the hooks need Claude Code" optional "https://claude.com/claude-code"

if command -v claude >/dev/null 2>&1; then
  ok "version" "$(claude --version 2>/dev/null | head -1)"
fi

if perl -MTime::HiRes -e 1 2>/dev/null; then
  ok "perl" "Time::HiRes present, hook requests carry millisecond timestamps"
else
  warn "perl" "Time::HiRes missing, hook requests fall back to whole seconds"
fi

if [ -f "$claude_home/settings.json" ]; then
  if grep -q 'punkt.sh' "$claude_home/settings.json" 2>/dev/null; then
    ok "hooks" "installed in $claude_home/settings.json"
  else
    warn "hooks" "not installed in $claude_home/settings.json, run make hooks-install"
  fi
else
  warn "hooks" "$claude_home/settings.json not found, run make hooks-install after Claude Code has started once"
fi

if [ "$missing" -gt 0 ]; then
  echo "$missing required tool(s) missing"
  exit 1
fi

echo "all required tools present"
