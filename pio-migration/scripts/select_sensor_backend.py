import os

Import("env")

backend = os.environ.get("SPACEMOUSE_SENSOR_BACKEND", "hall").strip().lower()
tlv_count = os.environ.get("SPACEMOUSE_TLV493D_COUNT", "3").strip()

defines = env.Flatten(env.get("CPPDEFINES", []))
explicit_backend = any(
    (item == "SENSOR_BACKEND_HALL" or item == "SENSOR_BACKEND_TLV493D")
    or (isinstance(item, tuple) and item[0] in ("SENSOR_BACKEND_HALL", "SENSOR_BACKEND_TLV493D"))
    for item in defines
)

if explicit_backend:
    Return()

if backend in ("hall", "ss49e", "analog"):
    env.Append(CPPDEFINES=[("SENSOR_BACKEND_HALL", 1)])
elif backend in ("tlv493d", "tlv", "i2c"):
    if tlv_count not in ("1", "3"):
        raise ValueError("SPACEMOUSE_TLV493D_COUNT must be 1 or 3")
    env.Append(CPPDEFINES=[
        ("SENSOR_BACKEND_TLV493D", 1),
        ("TLV493D_SENSOR_COUNT", int(tlv_count)),
    ])
else:
    raise ValueError("SPACEMOUSE_SENSOR_BACKEND must be hall or tlv493d")
