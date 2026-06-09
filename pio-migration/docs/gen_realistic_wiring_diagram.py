"""Render a realistic wiring diagram for the RP2040 TLV493D SpaceMouse build."""

from __future__ import annotations

import math
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont


CANVAS_W = 1800
CANVAS_H = 2300
SCALE = 2
ORTHOGONAL_WIRES = True
LEFT_PIN_LAYOUT = (
    ("VOUT", "VOUT"),
    ("VIN", "VIN"),
    ("GND", "GND"),
    ("GP23", "GP23 LED"),
    ("3V3", "3V3"),
    ("GP29", "GP29"),
    ("GP28", "GP28"),
    ("GND_L1", "GND"),
    ("GP27", "GP27"),
    ("GP26", "GP26"),
    ("RUN", "RUN"),
    ("GP22", "GP22"),
    ("GND_L2", "GND"),
    ("GP21", "GP21"),
    ("GP20", "GP20"),
    ("GP19", "GP19"),
    ("GP18", "GP18"),
    ("GND_L3", "GND"),
    ("GP17", "GP17"),
    ("GP16", "GP16"),
)
RIGHT_PIN_LAYOUT = (
    ("GP0", "GP0 BTN0"),
    ("GP1", "GP1 BTN1"),
    ("GND_R0", "GND"),
    ("GP2", "GP2 SDA1"),
    ("GP3", "GP3 SCL1"),
    ("GP4", "GP4 SDA0"),
    ("GP5", "GP5 SCL0"),
    ("GND_R1", "GND"),
    ("GP6", "GP6 BTN2"),
    ("GP7", "GP7"),
    ("GP8", "GP8"),
    ("GP9", "GP9"),
    ("GND_R2", "GND"),
    ("GP10", "GP10"),
    ("GP11", "GP11"),
    ("GP12", "GP12"),
    ("GP13", "GP13"),
    ("GND_R3", "GND"),
    ("GP14", "GP14"),
    ("GP15", "GP15"),
)
BOTTOM_PIN_LAYOUT = (
    ("SWD_GND", "GND"),
    ("SWCLK", "SWCLK"),
    ("SWDIO", "SWDIO"),
    ("3V3_BOOT", "3V3"),
)
RP2040_PIN_LABELS = tuple(
    [label.split()[0] for _, label in LEFT_PIN_LAYOUT + RIGHT_PIN_LAYOUT + BOTTOM_PIN_LAYOUT]
    + ["GND", "VBUS", "VSYS"]
)

COLORS = {
    "paper": (244, 242, 236),
    "paper_shadow": (206, 199, 185),
    "black": (28, 29, 31),
    "white": (252, 252, 250),
    "muted": (94, 91, 84),
    "board": (25, 111, 87),
    "board_dark": (17, 70, 58),
    "board_edge": (14, 82, 66),
    "copper": (210, 178, 82),
    "chip": (30, 38, 44),
    "sensor": (37, 78, 124),
    "sensor_edge": (22, 45, 75),
    "plate": (73, 67, 172),
    "plate_edge": (50, 47, 135),
    "switch": (24, 24, 24),
    "metal": (184, 190, 190),
    "usb": (185, 190, 194),
    "red": (210, 39, 33),
    "black_wire": (15, 15, 15),
    "green": (24, 182, 68),
    "cyan": (19, 186, 176),
    "yellow": (229, 179, 38),
    "orange": (219, 122, 48),
    "blue": (24, 89, 165),
    "purple": (154, 54, 159),
    "gray": (98, 98, 98),
}


def _font(size: int, bold: bool = False) -> ImageFont.FreeTypeFont:
    names = [
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf" if bold else "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Bold.ttf" if bold else "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf",
    ]
    for name in names:
        try:
            return ImageFont.truetype(name, size * SCALE)
        except OSError:
            pass
    return ImageFont.load_default()


def _xy(point: tuple[float, float]) -> tuple[int, int]:
    return (round(point[0] * SCALE), round(point[1] * SCALE))


def _box(box: tuple[float, float, float, float]) -> tuple[int, int, int, int]:
    return tuple(round(v * SCALE) for v in box)


def rounded(draw: ImageDraw.ImageDraw, box: tuple[float, float, float, float], radius: int, fill, outline=None, width: int = 1) -> None:
    draw.rounded_rectangle(_box(box), radius=radius * SCALE, fill=fill, outline=outline, width=width * SCALE)


def text(
    draw: ImageDraw.ImageDraw,
    xy: tuple[float, float],
    label: str,
    size: int = 28,
    fill=COLORS["black"],
    bold: bool = False,
    anchor: str = "mm",
) -> None:
    draw.text(_xy(xy), label, font=_font(size, bold), fill=fill, anchor=anchor)


def shadow_layer(base: Image.Image, box: tuple[float, float, float, float], radius: int, alpha: int = 75) -> None:
    layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    d.rounded_rectangle(_box((box[0] + 10, box[1] + 16, box[2] + 10, box[3] + 16)), radius=radius * SCALE, fill=(0, 0, 0, alpha))
    layer = layer.filter(ImageFilter.GaussianBlur(14 * SCALE))
    base.alpha_composite(layer)


def circle_shadow(base: Image.Image, center: tuple[float, float], radius: float, alpha: int = 75) -> None:
    layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    cx, cy = center
    d.ellipse(_box((cx - radius + 12, cy - radius + 18, cx + radius + 12, cy + radius + 18)), fill=(0, 0, 0, alpha))
    layer = layer.filter(ImageFilter.GaussianBlur(16 * SCALE))
    base.alpha_composite(layer)


def bezier_points(points: list[tuple[float, float]], steps: int = 80) -> list[tuple[float, float]]:
    if len(points) == 2:
        return points
    if len(points) != 4:
        result: list[tuple[float, float]] = []
        for a, b in zip(points, points[1:]):
            result.extend(bezier_points([a, b], steps=2))
        return result
    p0, p1, p2, p3 = points
    out = []
    for i in range(steps + 1):
        t = i / steps
        mt = 1 - t
        x = mt**3 * p0[0] + 3 * mt**2 * t * p1[0] + 3 * mt * t**2 * p2[0] + t**3 * p3[0]
        y = mt**3 * p0[1] + 3 * mt**2 * t * p1[1] + 3 * mt * t**2 * p2[1] + t**3 * p3[1]
        out.append((x, y))
    return out


def orthogonal_points(points: list[tuple[float, float]]) -> list[tuple[float, float]]:
    routed = [points[0]]
    for target in points[1:]:
        current = routed[-1]
        if current[0] != target[0] and current[1] != target[1]:
            routed.append((target[0], current[1]))
        routed.append(target)
    return routed


def wire(draw: ImageDraw.ImageDraw, points: list[tuple[float, float]], color, width: int = 8, label: str | None = None) -> None:
    routed = orthogonal_points(points)
    pts = [_xy(p) for p in routed]
    draw.line(pts, fill=(0, 0, 0, 55), width=(width + 4) * SCALE, joint="curve")
    draw.line(pts, fill=color, width=width * SCALE, joint="curve")
    draw.line(pts, fill=tuple(min(255, c + 42) for c in color), width=max(1, width // 3) * SCALE, joint="curve")
    if label:
        mid = routed[len(routed) // 2]
        text(draw, (mid[0] + 8, mid[1] - 12), label, 20, color, True, "lm")


def pin(draw: ImageDraw.ImageDraw, x: float, y: float, label: str, side: str, color=COLORS["copper"]) -> None:
    draw.ellipse(_box((x - 8, y - 8, x + 8, y + 8)), fill=color, outline=(80, 66, 28), width=2 * SCALE)
    anchor = "rm" if side == "left" else "lm"
    dx = -18 if side == "left" else 18
    text(draw, (x + dx, y), label, 18, COLORS["black"], True, anchor)


def draw_rp2040(draw: ImageDraw.ImageDraw, base: Image.Image) -> dict[str, tuple[float, float]]:
    x, y, w, h = 700, 1330, 400, 760
    shadow_layer(base, (x, y, x + w, y + h), 34, 85)
    rounded(draw, (x, y, x + w, y + h), 34, COLORS["board"], COLORS["board_edge"], 5)
    rounded(draw, (x + 32, y + 42, x + w - 32, y + h - 42), 20, (31, 131, 101), None, 1)

    rounded(draw, (x + 130, y + 28, x + 270, y + 96), 18, COLORS["usb"], (104, 110, 114), 4)
    rounded(draw, (x + 158, y + 48, x + 242, y + 82), 9, (94, 99, 103), None, 1)
    text(draw, (x + w / 2, y + 128), "YD-RP2040", 32, COLORS["white"], True)
    text(draw, (x + w / 2, y + 164), "VCC-GND.COM", 19, (203, 234, 222), False)

    rounded(draw, (x + 126, y + 292, x + 274, y + 440), 12, COLORS["chip"], (10, 14, 18), 3)
    text(draw, (x + 200, y + 358), "RP2040", 27, COLORS["white"], True)
    text(draw, (x + 200, y + 394), "MCU", 20, (170, 183, 187), False)

    pins: dict[str, tuple[float, float]] = {}
    pin_colors = {
        "GP0": COLORS["orange"],
        "GP1": COLORS["orange"],
        "GP2": COLORS["cyan"],
        "GP3": COLORS["cyan"],
        "GP4": COLORS["green"],
        "GP5": COLORS["green"],
        "GP6": COLORS["orange"],
        "GP23": COLORS["yellow"],
        "3V3": COLORS["red"],
        "3V3_BOOT": COLORS["red"],
        "VIN": COLORS["red"],
        "VOUT": COLORS["red"],
    }
    start_y = y + 104
    step = 32
    for i, (key, label) in enumerate(LEFT_PIN_LAYOUT):
        py = start_y + i * step
        pins[key] = (x + 28, py)
        col = COLORS["black_wire"] if label == "GND" else pin_colors.get(key, COLORS["copper"])
        pin(draw, x + 28, py, label, "left", col)
    for i, (key, label) in enumerate(RIGHT_PIN_LAYOUT):
        py = start_y + i * step
        pins[key] = (x + w - 28, py)
        col = COLORS["black_wire"] if label == "GND" else pin_colors.get(key, COLORS["copper"])
        pin(draw, x + w - 28, py, label, "right", col)
    pins["GND_R"] = pins["GND_R1"]

    bottom_x = [x + 92, x + 162, x + 238, x + 308]
    for (key, label), px in zip(BOTTOM_PIN_LAYOUT, bottom_x):
        loc = (px, y + h - 68)
        pins[key] = loc
        col = COLORS["red"] if label == "3V3" else COLORS["black_wire"] if label == "GND" else COLORS["gray"]
        draw.ellipse(_box((loc[0] - 8, loc[1] - 8, loc[0] + 8, loc[1] + 8)), fill=col, outline=(80, 66, 28), width=2 * SCALE)
        text(draw, (loc[0], loc[1] - 24), label, 15, COLORS["black"], True)

    # Unused castellated pads visible in the reference photo.
    for loc, label in [((x + 104, y + 84), "VBUS"), ((x + 238, y + 112), "VSYS"), ((x + 90, y + 214), "GND"), ((x + 155, y + 214), "VREF")]:
        draw.ellipse(_box((loc[0] - 8, loc[1] - 8, loc[0] + 8, loc[1] + 8)), fill=COLORS["copper"], outline=(80, 66, 28), width=2 * SCALE)
        text(draw, (loc[0] + 17, loc[1]), label, 14, COLORS["black"], True, "lm")

    for px in (x + 58, x + w - 58):
        for py in (y + 64, y + h - 64):
            draw.ellipse(_box((px - 20, py - 20, px + 20, py + 20)), fill=(214, 210, 192), outline=(88, 92, 82), width=3 * SCALE)
            draw.ellipse(_box((px - 9, py - 9, px + 9, py + 9)), fill=COLORS["paper"])

    return pins


def draw_sensor_plate(draw: ImageDraw.ImageDraw, base: Image.Image) -> dict[str, dict[str, tuple[float, float]]]:
    cx, cy, radius = 900, 565, 300
    circle_shadow(base, (cx, cy), radius, 90)
    draw.ellipse(_box((cx - radius, cy - radius, cx + radius, cy + radius)), fill=COLORS["plate"], outline=COLORS["plate_edge"], width=6 * SCALE)
    draw.ellipse(_box((cx - 118, cy - 118, cx + 118, cy + 118)), fill=(81, 75, 184), outline=(63, 58, 150), width=4 * SCALE)
    for deg in range(0, 360, 45):
        a = math.radians(deg)
        px = cx + math.cos(a) * 238
        py = cy + math.sin(a) * 238
        rounded(draw, (px - 24, py - 10, px + 24, py + 10), 8, (90, 83, 196), COLORS["plate_edge"], 2)
    text(draw, (cx, cy - 22), "SENSOR PLATE", 34, COLORS["white"], True)
    text(draw, (cx, cy + 22), "3x TLV493D", 25, (226, 225, 246), False)

    modules = {
        "S1": (cx - 365, cy - 150, "TLV #1", "I2C0 A0", COLORS["green"]),
        "S2": (cx + 245, cy - 42, "TLV #2", "I2C0 A1", COLORS["green"]),
        "S3": (cx - 70, cy + 245, "TLV #3", "I2C1 A0", COLORS["cyan"]),
    }
    pin_map: dict[str, dict[str, tuple[float, float]]] = {}
    for key, (mx, my, title, subtitle, bus_color) in modules.items():
        shadow_layer(base, (mx, my, mx + 170, my + 112), 10, 60)
        rounded(draw, (mx, my, mx + 170, my + 112), 10, COLORS["sensor"], COLORS["sensor_edge"], 4)
        rounded(draw, (mx + 112, my + 28, mx + 148, my + 70), 5, COLORS["chip"], None, 1)
        text(draw, (mx + 58, my + 34), title, 24, COLORS["white"], True)
        text(draw, (mx + 58, my + 64), subtitle, 19, (200, 228, 245), False)
        pins = {
            "VDD": (mx + 170, my + 22),
            "GND": (mx + 170, my + 48),
            "SDA": (mx + 170, my + 74),
            "SCL": (mx + 170, my + 100),
        }
        for name, loc in pins.items():
            col = COLORS["red"] if name == "VDD" else COLORS["black_wire"] if name == "GND" else bus_color
            pin(draw, loc[0], loc[1], name, "right", col)
        pin_map[key] = pins
    return pin_map


def draw_button(draw: ImageDraw.ImageDraw, base: Image.Image, center: tuple[float, float], label: str) -> dict[str, tuple[float, float]]:
    cx, cy = center
    shadow_layer(base, (cx - 72, cy - 54, cx + 72, cy + 54), 10, 55)
    rounded(draw, (cx - 72, cy - 54, cx + 72, cy + 54), 10, COLORS["switch"], (0, 0, 0), 3)
    draw.ellipse(_box((cx - 36, cy - 38, cx + 36, cy + 34)), fill=(42, 42, 42), outline=(0, 0, 0), width=3 * SCALE)
    draw.ellipse(_box((cx - 25, cy - 45, cx + 25, cy + 5)), fill=(18, 18, 18), outline=(0, 0, 0), width=3 * SCALE)
    for px in (cx - 46, cx + 46):
        for py in (cy + 36, cy + 65):
            rounded(draw, (px - 9, py - 25, px + 9, py + 22), 3, COLORS["metal"], (116, 116, 112), 1)
    text(draw, (cx, cy - 78), label, 25, COLORS["black"], True)
    return {"SIG": (cx - 46, cy + 66), "GND": (cx + 46, cy + 66)}


def draw_neopixel(draw: ImageDraw.ImageDraw, base: Image.Image) -> dict[str, tuple[float, float]]:
    x, y = 1260, 1620
    shadow_layer(base, (x, y, x + 190, y + 135), 16, 55)
    rounded(draw, (x, y, x + 190, y + 135), 16, (43, 95, 66), (18, 65, 42), 4)
    draw.ellipse(_box((x + 62, y + 26, x + 128, y + 92)), fill=(245, 245, 230), outline=(180, 180, 170), width=4 * SCALE)
    draw.ellipse(_box((x + 80, y + 44, x + 110, y + 74)), fill=(238, 208, 68), outline=(188, 151, 26), width=2 * SCALE)
    text(draw, (x + 95, y + 116), "WS2812B", 23, COLORS["white"], True)
    pins = {"DIN": (x, y + 36), "VDD": (x, y + 68), "GND": (x, y + 100)}
    for name, loc in pins.items():
        col = COLORS["yellow"] if name == "DIN" else COLORS["red"] if name == "VDD" else COLORS["black_wire"]
        pin(draw, loc[0], loc[1], name, "left", col)
    return pins


def draw_legend(draw: ImageDraw.ImageDraw) -> None:
    x, y = 90, 2050
    rounded(draw, (x, y, x + 520, y + 170), 18, (255, 255, 252), (204, 199, 188), 3)
    text(draw, (x + 28, y + 34), "Signal colors", 26, COLORS["black"], True, "lm")
    rows = [
        (COLORS["red"], "3.3 V power"),
        (COLORS["black_wire"], "GND"),
        (COLORS["green"], "I2C0: GP4 SDA / GP5 SCL"),
        (COLORS["cyan"], "I2C1: GP2 SDA / GP3 SCL"),
        (COLORS["orange"], "Buttons: GP0 / GP1 / GP6"),
        (COLORS["yellow"], "NeoPixel data: GP23"),
    ]
    for i, (col, label) in enumerate(rows):
        yy = y + 67 + i * 18
        draw.line([_xy((x + 30, yy)), _xy((x + 92, yy))], fill=col, width=8 * SCALE)
        text(draw, (x + 112, yy), label, 17, COLORS["black"], False, "lm")


def draw_board_labels(draw: ImageDraw.ImageDraw) -> None:
    text(draw, (900, 1458), "YD-RP2040", 32, COLORS["white"], True)
    text(draw, (900, 1494), "VCC-GND.COM", 19, (203, 234, 222), False)
    text(draw, (900, 1710), "RP2040", 27, COLORS["white"], True)
    text(draw, (900, 1746), "MCU", 20, (170, 183, 187), False)


def draw_rp2040_pin_label_overlay(draw: ImageDraw.ImageDraw, pins: dict[str, tuple[float, float]]) -> None:
    for side, layout in (("left", LEFT_PIN_LAYOUT), ("right", RIGHT_PIN_LAYOUT)):
        for pin_name, label in layout:
            x, y = pins[pin_name]
            label_w = 122 if " " in label else 70
            if side == "left":
                box = (x - label_w - 18, y - 12, x - 13, y + 12)
                anchor = "rm"
                text_x = x - 18
            else:
                box = (x + 13, y - 12, x + label_w + 18, y + 12)
                anchor = "lm"
                text_x = x + 18
            rounded(draw, box, 5, (250, 248, 239), (184, 177, 160), 1)
            text(draw, (text_x, y), label, 15, COLORS["black"], True, anchor)

    for pin_name, label in BOTTOM_PIN_LAYOUT:
        x, y = pins[pin_name]
        label_w = 72 if label in ("SWCLK", "SWDIO") else 46
        box = (x - label_w / 2, y - 42, x + label_w / 2, y - 18)
        rounded(draw, box, 5, (250, 248, 239), (184, 177, 160), 1)
        text(draw, (x, y - 30), label, 14, COLORS["black"], True)



def render(output: str | Path | None = None) -> Path:
    out = Path(output) if output else Path(__file__).with_name("realistic_wiring_diagram.png")
    img = Image.new("RGBA", (CANVAS_W * SCALE, CANVAS_H * SCALE), COLORS["paper"] + (255,))
    draw = ImageDraw.Draw(img)

    for x in range(0, CANVAS_W, 46):
        draw.line([_xy((x, 0)), _xy((x, CANVAS_H))], fill=COLORS["paper_shadow"] + (42,), width=SCALE)
    for y in range(0, CANVAS_H, 46):
        draw.line([_xy((0, y)), _xy((CANVAS_W, y))], fill=COLORS["paper_shadow"] + (42,), width=SCALE)

    text(draw, (CANVAS_W / 2, 70), "HE-SpaceMouse RP2040 TLV493D wiring", 44, COLORS["black"], True)
    text(draw, (CANVAS_W / 2, 118), "Realistic layout based on physical wiring: controller, sensor plate, switches, and LED", 24, COLORS["muted"], False)

    sensor_pins = draw_sensor_plate(draw, img)
    board_pins = draw_rp2040(draw, img)
    buttons = {
        "BTN0": draw_button(draw, img, (280, 940), "BTN0"),
        "BTN1": draw_button(draw, img, (1515, 900), "BTN1"),
        "BTN2": draw_button(draw, img, (1280, 1125), "BTN2"),
    }
    led = draw_neopixel(draw, img)

    # Power and ground trunks mirror the hand-wired style of the reference image.
    wire(draw, [board_pins["3V3"], (555, 1220), (400, 970), sensor_pins["S1"]["VDD"]], COLORS["red"], 8, "3V3")
    wire(draw, [board_pins["3V3"], (650, 1185), (1040, 850), sensor_pins["S2"]["VDD"]], COLORS["red"], 8)
    wire(draw, [board_pins["3V3"], (690, 1188), (850, 900), sensor_pins["S3"]["VDD"]], COLORS["red"], 8)
    wire(draw, [board_pins["GND"], (520, 1290), (385, 1030), sensor_pins["S1"]["GND"]], COLORS["black_wire"], 8, "GND")
    wire(draw, [board_pins["GND"], (590, 1275), (1040, 900), sensor_pins["S2"]["GND"]], COLORS["black_wire"], 8)
    wire(draw, [board_pins["GND"], (660, 1270), (872, 940), sensor_pins["S3"]["GND"]], COLORS["black_wire"], 8)

    wire(draw, [board_pins["GP4"], (600, 1250), (500, 760), sensor_pins["S1"]["SDA"]], COLORS["green"], 8, "SDA0")
    wire(draw, [board_pins["GP5"], (565, 1270), (500, 795), sensor_pins["S1"]["SCL"]], COLORS["green"], 8, "SCL0")
    wire(draw, [board_pins["GP4"], (1060, 1250), (1190, 645), sensor_pins["S2"]["SDA"]], COLORS["green"], 8)
    wire(draw, [board_pins["GP5"], (1120, 1268), (1195, 670), sensor_pins["S2"]["SCL"]], COLORS["green"], 8)
    wire(draw, [board_pins["GP2"], (610, 1208), (820, 950), sensor_pins["S3"]["SDA"]], COLORS["cyan"], 8, "SDA1")
    wire(draw, [board_pins["GP3"], (640, 1225), (860, 1000), sensor_pins["S3"]["SCL"]], COLORS["cyan"], 8, "SCL1")

    wire(draw, [board_pins["GP0"], (1290, 1430), (1440, 1190), buttons["BTN1"]["SIG"]], COLORS["orange"], 8, "GP0")
    wire(draw, [board_pins["GP1"], (520, 1435), (330, 1190), buttons["BTN0"]["SIG"]], COLORS["orange"], 8, "GP1")
    wire(draw, [board_pins["GP6"], (1165, 1510), (1250, 1280), buttons["BTN2"]["SIG"]], COLORS["orange"], 8, "GP6")
    wire(draw, [board_pins["GND_R"], (1490, 1540), buttons["BTN1"]["GND"]], COLORS["black_wire"], 8)
    wire(draw, [board_pins["GND_R"], (1260, 1525), buttons["BTN2"]["GND"]], COLORS["black_wire"], 8)
    wire(draw, [board_pins["GND"], (350, 1550), buttons["BTN0"]["GND"]], COLORS["black_wire"], 8)

    wire(draw, [board_pins["GP23"], (1220, 1625), led["DIN"]], COLORS["yellow"], 8, "GP23")
    wire(draw, [board_pins["3V3"], (920, 1830), (1210, 1700), led["VDD"]], COLORS["red"], 7)
    wire(draw, [board_pins["GND_R"], (1185, 1800), led["GND"]], COLORS["black_wire"], 7)

    draw_board_labels(draw)
    draw_rp2040_pin_label_overlay(draw, board_pins)
    draw_legend(draw)
    text(draw, (CANVAS_W - 82, CANVAS_H - 52), "Generated from pio-migration/docs/gen_realistic_wiring_diagram.py", 20, COLORS["muted"], False, "rm")

    img = img.resize((CANVAS_W, CANVAS_H), Image.Resampling.LANCZOS).convert("RGB")
    out.parent.mkdir(parents=True, exist_ok=True)
    img.save(out, "PNG", optimize=True)
    return out


if __name__ == "__main__":
    print(f"Saved: {render()}")
