"""Generate wiring diagram PNG for the HE-SpaceMouse / rp2040-tlv493d-3 build."""
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch
import matplotlib.patheffects as pe
import numpy as np
from pathlib import Path

# ── figure ────────────────────────────────────────────────────────────────────
FW, FH = 18, 11.5
fig, ax = plt.subplots(figsize=(FW, FH), dpi=100)
ax.set_xlim(0, FW); ax.set_ylim(0, FH)
ax.set_aspect('equal'); ax.axis('off')
fig.patch.set_facecolor('#0d1117')
ax.set_facecolor('#0d1117')

# grid dots
for gx in np.arange(0.3, FW, 0.6):
    for gy in np.arange(0.3, FH, 0.6):
        ax.plot(gx, gy, '.', color='#1a1f2e', ms=1.2, zorder=0)

# ── palette ───────────────────────────────────────────────────────────────────
P = dict(
    power ='#e74c3c', gnd='#636e72', i2c0='#2ecc71', i2c1='#1abc9c',
    gpio ='#f39c12', led='#f1c40f', warn='#ff6b6b', board='#161b2e',
    boardbdr='#4a9eff', comp='#111c30', compbdr='#5dade2', txt='#ecf0f1',
    dim='#636e72', bus='#2980b9', chip='#0a1220',
)

# ── helpers ───────────────────────────────────────────────────────────────────
def box(x, y, w, h, fc, ec, lw=1.5, r=0.15, glow=None, zorder=3):
    patch = FancyBboxPatch((x, y), w, h, boxstyle=f'round,pad=0,rounding_size={r}',
                           fc=fc, ec=ec, lw=lw, zorder=zorder)
    if glow:
        patch.set_path_effects([pe.withSimplePatchShadow(offset=(0,0),
                                shadow_rgbFace=glow, alpha=0.35, rho=1.3)])
    ax.add_patch(patch)

def label(s, x, y, color, size=9, ha='left', va='center', bold=False, zorder=5):
    w = 'bold' if bold else 'normal'
    ax.text(x, y, s, color=color, fontsize=size, ha=ha, va=va,
            fontfamily='monospace', fontweight=w, zorder=zorder)

def dot(x, y, color, r=0.07, zorder=6):
    ax.plot(x, y, 'o', color=color, ms=r*72*0.5, zorder=zorder, mew=0)

def wire(pts, color, lw=1.8, dashed=False, zorder=4):
    xs = [p[0] for p in pts]; ys = [p[1] for p in pts]
    ls = '--' if dashed else '-'
    ax.plot(xs, ys, color=color, lw=lw, ls=ls, solid_capstyle='round',
            dash_capstyle='round', zorder=zorder)

def hline(x1, x2, y, color, lw=1.8, dashed=False):
    wire([(x1,y),(x2,y)], color, lw, dashed)
def vline(x, y1, y2, color, lw=1.8, dashed=False):
    wire([(x,y1),(x,y2)], color, lw, dashed)

# ── title ─────────────────────────────────────────────────────────────────────
label('Diagrama de Conexión — HE-SpaceMouse', FW/2, FH-0.33,
      P['boardbdr'], 16, 'center', bold=True)
label('Entorno: rp2040-tlv493d-3  |  3× TLV493D-A1B6  |  3 botones  |  LED RGB WS2812B  |  USB HID VID:256F PID:C631',
      FW/2, FH-0.70, P['dim'], 9, 'center')

# ─────────────────────────────────────────────────────────────────────────────
# YD-RP2040 BOARD
# ─────────────────────────────────────────────────────────────────────────────
BX, BY, BW, BH = 6.5, 1.1, 3.0, 9.3
box(BX, BY, BW, BH, P['board'], P['boardbdr'], lw=2, r=0.2, glow='#4a9eff', zorder=2)
label('YD-RP2040', BX+BW/2, BY+BH-0.28, P['boardbdr'], 13, 'center', bold=True)
label('VCC-GND.COM', BX+BW/2, BY+BH-0.55, P['dim'], 8, 'center')

# RP2040 chip block
cx, cy, cw, ch = BX+0.6, BY+3.8, 1.8, 1.4
box(cx, cy, cw, ch, P['chip'], P['boardbdr'], lw=1.2, r=0.1, zorder=4)
label('RP2040', cx+cw/2, cy+ch/2+0.15, P['boardbdr'], 11, 'center', bold=True, zorder=5)
label('Cortex-M0+  125MHz', cx+cw/2, cy+ch/2-0.18, P['dim'], 7.5, 'center', zorder=5)

# USB-C symbol
ux, uy, uw, uh = BX+BW/2-0.45, BY+0.18, 0.9, 0.45
box(ux, uy, uw, uh, P['chip'], P['boardbdr'], lw=1.2, r=0.08, zorder=4)
label('USB-C', ux+uw/2, uy+uh/2, P['boardbdr'], 7.5, 'center', zorder=5)

# ─────────────────────────────────────────────────────────────────────────────
# PIN POSITIONS  (absolute y, all on board edges)
# Left edge x = BX,  Right edge x = BX+BW
# ─────────────────────────────────────────────────────────────────────────────
PL = dict(         # left-edge pins
    v33 = BY+BH-0.70,
    gnd = BY+BH-1.20,
    gnd2= BY+BH-1.70,
    gp2 = BY+5.45,    # I2C1 SDA
    gp3 = BY+4.85,    # I2C1 SCL
    gp4 = BY+3.75,    # I2C0 SDA
    gp5 = BY+3.15,    # I2C0 SCL
)
PR = dict(         # right-edge pins
    gp0 = BY+7.20,    # BTN0
    gp1 = BY+6.20,    # BTN1
    gp6 = BY+5.20,    # BTN2
    gp23= BY+2.40,    # NeoPixel
)

def pin_left(key, pin_label, wire_color, desc, warn=False):
    y = PL[key]; x = BX
    dot(x, y, wire_color)
    hline(x, x-0.22, y, wire_color, 1.5)
    col = P['warn'] if warn else P['txt']
    label(pin_label, x-0.26, y, col, 8.5, 'right', bold=warn)
    dcolor = P['warn'] if warn else P['dim']
    label(desc, x+0.10, y, dcolor, 7.5, 'left', bold=warn)

def pin_right(key, pin_label, wire_color, desc):
    y = PR[key]; x = BX+BW
    dot(x, y, wire_color)
    hline(x, x+0.22, y, wire_color, 1.5)
    label(pin_label, x+0.26, y, P['txt'], 8.5, 'left')
    label(desc, x-0.10, y, P['dim'], 7.5, 'right')

pin_left('v33',  '3V3',  P['power'],  '3.3 V')
pin_left('gnd',  'GND',  P['gnd'],    'Ground')
pin_left('gnd2', 'GND',  P['gnd'],    'Ground')
pin_left('gp2',  'GP2',  P['i2c1'],   'I2C1 SDA')
pin_left('gp3',  'GP3',  P['i2c1'],   'I2C1 SCL')
pin_left('gp4',  'GP4',  P['i2c0'],   'I2C0 SDA')
pin_left('gp5',  'GP5',  P['i2c0'],   'I2C0 SCL')

pin_right('gp0',  'GP0',  P['gpio'], 'BTN0')
pin_right('gp1',  'GP1',  P['gpio'], 'BTN1')
pin_right('gp6',  'GP6',  P['gpio'], 'BTN2')
pin_right('gp23', 'GP23', P['led'],  'NeoPixel')

# ─────────────────────────────────────────────────────────────────────────────
# TLV493D SENSORS
# ─────────────────────────────────────────────────────────────────────────────
SW, SH = 2.05, 1.35

def draw_sensor(sx, sy, sid, bus_label, bus_color, addr):
    box(sx, sy, SW, SH, P['comp'], bus_color, lw=1.5, r=0.12, glow=bus_color, zorder=3)
    label(f'TLV493D-A1B6  #{sid}', sx+SW/2, sy+SH-0.22, bus_color, 9.5, 'center', bold=True)
    label(bus_label, sx+SW/2, sy+SH-0.46, P['dim'], 7.5, 'center')
    label(f'Addr: {addr}', sx+SW/2, sy+SH-0.63, P['dim'], 7.5, 'center')
    # pins on right edge
    pins = [('VDD', P['power']), ('GND', P['gnd']),
            ('SDA', bus_color),  ('SCL', bus_color)]
    py0 = sy + SH - 0.90
    pys = {}
    for i, (n, c) in enumerate(pins):
        py = py0 - i*0.26
        dot(sx+SW, py, c, 0.065)
        label(n, sx+SW-0.08, py, c, 7.5, 'right')
        pys[n] = py
    return pys

# Sensor 1 – I2C0, address A0
S1x, S1y = 0.25, BY+6.55
p1 = draw_sensor(S1x, S1y, 1, 'Wire · I2C0', P['i2c0'], '0x5E  (ADDR pin → GND)')

# Sensor 2 – I2C0, address A1
S2x, S2y = 0.25, BY+4.85
p2 = draw_sensor(S2x, S2y, 2, 'Wire · I2C0', P['i2c0'], '0x1F  (ADDR pin → VDD)')

# Sensor 3 – I2C1, address A0
S3x, S3y = 0.25, BY+3.15
p3 = draw_sensor(S3x, S3y, 3, 'TlvWire1 · I2C1', P['i2c1'], '0x5E  (ADDR pin → GND)')

# ─────────────────────────────────────────────────────────────────────────────
# I2C0 BUS  (sensors 1 & 2  →  GP4/GP5)
# ─────────────────────────────────────────────────────────────────────────────
BUS0X_SDA = 4.90   # vertical bus x for SDA
BUS0X_SCL = 4.68   # slightly offset for SCL

# SDA  S1 → bus → S2 → bus → GP4
wire([(S1x+SW, p1['SDA']), (BUS0X_SDA, p1['SDA'])], P['i2c0'])
wire([(S2x+SW, p2['SDA']), (BUS0X_SDA, p2['SDA'])], P['i2c0'])
vline(BUS0X_SDA, p2['SDA'], p1['SDA'], P['i2c0'])
wire([(BUS0X_SDA, PL['gp4']), (BX, PL['gp4'])], P['i2c0'])
vline(BUS0X_SDA, PL['gp4'], p2['SDA'], P['i2c0'])
dot(BUS0X_SDA, p2['SDA'], P['i2c0'], 0.08)  # junction

# SCL  S1 → bus → S2 → bus → GP5
wire([(S1x+SW, p1['SCL']), (BUS0X_SCL, p1['SCL'])], P['i2c0'])
wire([(S2x+SW, p2['SCL']), (BUS0X_SCL, p2['SCL'])], P['i2c0'])
vline(BUS0X_SCL, p2['SCL'], p1['SCL'], P['i2c0'])
wire([(BUS0X_SCL, PL['gp5']), (BX, PL['gp5'])], P['i2c0'])
vline(BUS0X_SCL, PL['gp5'], p2['SCL'], P['i2c0'])
dot(BUS0X_SCL, p2['SCL'], P['i2c0'], 0.08)

# I2C0 bus label
label('I2C0', BUS0X_SDA+0.05, (PL['gp4']+PL['gp5'])/2, P['i2c0'], 7.5, 'left')

# ─────────────────────────────────────────────────────────────────────────────
# I2C1 BUS  (sensor 3  →  GP2/GP3)
# ─────────────────────────────────────────────────────────────────────────────
# SDA: S3 → GP2
wire([(S3x+SW, p3['SDA']), (BX, PL['gp2'])], P['i2c1'])
# SCL: S3 → GP3
wire([(S3x+SW, p3['SCL']), (BUS0X_SCL-0.12, p3['SCL']),
      (BUS0X_SCL-0.12, PL['gp3']), (BX, PL['gp3'])], P['i2c1'])

label('I2C1', S3x+SW+0.08, (p3['SDA']+p3['SCL'])/2, P['i2c1'], 7.5, 'left')

# ─────────────────────────────────────────────────────────────────────────────
# POWER RAILS  (left side)
# ─────────────────────────────────────────────────────────────────────────────
RAIL3V = 0.38   # x for 3.3V rail
RAILGND = 0.60  # x for GND rail
RAIL_Y_TOP = p1['VDD'] + 0.15
RAIL_Y_BOT = p3['GND'] - 0.15

# 3.3V rail (vertical dashed)
vline(RAIL3V, RAIL_Y_BOT, RAIL_Y_TOP, P['power'], 1.2, dashed=True)
label('+3.3V', RAIL3V, RAIL_Y_TOP+0.15, P['power'], 7.5, 'center')
label('+3.3V', RAIL3V, RAIL_Y_BOT-0.15, P['power'], 7.5, 'center')
dot(RAIL3V, RAIL_Y_TOP, P['power'], 0.055)
dot(RAIL3V, RAIL_Y_BOT, P['power'], 0.055)

# GND rail
vline(RAILGND, RAIL_Y_BOT, RAIL_Y_TOP, P['gnd'], 1.2, dashed=True)
label('GND', RAILGND, RAIL_Y_BOT-0.15, P['gnd'], 7.5, 'center')

# connect sensor VDD/GND to rails
for p, sx in [(p1, S1x), (p2, S2x), (p3, S3x)]:
    wire([(sx+SW, p['VDD']), (RAIL3V, p['VDD'])], P['power'], 1.2)
    dot(RAIL3V, p['VDD'], P['power'], 0.055)
    wire([(sx+SW, p['GND']), (RAILGND, p['GND'])], P['gnd'], 1.2)
    dot(RAILGND, p['GND'], P['gnd'], 0.055)

# connect rail to board 3V3 and GND
wire([(RAIL3V, RAIL_Y_TOP), (RAIL3V, PL['v33']), (BX, PL['v33'])], P['power'], 1.2)
wire([(RAILGND, RAIL_Y_TOP), (RAILGND, PL['gnd']-0.1), (BX, PL['gnd']-0.1),
      (BX, PL['gnd'])], P['gnd'], 1.2)
wire([(RAILGND, RAIL_Y_TOP), (RAILGND+0.1, PL['gnd2']-0.1), (BX, PL['gnd2'])], P['gnd'], 1.2)

# ─────────────────────────────────────────────────────────────────────────────
# BUTTONS  (right side)
# ─────────────────────────────────────────────────────────────────────────────
BTN_X = BX+BW+0.55
BTN_W, BTN_H = 1.85, 0.72

def draw_button(bid, y_center, gp_key):
    bx2 = BTN_X; by2 = y_center - BTN_H/2
    col = P['gpio']
    box(bx2, by2, BTN_W, BTN_H, P['comp'], col, lw=1.5, r=0.1, zorder=3)
    label(f'BOTÓN  {bid}', bx2+BTN_W/2, by2+BTN_H-0.22, col, 9, 'center', bold=True)
    gp_y = PR[f'gp{bid}'] if bid < 2 else PR['gp6']
    # left pin of button
    lpin_y = by2+BTN_H/2
    dot(bx2, lpin_y, col, 0.065)
    # GND on right
    dot(bx2+BTN_W, lpin_y, P['gnd'], 0.065)
    return lpin_y

b0_y = draw_button(0, PR['gp0'], 'gp0')
b1_y = draw_button(1, PR['gp1'], 'gp1')

b2_y = draw_button(2, PR['gp6'], 'gp6')

# GND rail right side
RGND_X = BTN_X + BTN_W + 0.20
RGND_Y0 = b2_y - 0.4
RGND_Y1 = b0_y + 0.4
vline(RGND_X, RGND_Y0, RGND_Y1, P['gnd'], 1.2, dashed=True)
label('GND', RGND_X, RGND_Y1+0.15, P['gnd'], 7.5, 'center')
for y_ in [b0_y, b1_y, b2_y]:
    wire([(BTN_X+BTN_W, y_), (RGND_X, y_)], P['gnd'], 1.2)
    dot(RGND_X, y_, P['gnd'], 0.055)

# Wires from board to buttons
wire([(BX+BW, PR['gp0']), (BTN_X, b0_y)], P['gpio'], 1.8)
wire([(BX+BW, PR['gp1']), (BTN_X, b1_y)], P['gpio'], 1.8)
wire([(BX+BW, PR['gp6']), (BTN_X, b2_y)], P['gpio'], 1.8)

label('(INPUT_PULLUP)', BTN_X+BTN_W+0.25, (b0_y+b1_y)/2, P['dim'], 7.5, 'left')

# ─────────────────────────────────────────────────────────────────────────────
# WS2812B NEOPIXEL  (right side, lower)
# ─────────────────────────────────────────────────────────────────────────────
NPX, NPY, NPW, NPH = BTN_X - 0.15, BY + 1.55, 2.25, 1.55
box(NPX, NPY, NPW, NPH, P['comp'], P['led'], lw=2, r=0.12, glow=P['led'], zorder=3)
label('WS2812B  NeoPixel', NPX+NPW/2, NPY+NPH-0.24, P['led'], 10, 'center', bold=True)
label('LED RGB integrado — YD-RP2040', NPX+NPW/2, NPY+NPH-0.48, P['dim'], 7.5, 'center')
label('GP23  (STATUS_LED_PIN = 23)', NPX+NPW/2, NPY+NPH-0.68, P['led'], 7.5, 'center')

np_pins = [('DIN', P['led'], NPY+0.65), ('VDD', P['power'], NPY+0.42),
           ('GND', P['gnd'], NPY+0.20)]
for (n, c, py_) in np_pins:
    dot(NPX, py_, c, 0.065)
    label(n, NPX+0.10, py_, c, 7.5, 'left')
    dot(NPX+NPW, py_, c, 0.055)

# Wire: board GP23 → NeoPixel DIN
din_y = NPY + 0.65
wire([(BX+BW, PR['gp23']), (BX+BW+0.35, PR['gp23']),
      (BX+BW+0.35, din_y),  (NPX, din_y)], P['led'], 1.8)

# Power to NeoPixel from rail
wire([(RAIL3V+0.05, p3['VDD']-0.40), (RAIL3V+0.05, NPY+0.42),
      (NPX, NPY+0.42)], P['power'], 1.2)
wire([(RAILGND+0.05, p3['GND']-0.40), (RAILGND+0.05, NPY+0.20),
      (NPX, NPY+0.20)], P['gnd'], 1.2)

# LED state box
lsX, lsY, lsW, lsH = NPX+NPW+0.22, NPY, 2.40, 1.55
box(lsX, lsY, lsW, lsH, '#0a120a', P['i2c0'], lw=1.2, r=0.1, zorder=3)
label('Estados del LED:', lsX+lsW/2, lsY+lsH-0.24, P['i2c0'], 8.5, 'center', bold=True)
states = [('[OFF]   Apagado',  'USB no conectado'),
          ('[ON ]   Azul fijo', 'USB listo / operando'),
          ('[~~~]   Azul pulso','Calibración (~1 s)')]
for i, (st, sd) in enumerate(states):
    sy2 = lsY + lsH - 0.52 - i*0.36
    label(st, lsX+0.15, sy2, P['txt'], 8, 'left')
    label(sd, lsX+0.15, sy2-0.17, P['dim'], 7, 'left')

# ─────────────────────────────────────────────────────────────────────────────
# BTN2 FIX NOTE
# ─────────────────────────────────────────────────────────────────────────────
wbX, wbY, wbW, wbH = BTN_X+2.25, BY+BH-2.05, BTN_W+2.85, 1.05
box(wbX, wbY, wbW, wbH, '#08140d', P['gpio'], lw=1.5, r=0.12, glow=P['gpio'], zorder=3)
label('BTN2 reasignado a GP6', wbX+wbW/2, wbY+wbH-0.25,
      P['gpio'], 9.5, 'center', bold=True)
label('GP2 queda dedicado a I2C1_SDA para el sensor 3.',
      wbX+wbW/2, wbY+wbH-0.55, P['txt'], 7.5, 'center')
label('Botones: GP0, GP1, GP6 con INPUT_PULLUP.',
      wbX+wbW/2, wbY+wbH-0.75, P['txt'], 7.5, 'center')

# ─────────────────────────────────────────────────────────────────────────────
# LEGEND  (bottom-left)
# ─────────────────────────────────────────────────────────────────────────────
legX, legY, legW, legH = 0.25, 0.15, 5.80, 2.0
box(legX, legY, legW, legH, P['comp'], P['dim'], lw=1, r=0.12, zorder=3)
label('Leyenda de señales', legX+legW/2, legY+legH-0.22, P['txt'], 9, 'center', bold=True)
leg = [(P['power'],'───  3.3 V (alimentación)'),
       (P['gnd'],  '───  GND  (tierra / masa)'),
       (P['i2c0'], '───  I2C0 – GP4 SDA / GP5 SCL  →  Sensores 1 & 2'),
       (P['i2c1'], '───  I2C1 – GP2 SDA / GP3 SCL  →  Sensor 3'),
       (P['gpio'], '───  GPIO digital  INPUT_PULLUP  →  Botones 0, 1 & 2'),
       (P['led'],  '───  GP23 data  →  WS2812B NeoPixel (LED RGB)')]
for i, (col, txt_) in enumerate(leg):
    ly = legY + legH - 0.50 - i*0.22
    ax.plot([legX+0.15, legX+0.52], [ly, ly],
            color=col, lw=2)
    dot(legX+0.52, ly, col, 0.055)
    label(txt_, legX+0.62, ly, col, 7.5, 'left')

# ─────────────────────────────────────────────────────────────────────────────
# PIN MAP TABLE  (bottom-center)
# ─────────────────────────────────────────────────────────────────────────────
pmX, pmY, pmW, pmH = 6.3, 0.15, 5.55, 2.0
box(pmX, pmY, pmW, pmH, P['comp'], P['dim'], lw=1, r=0.12, zorder=3)
label('Mapa de pines  (rp2040-tlv493d-3)', pmX+pmW/2, pmY+pmH-0.22,
      P['txt'], 9, 'center', bold=True)
rows = [('GP0',  P['gpio'],  'BTN0 – pulsador derecho',          'INPUT_PULLUP'),
        ('GP1',  P['gpio'],  'BTN1 – pulsador izquierdo',        'INPUT_PULLUP'),
        ('GP2',  P['i2c1'],  'I2C1 SDA – TLV493D #3',             'I2C1'),
        ('GP3',  P['i2c1'], 'I2C1 SCL – TLV493D #3',             'I2C1'),
        ('GP4',  P['i2c0'], 'I2C0 SDA – TLV493D #1 & #2',        'I2C0'),
        ('GP5',  P['i2c0'], 'I2C0 SCL – TLV493D #1 & #2',        'I2C0'),
        ('GP6',  P['gpio'], 'BTN2 – pulsador extra',             'INPUT_PULLUP'),
        ('GP23', P['led'],  'NeoPixel WS2812B LED RGB',           'OUTPUT')]
for i, (pin, col, desc_, mode) in enumerate(rows):
    ry = pmY + pmH - 0.46 - i*0.20
    label(f'{pin}:', pmX+0.15, ry, col, 8, 'left', bold=True)
    label(desc_, pmX+0.72, ry, P['txt'], 7.5, 'left')
    label(mode, pmX+pmW-0.12, ry, col, 7, 'right')

# ─────────────────────────────────────────────────────────────────────────────
# SENSOR PLACEMENT NOTE  (bottom-right)
# ─────────────────────────────────────────────────────────────────────────────
spX, spY, spW, spH = 12.1, 0.15, 5.65, 2.0
box(spX, spY, spW, spH, P['comp'], P['dim'], lw=1, r=0.12, zorder=3)
label('Posición física de sensores (SpaceMouse)', spX+spW/2, spY+spH-0.22,
      P['txt'], 9, 'center', bold=True)
note_rows = [
  (P['i2c0'], 'Sensor #1 (I2C0, A0) →  eje Y−   posición 6 en punto'),
  (P['i2c0'], 'Sensor #2 (I2C0, A1) →  eje X+   posición 3 en punto'),
  (P['i2c1'], 'Sensor #3 (I2C1, A0) →  eje Y+   posición 12 en punto'),
  (P['dim'],  'Imanes sobre membrana flexible / resorte'),
  (P['dim'],  'TLV493D mide campo magnético 3D (X, Y, Z)'),
  (P['dim'],  'Fusiona → 6 DoF: Tx Ty Tz Rx Ry Rz'),
]
for i, (col, txt_) in enumerate(note_rows):
    ry = spY + spH - 0.50 - i*0.22
    label(txt_, spX+0.18, ry, col, 7.5, 'left')

# stamp
label('HE-SpaceMouse / pio-migration  –  rp2040-tlv493d-3',
      FW-0.12, 0.10, P['dim'], 7.5, 'right')

# ─────────────────────────────────────────────────────────────────────────────
out = Path(__file__).with_name('wiring_diagram.png')
fig.savefig(out, dpi=120, bbox_inches='tight',
            facecolor=fig.get_facecolor(), edgecolor='none')
print(f'Saved: {out}')
plt.close(fig)
