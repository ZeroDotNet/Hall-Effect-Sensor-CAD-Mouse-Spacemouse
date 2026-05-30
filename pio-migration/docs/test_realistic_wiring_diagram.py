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
        for pin_number in range(30):
            self.assertIn(f"GP{pin_number}", gen_realistic_wiring_diagram.RP2040_PIN_LABELS)
        for power_pin in ("3V3", "5V", "GND", "RUN"):
            self.assertIn(power_pin, gen_realistic_wiring_diagram.RP2040_PIN_LABELS)

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
