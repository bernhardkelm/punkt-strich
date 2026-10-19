#!/bin/bash
# Installs the punkt hooks into the Claude Code user settings or removes them again.
# Usage: install.sh install | uninstall | diff
# Environment: CLAUDE_HOME, else CLAUDE_CONFIG_DIR (default ~/.claude) points the files at another directory
#              PUNKT_HOST pins the installed commands to one board, e.g. punkt-work.local,
#              or to several boards separated by commas, e.g. punkt.local,strich.local
set -euo pipefail

source_dir=$(cd "$(dirname "$0")" && pwd)
claude_home="${CLAUDE_HOME:-${CLAUDE_CONFIG_DIR:-$HOME/.claude}}"

# zsh passes make CLAUDE_HOME=~/dir through unexpanded
# shellcheck disable=SC2088
case "$claude_home" in
  "~"|"~/"*)
    claude_home="$HOME${claude_home#\~}"
    ;;
esac

punkt_host="${PUNKT_HOST:-}"
settings="$claude_home/settings.json"
hooks_dir="$claude_home/hooks"
scripts="punkt.sh punkt-statusline.sh"
# the default keeps $HOME unexpanded in settings.json, another home is written out literally
hooks_prefix='$HOME/.claude/hooks/'
wrapped_statusline='$HOME/.claude/statusline-command.sh'
statusline_prev="$hooks_dir/punkt-statusline.prev.json"
backup=""
backup_skipped=""
# same expression as marker_dir in punkt.sh
marker_dir="${TMPDIR:-/tmp}/punkt"

if [ "$claude_home" != "$HOME/.claude" ]; then
  hooks_prefix="$hooks_dir/"
  wrapped_statusline="$claude_home/statusline-command.sh"
fi

if [ -n "$punkt_host" ]; then
  case "$punkt_host" in
    ,*|*,|*,,*)
      echo "PUNKT_HOST must not contain empty entries, got: $punkt_host" >&2
      exit 1
      ;;
  esac

  IFS=',' read -r -a host_entries <<<"$punkt_host"

  for entry in "${host_entries[@]}"; do
    if [ -z "$entry" ]; then
      echo "PUNKT_HOST must not contain empty entries, got: $punkt_host" >&2
      exit 1
    fi

    case "$entry" in
      *[!A-Za-z0-9.-]*)
        echo "PUNKT_HOST entries may only contain letters, digits, dots and hyphens, got: $entry" >&2
        exit 1
        ;;
    esac

    if [ "${#entry}" -gt 64 ]; then
      echo "PUNKT_HOST entries may be at most 64 characters, got ${#entry} in: $entry" >&2
      exit 1
    fi
  done

  # an environment prefix on the command, the hook runner passes it through a shell
  hooks_prefix="PUNKT_HOST=$punkt_host $hooks_prefix"
fi

# every hook or status line whose command runs a punkt script belongs to this installer
common_filter='
  def is_punkt: (.command // "") | test("/punkt(-statusline)?\\.sh\\b");

  def strip_hooks:
    if has("hooks") then
      .hooks |= (
        with_entries(.value |= (map(.hooks |= map(select(is_punkt | not))) | map(select(.hooks | length > 0))))
        | with_entries(select(.value | length > 0))
      )
      | if .hooks == {} then del(.hooks) else . end
    else
      .
    end;
'

install_filter="$common_filter"'
  ($punkt[0] | walk(if type == "string" then sub("^\\$HOME/\\.claude/hooks/"; $prefix) else . end)) as $p
  | strip_hooks
  | .hooks = (reduce ($p.hooks | to_entries[]) as $event (.hooks // {};
      .[$event.key] = ((.[$event.key] // []) + $event.value)))
  | if .statusLine == null or (.statusLine | is_punkt) or ((.statusLine.command // "") == $wrapped) then
      .statusLine = $p.statusLine
    else
      .
    end
'

uninstall_filter="$common_filter"'
  strip_hooks
  | if (.statusLine // {} | is_punkt) then
      if $previous == null then
        if $restore then
          .statusLine.command = $wrapped
        else
          del(.statusLine)
        end
      elif $previous == "none" then
        del(.statusLine)
      else
        .statusLine = $previous
      end
    else
      .
    end
'

current_settings() {
  if [ -f "$settings" ]; then
    cat "$settings"
  else
    printf '{}'
  fi
}

render() {
  local restore=false
  local previous=null

  if [ -x "$claude_home/statusline-command.sh" ]; then
    restore=true
  fi

  # no record means an install from before the sidecar existed, $restore is the fallback there
  if [ -f "$statusline_prev" ]; then
    previous=$(jq -c '.previous' "$statusline_prev" 2>/dev/null || printf 'null')
  fi

  if [ -z "$previous" ]; then
    previous=null
  fi

  current_settings | jq --indent 2 \
    --slurpfile punkt "$source_dir/hooks.json" \
    --arg wrapped "$wrapped_statusline" \
    --arg prefix "$hooks_prefix" \
    --argjson restore "$restore" \
    --argjson previous "$previous" \
    "$1"
}

# keeps what .statusLine was before the first install, so uninstall restores that exact value
record_statusline() {
  local previous

  if [ -f "$statusline_prev" ]; then
    return
  fi

  previous=$(current_settings | jq -c "$common_filter"'
    if .statusLine == null then
      "none"
    elif (.statusLine | is_punkt) then
      empty
    else
      .statusLine
    end
  ')

  if [ -z "$previous" ]; then
    return
  fi

  mkdir -p "$hooks_dir"
  printf '{"previous":%s}\n' "$previous" >"$statusline_prev"
}

# true while the settings hold a punkt hook or the punkt status line, a copy of those is no backup
has_punkt() {
  current_settings | jq -e "$common_filter"'
    [((.hooks // {})[]?[]?.hooks[]?), (.statusLine // {})] | map(objects) | any(is_punkt)
  ' >/dev/null
}

# only an install from settings without punkt writes the backup, a reinstall would save its own output
backup_settings() {
  if [ ! -f "$settings" ]; then
    return
  fi

  if has_punkt; then
    backup_skipped=1
    return
  fi

  cp "$settings" "$settings.bak"
  backup="$settings.bak"
}

write_settings() {
  local next="$1"

  mkdir -p "$claude_home"
  printf '%s\n' "$next" >"$settings.tmp"
  mv "$settings.tmp" "$settings"
}

report() {
  if [ -n "$backup" ]; then
    echo "$1, previous version in $backup"
  elif [ -n "$backup_skipped" ]; then
    echo "$1, punkt was already installed, backup left alone"
  else
    echo "$1"
  fi
}

install_hooks() {
  local next
  next=$(render "$install_filter")

  mkdir -p "$hooks_dir"

  for script in $scripts; do
    install -m 755 "$source_dir/$script" "$hooks_dir/$script"
  done

  record_statusline
  backup_settings
  write_settings "$next"
  report "punkt hooks installed into $settings"

  if ! printf '%s' "$next" | jq -e '.statusLine.command | test("punkt-statusline")' >/dev/null; then
    echo "statusLine left alone, it runs another command, the context gauge needs punkt-statusline.sh"
  fi
}

uninstall_hooks() {
  local next
  next=$(render "$uninstall_filter")

  write_settings "$next"

  for script in $scripts; do
    rm -f "$hooks_dir/$script"
  done

  rm -f "$statusline_prev"

  case "$marker_dir" in
    */punkt)
      rm -rf "$marker_dir"
      ;;
  esac

  report "punkt hooks removed from $settings"

  if [ -f "$settings.bak" ]; then
    echo "$settings.bak stays, delete it once it is no longer needed"
  fi
}

diff_hooks() {
  local next
  next=$(render "$install_filter")

  if printf '%s\n' "$next" | diff -u -L "$settings" -L "after hooks-install" <(current_settings) -; then
    echo "settings already match claude/hooks.json"
  fi

  for script in $scripts; do
    if ! cmp -s "$source_dir/$script" "$hooks_dir/$script"; then
      echo "$hooks_dir/$script differs from claude/$script"
    fi
  done
}

if ! command -v jq >/dev/null 2>&1; then
  echo "install.sh needs jq on the PATH" >&2
  exit 1
fi

case "${1:-}" in
  install)
    install_hooks
    ;;
  uninstall)
    uninstall_hooks
    ;;
  diff)
    diff_hooks
    ;;
  *)
    echo "usage: install.sh install | uninstall | diff" >&2
    exit 1
    ;;
esac
