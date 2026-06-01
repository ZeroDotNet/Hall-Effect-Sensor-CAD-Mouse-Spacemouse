Import("env")

TIMEOUT = 30  # seconds to wait for bootloader port after manual reset

# PlatformIO 6.x
try:
    from platformio.upload import helpers as _h
    _orig = _h.wait_for_new_serial_port
    def _wait(before, timeout=TIMEOUT):
        return _orig(before, timeout)
    _h.wait_for_new_serial_port = _wait
except (ImportError, AttributeError):
    pass

# PlatformIO 5.x
try:
    from platformio.commands.run import helpers as _h
    _orig = _h.wait_for_new_serial_port
    def _wait(before, timeout=TIMEOUT):
        return _orig(before, timeout)
    _h.wait_for_new_serial_port = _wait
except (ImportError, AttributeError):
    pass
