#!/bin/sh
# Builds the host renderers, writes the punkt pictures to docs/animations and strich to docs/strich.
set -eu

root=$(cd "$(dirname "$0")/../.." && pwd)
build="$root/tools/render/build"
anim="$root/src/animations"
cxx=${CXX:-c++}

mkdir -p "$build/strich" "$root/docs/animations" "$root/docs/strich"
$cxx -std=gnu++17 -O1 -I "$root/tools/render/stub" -I "$root/src" \
  -o "$build/render" "$root/tools/render/main.cpp" "$root"/src/animations/*.cpp

# src before stub, so the strich build takes the real Display, Animation and TimeSync headers
$cxx -std=gnu++17 -O1 -DPUNKT_BOARD_STRICH -I "$root/src" -I "$root/tools/render/stub" \
  -o "$build/render-strich" "$root"/tools/render/strich/*.cpp "$root"/src/clockFaces/*.cpp \
  "$root/src/DisplayLayers.cpp" "$root/src/BrightnessRamp.cpp" "$anim/Knight.cpp" \
  "$anim/UsageView.cpp" "$anim/bottomFill.cpp" "$anim/blink.cpp" "$anim/digitFont.cpp" \
  "$anim/marker.cpp" "$anim/contextGauge.cpp"

# Renders every picture of the list on stdin: renderer binary, output directory, dump directory.
render_list() {
  renderer=$1
  out=$2
  dumps=$3

  # name durationMs seed percent windowStartMs budgetEndMs stillMs mode [gif|png]
  while read -r name duration seed percent start budget still mode only; do
    if [ -z "$name" ]; then
      continue
    fi

    gif="$out/$name.gif"
    png="$out/$name.png"

    if [ "$only" = png ]; then
      gif=-
    fi

    if [ "$only" = gif ]; then
      png=-
    fi

    "$renderer" "$name" "$duration" "$seed" "$percent" > "$dumps/$name.txt"
    info=$(python3 "$root/tools/render/gif.py" "$dumps/$name.txt" "$gif" "$png" \
      "$start" "$budget" "$still" "$mode")

    written=$gif

    if [ "$gif" = - ]; then
      written=$png
    fi

    printf '%-16s %4s KB  %s\n' "$name" "$(($(wc -c < "$written") / 1024))" "$info"
  done
}

render_list "$build/render" "$root/docs/animations" "$build" <<'PICTURES'
off 25000 1 0 0 21000 9900 loop
loading 8000 1 0 0 6300 1500 loop
attention 7000 1 0 0 6000 300 loop
done 5000 1 0 0 4900 900 shot
error 7000 1 0 0 6000 300 loop
waiting 15000 1 60 0 12000 2000 loop
compacting 8000 1 0 0 7200 200 loop
limit 9000 1 0 0 8000 1200 loop
unread 13000 1 60 0 12000 1000 loop
glitch 2500 1 0 0 2100 1200 shot
clock 9000 1 0 0 6000 3000 loop
tetris 12000 54 0 0 9000 3300 clear
life 15500 6 0 0 12000 1200 cut
knight 10000 1 0 0 8400 700 loop
orbit 8000 1 0 0 6300 1500 loop
code 15000 1 0 0 12000 4000 clear
rain 11500 1 0 0 8000 3000 cut
scan 11000 1 0 0 7600 2000 clear
pipes 15000 38 0 0 12000 6500 clear
pointer 15500 60 0 0 12000 3600 cut
pong 11500 1 0 0 8000 1500 cut
terminal 15000 1 0 0 12000 4481 cut
reset 10000 1 0 0 9500 4000 shot
wifi-search 8000 1 0 0 7600 1100 loop
wifi-lost 9000 1 0 0 8000 500 loop
ota 8000 1 0 0 7500 3000 shot
usage 13000 1 85 0 12000 500 loop
PICTURES

render_list "$build/render-strich" "$root/docs/strich" "$build/strich" <<'PICTURES'
clock-staircase 1000 1 0 0 1000 0 shot png
clock-tall 1000 1 0 0 1000 0 shot png
normal 10000 1 0 0 8400 700 loop gif
split-digits 1000 1 0 0 1000 700 shot png
split-squares 1000 1 0 0 1000 700 shot png
split-stagger 1000 1 0 0 1000 700 shot png
split-bar 1000 1 0 0 1000 700 shot png
usage 1000 1 60 0 1000 0 shot gif
battery 1000 1 84 0 1000 0 shot png
battery-charging 1000 1 0 0 1000 0 shot png
slide 7330 1 0 0 7330 0 shot gif
low-battery 5000 1 0 0 2000 0 loop gif
PICTURES
