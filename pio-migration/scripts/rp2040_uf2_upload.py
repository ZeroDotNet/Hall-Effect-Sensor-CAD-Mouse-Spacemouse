import ctypes
import os
import shutil
import string
import sys
import time
from pathlib import Path

from serial import Serial
from serial import SerialException
from serial.tools import list_ports


BOOT_LABEL = "RPI-RP2"
POLL_INTERVAL_SECONDS = 0.25
DEFAULT_TIMEOUT_SECONDS = 15.0
RESET_BAUDRATE = 1200


def _windows_boot_drives():
    kernel32 = ctypes.windll.kernel32
    drives_bitmask = kernel32.GetLogicalDrives()
    if drives_bitmask == 0:
        return []

    volume_name = ctypes.create_unicode_buffer(261)
    filesystem_name = ctypes.create_unicode_buffer(261)
    serial_number = ctypes.c_uint()
    max_component_length = ctypes.c_uint()
    flags = ctypes.c_uint()
    drives = []

    for index, letter in enumerate(string.ascii_uppercase):
        if not (drives_bitmask & (1 << index)):
            continue

        root_path = f"{letter}:\\"
        success = kernel32.GetVolumeInformationW(
            ctypes.c_wchar_p(root_path),
            volume_name,
            len(volume_name),
            ctypes.byref(serial_number),
            ctypes.byref(max_component_length),
            ctypes.byref(flags),
            filesystem_name,
            len(filesystem_name),
        )
        if success and volume_name.value == BOOT_LABEL:
            drives.append(Path(root_path))

    return drives


def _posix_boot_drives():
    roots = [Path("/Volumes"), Path("/media"), Path("/run/media")]
    drives = []
    for root in roots:
        if not root.exists():
            continue
        for path in root.rglob("*"):
            if path.is_dir() and path.name == BOOT_LABEL:
                drives.append(path)
    return drives


def find_boot_drive(timeout_seconds):
    deadline = time.monotonic() + timeout_seconds
    while time.monotonic() < deadline:
        drives = _windows_boot_drives() if os.name == "nt" else _posix_boot_drives()
        if drives:
            return drives[0]
        time.sleep(POLL_INTERVAL_SECONDS)
    return None


def _candidate_serial_ports():
    preferred_vid_pid_pairs = {
        (0x2E8A, 0x0003),
        (0x2E8A, 0x000A),
        (0x256F, 0xC631),
    }
    preferred = []
    fallback = []

    for port in list_ports.comports():
        if not port.device:
            continue
        vid_pid = (port.vid, port.pid)
        if vid_pid in preferred_vid_pid_pairs:
            preferred.append(port.device)
        else:
            fallback.append(port.device)

    return preferred + fallback


def _touch_serial_port(port_name):
    print(f"Forcing reset using {RESET_BAUDRATE}bps open/close on port {port_name}")
    try:
        with Serial(port=port_name, baudrate=RESET_BAUDRATE) as serial_port:
            serial_port.dtr = False
    except SerialException as exc:
        print(
            f"Could not open {port_name} for the 1200bps reset touch: {exc}. "
            "Close any serial monitor or Teleplot bridge using that port, then retry."
            ,
            file=sys.stderr,
        )
        raise SystemExit(1) from exc


def maybe_reset_to_bootsel(upload_port):
    if upload_port:
        _touch_serial_port(upload_port)
        return

    candidates = _candidate_serial_ports()
    if len(candidates) == 1:
        _touch_serial_port(candidates[0])
        return

    if len(candidates) > 1:
        joined = ", ".join(candidates)
        print(
            "Multiple serial ports detected. Pass --upload-port to choose one: "
            f"{joined}",
            file=sys.stderr,
        )
        raise SystemExit(1)

    print(
        f"No serial port detected. Waiting for the {BOOT_LABEL} boot drive in case the board is already in BOOTSEL mode."
    )


def main():
    if len(sys.argv) not in (2, 3):
        print("Usage: rp2040_uf2_upload.py <firmware.uf2> [upload_port]", file=sys.stderr)
        return 2

    firmware_path = Path(sys.argv[1]).resolve()
    if not firmware_path.is_file():
        print(f"UF2 file not found: {firmware_path}", file=sys.stderr)
        return 1

    upload_port = sys.argv[2].strip() if len(sys.argv) == 3 else ""
    if upload_port.startswith("$"):
        upload_port = ""

    maybe_reset_to_bootsel(upload_port)

    timeout_seconds = float(os.environ.get("RP2040_UF2_TIMEOUT", DEFAULT_TIMEOUT_SECONDS))
    boot_drive = find_boot_drive(timeout_seconds)
    if boot_drive is None:
        print(
            f"Timed out waiting for the {BOOT_LABEL} boot drive. "
            "Press BOOTSEL and reconnect the board, or retry the upload.",
            file=sys.stderr,
        )
        return 1

    destination = boot_drive / firmware_path.name
    print(f"Detected {BOOT_LABEL} at {boot_drive}")
    print(f"Copying {firmware_path.name} to {destination}")
    shutil.copy2(firmware_path, destination)
    print("UF2 upload finished")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())