import tempfile
import unittest
from pathlib import Path
import sys

from PIL import Image, ImageStat

sys.path.insert(0, str(Path(__file__).resolve().parent))
import gen_realistic_wiring_diagram


class RealisticWiringDiagramTest(unittest.TestCase):
    def test_generator_uses_orthogonal_wires_and_complete_rp2040_labels(self):
        self.assertTrue(gen_realistic_wiring_diagram.ORTHOGONAL_WIRES)
        exposed_gpio = list(range(0, 24)) + [26, 27, 28, 29]
        for pin_number in exposed_gpio:
            self.assertIn(f"GP{pin_number}", gen_realistic_wiring_diagram.RP2040_PIN_LABELS)
        for power_pin in ("VOUT", "VIN", "3V3", "GND", "RUN", "SWCLK", "SWDIO"):
            self.assertIn(power_pin, gen_realistic_wiring_diagram.RP2040_PIN_LABELS)

    def test_rp2040_pin_layout_matches_attached_photo(self):
        left_labels = [label for _, label in gen_realistic_wiring_diagram.LEFT_PIN_LAYOUT]
        right_labels = [label for _, label in gen_realistic_wiring_diagram.RIGHT_PIN_LAYOUT]
        bottom_labels = [label for _, label in gen_realistic_wiring_diagram.BOTTOM_PIN_LAYOUT]

        self.assertEqual(
            left_labels,
            [
                "VOUT",
                "VIN",
                "GND",
                "GP23 LED",
                "3V3",
                "GP29",
                "GP28",
                "GND",
                "GP27",
                "GP26",
                "RUN",
                "GP22",
                "GND",
                "GP21",
                "GP20",
                "GP19",
                "GP18",
                "GND",
                "GP17",
                "GP16",
            ],
        )
        self.assertEqual(
            right_labels,
            [
                "GP0 BTN0",
                "GP1 BTN1",
                "GND",
                "GP2 SDA1",
                "GP3 SCL1",
                "GP4 SDA0",
                "GP5 SCL0",
                "GND",
                "GP6 BTN2",
                "GP7",
                "GP8",
                "GP9",
                "GND",
                "GP10",
                "GP11",
                "GP12",
                "GP13",
                "GND",
                "GP14",
                "GP15",
            ],
        )
        self.assertEqual(bottom_labels, ["GND", "SWCLK", "SWDIO", "3V3"])

    def test_render_creates_large_nonblank_png(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            output = Path(tmpdir) / "realistic_wiring_diagram.png"

            gen_realistic_wiring_diagram.render(output)

            self.assertTrue(output.exists())
            self.assertGreater(output.stat().st_size, 100_000)

            with Image.open(output) as img:
                self.assertEqual(img.format, "PNG")
                self.assertGreaterEqual(img.width, 1800)
                self.assertGreaterEqual(img.height, 1200)
                stat = ImageStat.Stat(img.convert("RGB"))
                self.assertGreater(max(stat.var), 500)


if __name__ == "__main__":
    unittest.main()
