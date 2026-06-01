"""Render a visibility-first wiring matrix for the RP2040 TLV493D build."""

from __future__ import annotations

from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont

from gen_realistic_wiring_diagram import BOTTOM_PIN_LAYOUT, LEFT_PIN_LAYOUT, RIGHT_PIN_LAYOUT


CANVAS_W = 2400
CANVAS_H = 1700
SCALE = 2

SIGNAL_LANES = {
    "3V3": 260,
    "GND": 335,
    "I2C0_SDA": 450,
    "I2C0_SCL": 525,
    "I2C1_SDA": 640,
    "I2C1_SCL": 715,
    "BTN0": 870,
    "BTN1": 945,
    "BTN2": 1020,
    "LED": 1135,
}

COLORS = {
    "bg": (246, 244, 238),
    "grid": (214, 208, 195),
    "ink": (26, 28, 31),
    "muted": (89, 88, 83),
    "panel": (255, 254, 249),
    "board": (20, 105, 82),
    "board_edge": (9, 70, 55),
    "chip": (25, 33, 39),
    "sensor": (35, 82, 132),
    "sensor_edge": (17, 48, 84),
    "switch": (32, 32, 32),
    "led_board": (43, 96, 66),
    "red": (214, 44, 37),
    "ground": (18, 18, 18),
    "green": (23, 177, 70),
    "cyan": (16, 178, 177),
    "orange": (225, 126, 35),
    "yellow": (230, 181, 35),
    "gray": (116, 116, 110),
    "copper": (210, 174, 75),
    "white": (255, 255, 255),
}

NETS = [
    ("3V3", "3V3", "3V3", COLORS["red"], ["TLV #1 VDD", "TLV #2 VDD", "TLV #3 VDD", "WS2812B VDD"]),
    ("GND", "GND", "GND", COLORS["ground"], ["TLV #1 GND", "TLV #2 GND", "TLV #3 GND", "BTN0 GND", "BTN1 GND", "BTN2 GND", "WS2812B GND"]),
    ("I2C0_SDA", "GP4 SDA0", "SDA0", COLORS["green"], ["TLV #1 SDA", "TLV #2 SDA"]),
    ("I2C0_SCL", "GP5 SCL0", "SCL0", COLORS["green"], ["TLV #1 SCL", "TLV #2 SCL"]),
    ("I2C1_SDA", "GP2 SDA1", "SDA1", COLORS["cyan"], ["TLV #3 SDA"]),
    ("I2C1_SCL", "GP3 SCL1", "SCL1", COLORS["cyan"], ["TLV #3 SCL"]),
    ("BTN0", "GP1 BTN0", "BTN0", COLORS["orange"], ["BTN0 SIG"]),
    ("BTN1", "GP0 BTN1", "BTN1", COLORS["orange"], ["BTN1 SIG"]),
    ("BTN2", "GP6 BTN2", "BTN2", COLORS["orange"], ["BTN2 SIG"]),
    ("LED", "GP23 LED", "DIN", COLORS["yellow"], ["WS2812B DIN"]),
]


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


def xy(point: tuple[float, float]) -> tuple[int, int]:
    return round(point[0] * SCALE), round(point[1] * SCALE)


def box(bounds: tuple[float, float, float, float]) -> tuple[int, int, int, int]:
    return tuple(round(v * SCALE) for v in bounds)


def text(draw: ImageDraw.ImageDraw, point: tuple[float, float], label: str, size: int, color=COLORS["ink"], bold: bool = False, anchor: str = "mm") -> None:
    draw.text(xy(point), label, font=_font(size, bold), fill=color, anchor=anchor)


def rounded(draw: ImageDraw.ImageDraw, bounds: tuple[float, float, float, float], radius: int, fill, outline=None, width: int = 1) -> None:
    draw.rounded_rectangle(box(bounds), radius=radius * SCALE, fill=fill, outline=outline, width=width * SCALE)


def shadow(base: Image.Image, bounds: tuple[float, float, float, float], radius: int = 16, alpha: int = 54) -> None:
    layer = Image.new("RGBA", base.size, (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)
    d.rounded_rectangle(box((bounds[0] + 12, bounds[1] + 16, bounds[2] + 12, bounds[3] + 16)), radius=radius * SCALE, fill=(0, 0, 0, alpha))
    base.alpha_composite(layer.filter(ImageFilter.GaussianBlur(12 * SCALE)))


def draw_wire(draw: ImageDraw.ImageDraw, points: list[tuple[float, float]], color, width: int = 10, label: str | None = None) -> None:
    scaled = [xy(point) for point in points]
    draw.line(scaled, fill=COLORS["white"], width=(width + 12) * SCALE, joint="curve")
    draw.line(scaled, fill=(0, 0, 0, 70), width=(width + 5) * SCALE, joint="curve")
    draw.line(scaled, fill=color, width=width * SCALE, joint="curve")
    draw.line(scaled, fill=tuple(min(255, c + 44) for c in color), width=max(2, width // 3) * SCALE, joint="curve")
    if label:
        mid = points[len(points) // 2]
        rounded(draw, (mid[0] - 58, mid[1] - 18, mid[0] + 58, mid[1] + 18), 8, COLORS["panel"], color, 2)
        text(draw, mid, label, 17, color, True)


def draw_wire_tag(draw: ImageDraw.ImageDraw, point: tuple[float, float], label: str, color, width: float = 118) -> None:
    x, y = point
    rounded(draw, (x - width / 2, y - 19, x + width / 2, y + 19), 8, COLORS["panel"], color, 2)
    text(draw, (x, y), label, 17, color, True)


def dot(draw: ImageDraw.ImageDraw, point: tuple[float, float], color, radius: int = 8) -> None:
    x, y = point
    draw.ellipse(box((x - radius, y - radius, x + radius, y + radius)), fill=color, outline=(70, 58, 25), width=2 * SCALE)


def draw_pin_rows(draw: ImageDraw.ImageDraw, x: float, y: float, side: str, layout: tuple[tuple[str, str], ...]) -> None:
    for i, (_, label) in enumerate(layout):
        py = y + i * 25
        color = COLORS["ground"] if label == "GND" else COLORS["red"] if label in ("VOUT", "VIN", "3V3") else COLORS["yellow"] if label.startswith("GP23") else COLORS["copper"]
        dot(draw, (x, py), color, 5)
        if side == "left":
            text(draw, (x - 13, py), label, 11, COLORS["white"], True, "rm")
        else:
            text(draw, (x + 13, py), label, 11, COLORS["white"], True, "lm")


def draw_rp2040(draw: ImageDraw.ImageDraw, base: Image.Image) -> dict[str, tuple[float, float]]:
    x, y, w, h = 155, 205, 520, 1120
    shadow(base, (x, y, x + w, y + h), 28, 72)
    rounded(draw, (x, y, x + w, y + h), 28, COLORS["board"], COLORS["board_edge"], 5)
    rounded(draw, (x + 42, y + 50, x + w - 42, y + h - 50), 18, (28, 134, 101), None, 1)
    rounded(draw, (x + 180, y + 30, x + 340, y + 88), 14, (183, 190, 193), (102, 108, 112), 4)
    rounded(draw, (x + 216, y + 47, x + 304, y + 72), 7, (88, 94, 98), None, 1)
    text(draw, (x + w / 2, y + 135), "YD-RP2040", 31, COLORS["white"], True)
    text(draw, (x + w / 2, y + 170), "VCC-GND.COM", 18, (213, 236, 226))
    rounded(draw, (x + 182, y + 490, x + 338, y + 646), 12, COLORS["chip"], (5, 9, 12), 3)
    text(draw, (x + w / 2, y + 555), "RP2040", 28, COLORS["white"], True)
    text(draw, (x + w / 2, y + 594), "MCU", 20, (172, 183, 188))

    draw_pin_rows(draw, x + 36, y + 235, "left", LEFT_PIN_LAYOUT)
    draw_pin_rows(draw, x + w - 36, y + 235, "right", RIGHT_PIN_LAYOUT)
    for i, (_, label) in enumerate(BOTTOM_PIN_LAYOUT):
        px = x + 150 + i * 75
        py = y + h - 42
        dot(draw, (px, py), COLORS["red"] if label == "3V3" else COLORS["ground"] if label == "GND" else COLORS["gray"], 5)
        text(draw, (px, py + 26), label, 11, COLORS["white"], True)

    source_x = x + w + 100
    sources = {}
    for lane_key, source_label, _, color, _ in NETS:
        lane_y = SIGNAL_LANES[lane_key]
        rounded(draw, (x + w + 28, lane_y - 20, x + w + 162, lane_y + 20), 8, COLORS["panel"], color, 2)
        text(draw, (x + w + 95, lane_y), source_label, 15, color, True)
        dot(draw, (source_x, lane_y), color, 7)
        sources[lane_key] = (source_x, lane_y)
    return sources


def draw_card(draw: ImageDraw.ImageDraw, base: Image.Image, bounds: tuple[float, float, float, float], title: str, subtitle: str, fill, outline) -> None:
    shadow(base, bounds, 14, 42)
    rounded(draw, bounds, 14, fill, outline, 4)
    text(draw, ((bounds[0] + bounds[2]) / 2, bounds[1] + 34), title, 23, COLORS["white"], True)
    text(draw, ((bounds[0] + bounds[2]) / 2, bounds[1] + 66), subtitle, 16, (216, 232, 244))


def draw_destination_column(draw: ImageDraw.ImageDraw, base: Image.Image) -> dict[str, tuple[float, float]]:
    destinations: dict[str, tuple[float, float]] = {}

    component_specs = [
        ("TLV #1", "I2C0 A0", 1320, COLORS["sensor"], COLORS["sensor_edge"], [("VDD", "3V3"), ("GND", "GND"), ("SDA", "I2C0_SDA"), ("SCL", "I2C0_SCL")]),
        ("TLV #2", "I2C0 A1", 1515, COLORS["sensor"], COLORS["sensor_edge"], [("VDD", "3V3"), ("GND", "GND"), ("SDA", "I2C0_SDA"), ("SCL", "I2C0_SCL")]),
        ("TLV #3", "I2C1 A0", 1710, COLORS["sensor"], COLORS["sensor_edge"], [("VDD", "3V3"), ("GND", "GND"), ("SDA", "I2C1_SDA"), ("SCL", "I2C1_SCL")]),
        ("BTN0", "switch", 1905, COLORS["switch"], (0, 0, 0), [("SIG", "BTN0"), ("GND", "GND")]),
        ("BTN1", "switch", 2070, COLORS["switch"], (0, 0, 0), [("SIG", "BTN1"), ("GND", "GND")]),
        ("BTN2", "switch", 2235, COLORS["switch"], (0, 0, 0), [("SIG", "BTN2"), ("GND", "GND")]),
        ("WS2812B", "NeoPixel", 2235, COLORS["led_board"], (18, 66, 43), [("DIN", "LED"), ("VDD", "3V3"), ("GND", "GND")]),
    ]

    for title, subtitle, x, fill, outline, pins in component_specs:
        header_y = 145 if title != "WS2812B" else 1210
        draw_card(draw, base, (x - 80, header_y, x + 80, header_y + 84), title, subtitle, fill, outline)
        column_top = 235
        column_bottom = 1190 if title != "WS2812B" else 1260
        draw.line([xy((x, column_top)), xy((x, column_bottom))], fill=COLORS["grid"], width=2 * SCALE)
        for pin_label, lane_key in pins:
            y = SIGNAL_LANES[lane_key]
            color = next(net_color for net_key, _, _, net_color, _ in NETS if net_key == lane_key)
            dot(draw, (x, y), color, 8)
            rounded(draw, (x - 39, y + 13, x + 39, y + 39), 6, COLORS["panel"], color, 2)
            text(draw, (x, y + 26), pin_label, 13, color, True)
            destinations[f"{title} {pin_label}"] = (x, y)

    return destinations


def draw_pin_tag_overlay(draw: ImageDraw.ImageDraw) -> None:
    destination_tags = [
        ("3V3", COLORS["red"], (1098, SIGNAL_LANES["3V3"])),
        ("GND", COLORS["ground"], (1098, SIGNAL_LANES["GND"])),
        ("SDA0", COLORS["green"], (1098, SIGNAL_LANES["I2C0_SDA"])),
        ("SCL0", COLORS["green"], (1098, SIGNAL_LANES["I2C0_SCL"])),
        ("SDA1", COLORS["cyan"], (1098, SIGNAL_LANES["I2C1_SDA"])),
        ("SCL1", COLORS["cyan"], (1098, SIGNAL_LANES["I2C1_SCL"])),
        ("BTN0", COLORS["orange"], (1098, SIGNAL_LANES["BTN0"])),
        ("BTN1", COLORS["orange"], (1098, SIGNAL_LANES["BTN1"])),
        ("BTN2", COLORS["orange"], (1098, SIGNAL_LANES["BTN2"])),
        ("DIN", COLORS["yellow"], (1098, SIGNAL_LANES["LED"])),
    ]
    for label, color, point in destination_tags:
        draw_wire_tag(draw, point, label, color, 104)


def draw_legend(draw: ImageDraw.ImageDraw) -> None:
    x, y = 1080, 1340
    rounded(draw, (x, y, x + 520, y + 245), 14, COLORS["panel"], COLORS["grid"], 2)
    text(draw, (x + 28, y + 34), "Reading this diagram", 24, COLORS["ink"], True, "lm")
    rows = [
        "Each horizontal lane is one electrical net.",
        "White halos indicate intentional separation.",
        "RP2040 pin order matches the YD-RP2040 photo.",
        "Destination blocks show exact module pin names.",
        "Shared rails are shown once, then fanned out.",
    ]
    for i, row in enumerate(rows):
        text(draw, (x + 30, y + 76 + i * 31), row, 17, COLORS["muted"], False, "lm")


def render(output: str | Path | None = None) -> Path:
    out = Path(output) if output else Path(__file__).with_name("clear_wiring_diagram.png")
    img = Image.new("RGBA", (CANVAS_W * SCALE, CANVAS_H * SCALE), COLORS["bg"] + (255,))
    draw = ImageDraw.Draw(img)

    for x in range(0, CANVAS_W, 50):
        draw.line([xy((x, 0)), xy((x, CANVAS_H))], fill=COLORS["grid"] + (45,), width=SCALE)
    for y in range(0, CANVAS_H, 50):
        draw.line([xy((0, y)), xy((CANVAS_W, y))], fill=COLORS["grid"] + (45,), width=SCALE)

    text(draw, (CANVAS_W / 2, 54), "HE-SpaceMouse wiring - high visibility matrix", 42, COLORS["ink"], True)
    text(draw, (CANVAS_W / 2, 96), "One separated lane per net; no overlapping cable paths", 22, COLORS["muted"])

    sources = draw_rp2040(draw, img)
    destinations = draw_destination_column(draw, img)

    for lane_key, _, lane_label, color, destination_labels in NETS:
        y = SIGNAL_LANES[lane_key]
        source = sources[lane_key]
        endpoint_x = max(destinations[destination_label][0] for destination_label in destination_labels)
        draw_wire(draw, [source, (endpoint_x, y)], color, 10)

    for lane_key, source_label, lane_label, color, _ in NETS:
        y = SIGNAL_LANES[lane_key]
        draw_wire_tag(draw, (755, y), source_label, color, 126)
    draw_pin_tag_overlay(draw)
    draw_legend(draw)
    text(draw, (CANVAS_W - 72, CANVAS_H - 38), "Generated from pio-migration/docs/gen_clear_wiring_diagram.py", 18, COLORS["muted"], False, "rm")

    img = img.resize((CANVAS_W, CANVAS_H), Image.Resampling.LANCZOS).convert("RGB")
    out.parent.mkdir(parents=True, exist_ok=True)
    img.save(out, "PNG", optimize=True)
    return out


if __name__ == "__main__":
    print(f"Saved: {render()}")
