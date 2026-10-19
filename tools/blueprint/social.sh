#!/bin/sh
# Renders docs/blueprint/social-preview.png, 1280x640, from a header SVG with headless Chrome.
# usage: tools/blueprint/social.sh [header-b.svg]
set -eu

root=$(cd "$(dirname "$0")/../.." && pwd)
svg="$root/docs/blueprint/${1:-header-b.svg}"
chrome=${CHROME:-"/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"}
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

cat > "$work/social.html" <<HTML
<!doctype html><meta charset="utf-8">
<style>html,body{margin:0;width:1280px;height:640px;background:#fff;display:flex;align-items:center;justify-content:center}
img{height:540px;max-width:1160px}</style>
<img src="file://$svg">
HTML

"$chrome" --headless=new --disable-gpu --hide-scrollbars --window-size=1280,640 \
  --screenshot="$root/docs/blueprint/social-preview.png" "file://$work/social.html" >/dev/null 2>&1
echo "docs/blueprint/social-preview.png"
