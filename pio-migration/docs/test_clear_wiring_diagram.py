import sys
import tempfile
import unittest
from pathlib import Path

from PIL import Image, ImageStat

sys.path.insert(0, str(Path(__file__).resolve().parent))
import gen_clear_wiring_diagram


class ClearWiringDiagramTest(unittest.TestCase):
    def test_signal_lanes_are_separated_for_readability(self):
        lanes = gen_clear_wiring_diagram.SIGNAL_LANES

        self.assertEqual(len(lanes), len(set(lanes.values())))
        for first, second in zip(sorted(lanes.values()), sorted(lanes.values())[1:]):
            self.assertGreaterEqual(second - first, 55)

    def test_render_creates_large_nonblank_png(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            output = Path(tmpdir) / "clear_wiring_diagram.png"

            gen_clear_wiring_diagram.render(output)

            self.assertTrue(output.exists())
            self.assertGreater(output.stat().st_size, 100_000)
            with Image.open(output) as img:
                self.assertEqual(img.format, "PNG")
                self.assertGreaterEqual(img.width, 2400)
                self.assertGreaterEqual(img.height, 1600)
                stat = ImageStat.Stat(img.convert("RGB"))
                self.assertGreater(max(stat.var), 500)


if __name__ == "__main__":
    unittest.main()
