#!/bin/bash
# Maps Claude Code hook events to punkt states and posts them to the matrix.
# Usage: punkt.sh hook      hook payload on stdin, every registered event calls this form
#        punkt.sh context   status line payload on stdin, posts the context window fill and the usage
# A working post carries the activity Claude is in (thinking, reading, editing, shell, delegating,
# mcp), the board picks the animation for it.
# Environment: PUNKT_HOST (default punkt.local, a comma-separated list posts to every board),
#              PUNKT_DRY_RUN=1 prints the requests to stderr
set -u
trap 'exit 0' EXIT

# sampled first, the board orders the requests of one session by it
ts=$(perl -MTime::HiRes=time -e 'printf("%d", time() * 1000)' 2>/dev/null)

if [ -z "$ts" ]; then
  ts="$(date +%s)000"
fi

host="${PUNKT_HOST:-punkt.local}"
marker_dir="${TMPDIR:-/tmp}/punkt"
marker=""
activity_marker=""
turn_marker=""
deferred_marker=""
agents_dir=""
state=""
activity=""
input=$(cat 2>/dev/null)

# posts $2 to the path $1 on every host of the comma-separated list
post() {
  local target
  local -a targets
  IFS=',' read -r -a targets <<<"$host"

  for target in "${targets[@]}"; do
    if [ -z "$target" ]; then
      continue
    fi

    if [ -n "${PUNKT_DRY_RUN:-}" ]; then
      printf 'POST %s %s\n' "http://$target$1" "$2" >&2
      continue
    fi

    ( curl -4 -s -m 5 -X POST "http://$target$1" -H 'Content-Type: application/json' -d "$2" >/dev/null 2>&1 & ) >/dev/null 2>&1 &
    disown >/dev/null 2>&1
  done
}

payload() {
  if ! command -v jq >/dev/null 2>&1 || [ -z "$input" ]; then
    return
  fi

  printf '%s' "$input" | jq -r "$1" 2>/dev/null
}

tool_activity() {
  case "$1" in
    mcp__*)
      printf 'mcp'
      ;;
    Bash)
      printf 'shell'
      ;;
    Edit|Write|MultiEdit|NotebookEdit)
      printf 'editing'
      ;;
    Read|Grep|Glob|WebFetch|WebSearch)
      printf 'reading'
      ;;
  esac
}

mark_asked() {
  mkdir -p "$marker_dir" 2>/dev/null
  printf '%s' "$ts" >"$marker" 2>/dev/null
}

# succeeds and removes the marker when it was left at least $1 ms before this event
take_asked() {
  local asked
  asked=$(cat "$marker" 2>/dev/null)

  case "$asked" in
    ''|*[!0-9]*)
      return 1
      ;;
  esac

  if [ $((ts - asked)) -lt "$1" ]; then
    return 1
  fi

  rm -f "$marker"
}

remember_activity() {
  mkdir -p "$marker_dir" 2>/dev/null
  printf '%s' "$activity" >"$activity_marker" 2>/dev/null
}

recall_activity() {
  cat "$activity_marker" 2>/dev/null | tr -cd 'a-z'
}

agent_file() {
  local id
  id=$(printf '%s' "$1" | tr -cd 'A-Za-z0-9_-' | cut -c1-64)
  printf '%s/%s' "$agents_dir" "${id:-anonymous}"
}

track_agent() {
  mkdir -p "$agents_dir" 2>/dev/null
  touch "$(agent_file "$1")" 2>/dev/null
}

untrack_agent() {
  rm -f "$(agent_file "$1")"
}

agents_left() {
  [ -n "$(find "$agents_dir" -mindepth 1 2>/dev/null | head -n 1)" ]
}

# an agent whose stop never fired leaves its file behind, only a fresh one still counts as running
agents_running() {
  find "$agents_dir" -mindepth 1 -mmin +30 -delete 2>/dev/null
  agents_left
}

defer_done() {
  mkdir -p "$marker_dir" 2>/dev/null
  touch "$deferred_marker" 2>/dev/null
}

# appends one line per event to hook.log while $marker_dir/trace exists, for chasing lost posts
trace_hook() {
  if [ ! -f "$marker_dir/trace" ]; then
    return
  fi

  printf '%s %s %s %s -> %s %s\n' "$ts" "$event" "${tool:--}" "${session:-anonymous}" \
    "${state:-none}" "${activity:--}" >>"$marker_dir/hook.log" 2>/dev/null
}

# the board only leaves a running working animation alone while the activity stays the same, so a
# tool without an activity of its own repeats the one the session sent last
tool_state_activity() {
  local mapped
  mapped=$(tool_activity "$1")

  if [ -z "$mapped" ]; then
    mapped=$(recall_activity)
  fi

  printf '%s' "$mapped"
}

# prints the payload field $1 rounded and clamped to 0-100, nothing when it is absent
percent_field() {
  payload "
    ($1) as \$p
    | if \$p == null then empty
      else
        (\$p | if . < 0 then 0 elif . > 100 then 100 else . end) as \$c
        | (\$c + 0.5 | floor)
      end
  "
}

post_context() {
  local context usage
  context=$(percent_field '.context_window.used_percentage')
  usage=$(percent_field '.rate_limits.five_hour.used_percentage')

  if [ -n "$context" ]; then
    post "/context" "{\"percent\":$context}"
  fi

  if [ -n "$usage" ]; then
    post "/usage" "{\"percent\":$usage}"
  fi
}

post_hook() {
  local fields session event tool notification error_type trigger source agent
  fields=$(payload '[.session_id, .hook_event_name, .tool_name, .notification_type, .error, .trigger, .source, .agent_id] | map(. // "" | tostring) | join("")')
  IFS=$'\x1f' read -r session event tool notification error_type trigger source agent <<<"$fields"

  session=$(printf '%s' "$session" | tr -cd 'A-Za-z0-9_-' | cut -c1-32)
  marker="$marker_dir/${session:-anonymous}.asked"
  activity_marker="$marker_dir/${session:-anonymous}.activity"
  turn_marker="$marker_dir/${session:-anonymous}.turn"
  deferred_marker="$marker_dir/${session:-anonymous}.deferred"
  agents_dir="$marker_dir/${session:-anonymous}.agents"

  case "$event" in
    SessionStart)
      # compaction restarts the session, PostCompact decides what follows
      if [ "$source" != "compact" ]; then
        state="idle"
      fi
      ;;
    SessionEnd)
      rm -f "$marker" "$activity_marker" "$turn_marker" "$deferred_marker"
      rm -rf "$agents_dir"
      state="off"
      ;;
    UserPromptSubmit)
      rm -f "$marker" "$activity_marker" "$deferred_marker"
      mkdir -p "$marker_dir" 2>/dev/null
      touch "$turn_marker" 2>/dev/null
      state="working"
      ;;
    PreToolUse)
      case "$tool" in
        AskUserQuestion)
          state="attention"
          ;;
        Monitor|TaskOutput)
          state="waiting"
          ;;
        *)
          # a marker younger than 1 s belongs to this very call, PermissionRequest follows closely
          take_asked 1000

          # every tool posts, the board's per session watchdog drops a session that goes quiet
          state="working"
          activity=$(tool_state_activity "$tool")
          ;;
      esac
      ;;
    PostToolUse)
      case "$tool" in
        AskUserQuestion|Monitor|TaskOutput)
          rm -f "$marker"
          state="working"
          ;;
        *)
          if take_asked 0; then
            state="working"
            activity=$(tool_state_activity "$tool")
          fi
          ;;
      esac
      ;;
    PostToolUseFailure)
      rm -f "$marker"
      state="glitch"
      ;;
    PermissionRequest)
      mark_asked
      state="attention"
      ;;
    Notification)
      case "$notification" in
        permission_prompt|elicitation_dialog|elicitation_url_dialog)
          mark_asked
          state="attention"
          ;;
        idle_prompt)
          state="unread"
          ;;
      esac
      ;;
    SubagentStart)
      track_agent "$agent"

      # helper agents also run after a turn ended, only a running turn shows them
      if [ -f "$turn_marker" ]; then
        state="working"
        activity="delegating"
      fi
      ;;
    SubagentStop)
      untrack_agent "$agent"

      if [ -f "$turn_marker" ]; then
        state="working"
        activity=$(recall_activity)
      elif [ -f "$deferred_marker" ] && ! agents_running; then
        rm -f "$deferred_marker"
        state="done"
      fi
      ;;
    PreCompact)
      state="compacting"
      ;;
    PostCompact)
      case "$trigger" in
        auto)
          state="working"
          ;;
        manual)
          state="idle"
          ;;
      esac
      ;;
    Stop)
      rm -f "$marker" "$activity_marker" "$turn_marker"

      # the last SubagentStop posts the done the still running helper agents hold back here
      if agents_running; then
        defer_done
        state="working"
        activity="delegating"
      else
        rm -f "$deferred_marker"
        state="done"
      fi
      ;;
    StopFailure)
      rm -f "$marker" "$activity_marker" "$turn_marker" "$deferred_marker"
      rm -rf "$agents_dir"

      case "$error_type" in
        rate_limit|overloaded)
          state="limit"
          ;;
        *)
          state="error"
          ;;
      esac
      ;;
  esac

  trace_hook

  if [ -z "$state" ]; then
    return
  fi

  if [ "$state" = "working" ]; then
    remember_activity
  fi

  local json="{\"state\":\"$state\",\"ts\":$ts"

  if [ -n "$session" ]; then
    json="$json,\"session\":\"$session\""
  fi

  if [ -n "$activity" ]; then
    json="$json,\"activity\":\"$activity\""
  fi

  post "/state" "$json}"
}

case "${1:-}" in
  hook)
    post_hook
    ;;
  context)
    post_context
    ;;
esac
