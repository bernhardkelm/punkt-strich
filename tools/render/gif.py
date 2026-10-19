"""Renders frame dumps from the host harness as an LED matrix GIF and a still PNG, 8 columns wide
unless the dump names its width."""
import sys

from PIL import Image, ImageDraw, ImageFilter

SCALE = 4
CELL = 24
PAD = 12
LED = 8.5
SIZE = PAD * 2 + CELL * 8
COLUMNS = 8
BOARD = (24, 22, 21)
OFF = (42, 38, 35)
ON = (232, 121, 43)
HIGHLIGHT = (250, 180, 120)
GLOW = (240, 110, 30)
GLOW_RADIUS = LED * 1.25
GLOW_BLUR = LED * SCALE * 0.6
GLOW_BLEND = 0.3
COLOURS = 64
MIN_FRAME_MS = 30
# a GIF stores its delays in hundredths of a second
GIF_STEP_MS = 10
MIN_LOOP_MS = 6000
MIN_PERIOD_EVENTS = 2
PERIOD_TOLERANCE_MS = 10
CUT_SEARCH_MS = 2500
PALETTE_SAMPLES = 12


def level_mix(level):
    """
    Returns how far a hardware brightness level sits between unlit and fully lit, gamma shaped so
    the dim levels read dim.
    @param {int} level - hardware brightness, 0 to 15
    @returns {float} - 0.3 at the dimmest level, 1.0 at the brightest
    """
    return 0.3 + 0.7 * (level / 15.0) ** 1.6


def blend(colour, level):
    """
    Returns a lit colour pulled back towards the unlit LED for a hardware brightness level.
    @param {tuple} colour - the colour at full brightness
    @param {int} level - hardware brightness, 0 to 15
    @returns {tuple} - the RGB the LED shows
    """
    mix = level_mix(level)
    return tuple(int(OFF[i] + (colour[i] - OFF[i]) * mix) for i in range(3))


def disc(draw, x, y, radius, fill):
    """
    Draws one supersampled disc centred on a cell of the matrix.
    @param {ImageDraw} draw - drawing context of the supersampled board
    @param {int} x - column, 0 is left
    @param {int} y - row, 0 is top
    @param {float} radius - disc radius in board pixels
    @param {tuple} fill - RGB the disc is filled with
    @returns {None}
    """
    cx = (PAD + x * CELL + CELL / 2) * SCALE
    cy = (PAD + y * CELL + CELL / 2) * SCALE
    r = radius * SCALE
    draw.ellipse((cx - r, cy - r, cx + r, cy + r), fill=fill)


def board_width(columns):
    """
    Returns the width of a rendered board, its height being SIZE for every board.
    @param {int} columns - LED columns of the board
    @returns {int} - width in pixels
    """
    return PAD * 2 + CELL * columns


def frame_image(rows, brightness, columns=COLUMNS):
    """
    Draws one frame as an image of round LEDs on a dark board with a soft glow.
    @param {list} rows - the eight row masks, one bit per column
    @param {int} brightness - hardware brightness of the frame, 0 to 15
    @param {int} columns - LED columns of the board
    @returns {Image} - the rendered frame at its final size
    """
    width = board_width(columns)
    big_w, big_h = width * SCALE, SIZE * SCALE
    board = Image.new("RGB", (big_w, big_h), BOARD)
    corners = (0, 0, big_w - 1, big_h - 1)
    ImageDraw.Draw(board).rounded_rectangle(corners, radius=14 * SCALE, fill=BOARD)
    lit = blend(ON, brightness)
    highlight = blend(HIGHLIGHT, brightness)
    strength = level_mix(brightness)

    glow = Image.new("RGB", (big_w, big_h), (0, 0, 0))
    glow_draw = ImageDraw.Draw(glow)
    glow_colour = tuple(int(c * strength) for c in GLOW)

    for y in range(8):
        for x in range(columns):
            if (rows[y] >> x) & 1:
                disc(glow_draw, x, y, GLOW_RADIUS, glow_colour)

    glow = glow.filter(ImageFilter.GaussianBlur(radius=GLOW_BLUR))
    board = Image.blend(board, Image.composite(glow, board, glow.convert("L").point(lambda v: min(255, v * 2))), GLOW_BLEND)

    draw = ImageDraw.Draw(board)

    for y in range(8):
        for x in range(columns):
            on = (rows[y] >> x) & 1
            disc(draw, x, y, LED, lit if on else OFF)

            if on:
                disc(draw, x, y, LED * 0.55, highlight)

    return board.resize((width, SIZE), Image.LANCZOS)


def read_frames(path):
    """
    Reads a frame dump, one entry per timestamp, plus the point the harness ran to and the board
    width a "width" line names.
    @param {str} path - frame dump written by the render binary
    @returns {tuple} - ([(timeMs, brightness, rows)], horizonMs, columns)
    """
    frames = []
    horizon = 0
    columns = COLUMNS

    for line in open(path):
        parts = line.split()

        if parts and parts[0] == "end":
            horizon = int(parts[1])
            continue

        if parts and parts[0] == "width":
            columns = int(parts[1])
            continue

        if len(parts) < 10:
            continue

        frame = [int(parts[0]), int(parts[1]), [int(v) for v in parts[2:10]]]

        # a brightness change and the redraw after it land on the same millisecond, the last wins
        if frames and frames[-1][0] == frame[0]:
            frames[-1] = frame
            continue

        frames.append(frame)

    return frames, horizon, columns


def picture(frame):
    """
    Returns the comparable picture of a frame, brightness and rows.
    @param {tuple} frame - one dump entry
    @returns {tuple} - brightness and the eight row masks
    """
    return (frame[1], tuple(frame[2]))


def frame_at(frames, time_ms):
    """
    Returns the frame showing at a point in time.
    @param {list} frames - the whole dump
    @param {int} time_ms - point to look at
    @returns {tuple} - the last frame pushed at or before time_ms, None before the first one
    """
    showing = None

    for frame in frames:
        if frame[0] > time_ms:
            break

        showing = frame

    return showing


def events(frames, start_ms, end_ms):
    """
    Returns the picture changes of a window, each as its offset from the start.
    @param {list} frames - the whole dump
    @param {int} start_ms - first millisecond of the window
    @param {int} end_ms - first millisecond after the window
    @returns {list} - (offsetMs, picture) pairs, opening with the picture showing at start_ms
    """
    opening = frame_at(frames, start_ms)
    changes = [(0, picture(opening))] if opening else []

    for frame in frames:
        if start_ms < frame[0] < end_ms:
            changes.append((frame[0] - start_ms, picture(frame)))

    return changes


def same_sequence(first, second):
    """
    Returns whether two windows run through the same pictures at the same offsets, allowing for
    the millisecond the harness tick lands on.
    @param {list} first - (offsetMs, picture) pairs of one window
    @param {list} second - (offsetMs, picture) pairs of the other window
    @returns {bool} - true when the two windows show the same thing
    """
    if len(first) != len(second):
        return False

    return all(a[1] == b[1] and abs(a[0] - b[0]) <= PERIOD_TOLERANCE_MS for a, b in zip(first, second))


def cycle(frames, start_ms, period_ms):
    """
    Returns the picture changes of one period, without the opening of the period after it.
    @param {list} frames - the whole dump
    @param {int} start_ms - first millisecond of the period
    @param {int} period_ms - length of the period
    @returns {list} - (offsetMs, picture) pairs
    """
    window = events(frames, start_ms, start_ms + period_ms)
    return [e for e in window if e[0] == 0 or e[0] < period_ms - PERIOD_TOLERANCE_MS]


def detect_period(frames, horizon_ms, start_ms, max_ms):
    """
    Returns the loop length of an animation, the smallest shift at which its picture sequence
    repeats itself for one further full period.
    @param {list} frames - the whole dump
    @param {int} horizon_ms - point the harness ran to, only what lies before it was observed
    @param {int} start_ms - point the loop is measured from
    @param {int} max_ms - longest period worth testing
    @returns {int} - the period in milliseconds, 0 when the dump never repeats
    """
    opening = frame_at(frames, start_ms)

    if opening is None:
        return 0

    for frame in frames:
        period = frame[0] - start_ms

        if period <= 0 or period > max_ms:
            continue

        if horizon_ms < start_ms + 2 * period:
            break

        if picture(frame) != picture(opening):
            continue

        first = cycle(frames, start_ms, period)

        if len(first) >= MIN_PERIOD_EVENTS and same_sequence(first, cycle(frames, start_ms + period, period)):
            return period

    return 0


def dark_times(frames, start_ms, end_ms):
    """
    Returns the times inside a window at which the matrix is completely dark.
    @param {list} frames - the whole dump
    @param {int} start_ms - first millisecond of the window
    @param {int} end_ms - last millisecond of the window
    @returns {list} - timestamps of the dark frames
    """
    return [f[0] for f in frames if start_ms <= f[0] <= end_ms and not any(f[2])]


def distance(left, right):
    """
    Returns how far two pictures are apart, the pixels they differ in.
    @param {tuple} left - picture of one frame
    @param {tuple} right - picture of the other frame
    @returns {int} - number of differing pixels, 64 when one side is missing
    """
    if left is None or right is None:
        return 64

    return sum(bin(left[1][y] ^ right[1][y]).count("1") for y in range(8))


def best_cut(frames, horizon_ms, start_ms, end_ms):
    """
    Returns the start of a fixed length window whose first picture is closest to its last one,
    the best a random animation without a period can do.
    @param {list} frames - the whole dump
    @param {int} horizon_ms - point the harness ran to, a window never ends past it
    @param {int} start_ms - earliest start to consider
    @param {int} end_ms - end the window has with that earliest start
    @returns {int} - the chosen start in milliseconds
    """
    span = end_ms - start_ms
    latest_ms = min(start_ms + CUT_SEARCH_MS, horizon_ms - span)
    candidates = [start_ms] + [f[0] for f in frames if start_ms < f[0] <= latest_ms]
    return min(candidates, key=lambda t: distance(picture(frame_at(frames, t)), picture(frame_at(frames, t + span))))


def window_for(frames, horizon_ms, start_ms, budget_ms, mode):
    """
    Returns the window the GIF covers and how its loop was closed.
    @param {list} frames - the whole dump
    @param {int} horizon_ms - point the harness ran to
    @param {int} start_ms - earliest start of the window
    @param {int} budget_ms - latest end of the window, a hard cut slides past it by up to CUT_SEARCH_MS
    @param {str} mode - shot for a one shot, clear to loop from dark to dark, cut for a hard cut,
     loop to close on the opening picture
    @returns {tuple} - (startMs, endMs, note describing the loop)
    """
    if mode == "shot":
        return start_ms, budget_ms, "one shot, held to the end"

    if mode == "clear":
        dark = dark_times(frames, start_ms, budget_ms)

        if len(dark) >= 2 and dark[-1] - dark[0] >= MIN_LOOP_MS:
            return dark[0], dark[-1], f"clear to clear, {len(dark)} clears"

    period = detect_period(frames, horizon_ms, start_ms, budget_ms - start_ms)

    if period > 0:
        # a period measured off the first drawn frame runs a tick long, the slack keeps the count
        loops = max(1, (budget_ms - start_ms + PERIOD_TOLERANCE_MS) // period)
        return start_ms, start_ms + loops * period, f"period {period} ms x {loops}"

    if mode != "cut":
        opening = picture(frame_at(frames, start_ms))
        same = [f[0] for f in frames if start_ms + MIN_LOOP_MS <= f[0] <= budget_ms and picture(f) == opening]

        if same:
            # frames are change events, the opening picture holds until the next one
            following = [f[0] for f in frames if f[0] > same[-1]]
            end_ms = min(budget_ms, following[0]) if following else budget_ms
            return start_ms, end_ms, "closes on the opening picture"

    span = budget_ms - start_ms
    start_ms = best_cut(frames, horizon_ms, start_ms, budget_ms)
    return start_ms, start_ms + span, "hard cut, nearest matching start"


def timeline(frames, start_ms, end_ms):
    """
    Cuts the window out of a dump, opening it on the frame that was showing at start_ms.
    @param {list} frames - the whole dump
    @param {int} start_ms - first millisecond of the window
    @param {int} end_ms - first millisecond after the window
    @returns {list} - [timeMs, brightness, rows, durationMs] per frame
    """
    opening = frame_at(frames, start_ms)
    inside = [f for f in frames if start_ms < f[0] < end_ms]
    kept = ([(start_ms, opening[1], opening[2])] if opening else []) + inside
    shots = []

    for i, (t, brightness, rows) in enumerate(kept):
        next_t = kept[i + 1][0] if i + 1 < len(kept) else end_ms
        shots.append([t, brightness, rows, next_t - t])

    return shots


def thin(shots, min_ms):
    """
    Drops frames shown for less than min_ms and gives their time to the frame before them.
    @param {list} shots - the timeline
    @param {int} min_ms - shortest delay a frame of its own is worth
    @returns {list} - the thinned timeline, the total duration unchanged
    """
    kept = []

    for shot in shots:
        if kept and shot[3] < min_ms:
            kept[-1][3] += shot[3]
            continue

        kept.append(shot)

    return kept


def quantise(shots, step_ms):
    """
    Rounds every delay onto the grid a GIF stores its timing on, carrying the rounding error into
    the next frame so the window keeps its exact length.
    @param {list} shots - the timeline
    @param {int} step_ms - the grid, 10 ms for a GIF
    @returns {list} - the timeline with grid aligned delays
    """
    carry = 0

    for shot in shots:
        wanted = shot[3] + carry
        rounded = max(step_ms, int(round(wanted / step_ms)) * step_ms)
        carry = wanted - rounded
        shot[3] = rounded

    return shots


def build_palette(images):
    """
    Returns one palette image covering every frame, so the GIF carries a single colour table.
    @param {list} images - the rendered frames
    @returns {Image} - a quantized image whose palette every frame is mapped onto
    """
    step = max(1, len(images) // PALETTE_SAMPLES)
    samples = images[::step][:PALETTE_SAMPLES]
    width, height = samples[0].size
    sheet = Image.new("RGB", (width, height * len(samples)))

    for i, image in enumerate(samples):
        sheet.paste(image, (0, height * i))

    return sheet.quantize(colors=COLOURS, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)


def main():
    """
    Writes the GIF of one loop of a picture and the still PNG of a frame inside it, taking the
    frame dump, the two output paths, the window, the still and the loop mode as arguments. An
    output path of "-" skips that output.
    @returns {None}
    """
    src, gif, png = sys.argv[1], sys.argv[2], sys.argv[3]
    start_ms, budget_ms = int(sys.argv[4]), int(sys.argv[5])
    still_ms = int(sys.argv[6]) if len(sys.argv) > 6 else start_ms
    mode = sys.argv[7] if len(sys.argv) > 7 else "loop"
    frames, horizon_ms, columns = read_frames(src)
    start_ms, end_ms, note = window_for(frames, horizon_ms, start_ms, budget_ms, mode)
    shots = quantise(thin(timeline(frames, start_ms, end_ms), MIN_FRAME_MS), GIF_STEP_MS)
    durations = [shot[3] for shot in shots]

    if gif != "-":
        images = [frame_image(rows, brightness, columns) for _, brightness, rows, _ in shots]
        palette = build_palette(images)
        flat = [image.quantize(palette=palette, dither=Image.Dither.NONE) for image in images]
        flat[0].save(gif, save_all=True, append_images=flat[1:], duration=durations, loop=0,
                     disposal=1, optimize=True)

    still = min(shots, key=lambda shot: abs(shot[0] - still_ms))

    if png != "-":
        frame_image(still[2], still[1], columns).save(png)

    print(f"{len(shots):3d} frames, {sum(durations):5d} ms, still {still[0]:5d} ms, {note}")


if __name__ == "__main__":
    main()
