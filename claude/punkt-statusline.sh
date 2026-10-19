#!/bin/bash
# Feeds the status line payload to punkt.sh context (context fill and usage), then renders the regular status line unchanged.
set -u

dir=$(cd "$(dirname "$0")" && pwd)
input=$(cat)

printf '%s' "$input" | "$dir/punkt.sh" context

if [ -x "$dir/../statusline-command.sh" ]; then
  printf '%s' "$input" | "$dir/../statusline-command.sh"
  exit $?
fi

exit 0
