# Armado del Circuito — Backend TLV493D (Sensor Magnetico I2C)

Plataforma target: **YD-RP2040** con 1 o 3 sensores TLV493D via I2C.  
Esta variante usa sensores magneticos 3D de Infineon que detectan los 3 ejes del campo magnetico por sensor, lo que permite mayor precision y menos sensores fisicos.

> **Nota:** El backend TLV493D solo funciona en RP2040 (requiere Mbed I2C). No compatible con Arduino Pro Micro ni ESP32-S3 en este firmware.

---

## Diferencias clave respecto a Hall Effect

| Aspecto | Hall Effect | TLV493D |
|---|---|---|
| Sensores necesarios | 4 analogicos | 1 o 3 I2C |
| Pines usados | GP26-GP29 (ADC) | GP4/GP5 (I2C) |
| Campos detectados | 1 eje por sensor | 3 ejes por sensor (Bx, By, Bz) |
| Precision | Media | Alta |
| Cableado | Mas simple | Mas compacto |
| Entorno PlatformIO | `rp2040` | `rp2040-tlv493d-1` o `rp2040-tlv493d-3` |

---

## Paso 0 — Componentes necesarios

### Opcion A: 1 sensor TLV493D (6-DOF limitado, solo para pruebas)

| # | Componente | Cantidad |
|---|---|---|
| 1 | YD-RP2040 | 1 |
| 2 | TLV493D-A1B6 (breakout o modulo) | 1 |
| 3 | Iman de neodimio cilindrico (~6mm) | 1 |
| 4 | Puck/knob mecanico | 1 |
| 5 | Pulsadores momentaneos | 3 |
| 6 | Cables Dupont M-F | varios |
| 7 | Protoboard o PCB | 1 |
| 8 | Cable USB-C | 1 |

### Opcion B: 3 sensores TLV493D (6-DOF completo, recomendado)

Igual que Opcion A pero con **3x TLV493D** (uno por eje de rotacion).

---

## Paso 1 — Pinout del YD-RP2040 para TLV493D

```
YD-RP2040 — pines del proyecto (TLV493D)
+----------------------------------------------+
|  GP4  (SDA0) -> TLV493D sensor 1 SDA         |
|  GP5  (SCL0) -> TLV493D sensor 1 SCL         |
|  GP4  (SDA0) -> TLV493D sensor 2 SDA (mismo) |
|  GP5  (SCL0) -> TLV493D sensor 2 SCL (mismo) |
|                                              |
|  GP2  (SDA1) -> TLV493D sensor 3 SDA (I2C1) |
|  GP3  (SCL1) -> TLV493D sensor 3 SCL (I2C1) |
|                                              |
|  GP0  -> BTN0 (boton izquierdo)              |
|  GP1  -> BTN1 (boton derecho)                |
|  GP6  -> BTN2 (boton frontal/centro)         |
|                                              |
|  GP23 -> LED RGB WS2812B (INTERNO)           |
|  3V3  -> VCC de sensores                     |
|  GND  -> GND comun                           |
+----------------------------------------------+
```

---

## Paso 2 — Direcciones I2C de los sensores

El TLV493D soporta 2 direcciones I2C configurables via pin de direccion (ADDR):

| Sensor | Bus I2C | Pines | Direccion | Config ADDR |
|---|---|---|---|---|
| Sensor 1 | I2C0 (Wire) | GP4/GP5 | 0x1F (A0) | ADDR a GND |
| Sensor 2 | I2C0 (Wire) | GP4/GP5 | 0x5E (A1) | ADDR a VCC |
| Sensor 3 | I2C1 (Wire1) | GP2/GP3 | 0x1F (A0) | ADDR a GND |

> Los sensores 1 y 2 comparten el mismo bus I2C pero tienen direcciones distintas.  
> El sensor 3 usa un segundo bus I2C independiente para evitar conflictos de direccion.

---

## Paso 3 — Pinout del TLV493D-A1B6

El TLV493D-A1B6 en formato SOT-23-6 tiene estos pines:

```
TLV493D-A1B6 (vista superior, SOT-23-6)
+---+
| 1 | SDA  -> GP4 (sensor 1 y 2) o GP2 (sensor 3)
| 2 | GND  -> GND
| 3 | VCC  -> 3V3 (rango: 2.7V - 3.5V)
| 4 | SCL  -> GP5 (sensor 1 y 2) o GP3 (sensor 3)
| 5 | ADDR -> GND (addr 0x1F) o VCC (addr 0x5E)
| 6 | INT  -> no conectar (opcional, no usado en firmware)
+---+
```

Si usas breakout board, los pines suelen estar etiquetados directamente (SDA, SCL, VCC, GND, ADDR).

---

## Paso 4 — Conexion con 1 sensor (Opcion A)

Usar entorno: `rp2040-tlv493d-1`

```
YD-RP2040           TLV493D (Sensor 1)
GP4 (SDA0) -------- SDA
GP5 (SCL0) -------- SCL
3V3         -------- VCC
GND         -------- GND
GND         -------- ADDR  (direccion 0x1F)

GP0 ---- BTN0 ---- GND
GP1 ---- BTN1 ---- GND
GP6 ---- BTN2 ---- GND
```

**Limitacion:** Con 1 solo sensor los 6 DOF son aproximados. El campo Bz del sensor se usa para RZ (rotacion Z), lo que no es ideal para uso real en CAD.

---

## Paso 5 — Conexion con 3 sensores (Opcion B, recomendado)

Usar entorno: `rp2040-tlv493d-3`

```
YD-RP2040           TLV493D Sensor 1        TLV493D Sensor 2        TLV493D Sensor 3
GP4 (SDA0) -------- SDA              ------ SDA
GP5 (SCL0) -------- SCL              ------ SCL
GP2 (SDA1) ----------------------------------------              -- SDA
GP3 (SCL1) ----------------------------------------              -- SCL
3V3         -------- VCC              ------ VCC              ------ VCC
GND         -------- GND              ------ GND              ------ GND
GND         -------- ADDR (0x1F)
3V3         ---------------------------------------- ADDR (0x5E)
GND         ----------------------------------------              -- ADDR (0x1F)

GP0 ---- BTN0 ---- GND
GP1 ---- BTN1 ---- GND
GP6 ---- BTN2 ---- GND
```

---

## Paso 6 — Disposicion fisica de los sensores

Con 3 sensores TLV493D, una disposicion efectiva es:

```
Vista lateral del mecanismo puck:

           [Iman - bajo el puck]
           
    [S1]          [S2]          [S3]
  (eje XY)      (eje XZ)      (eje YZ)

Los 3 sensores se ubican en posiciones ortogonales entre si.
```

Cada sensor TLV493D detecta Bx, By, Bz del campo magnetico del iman. Con 3 sensores en posiciones distintas se puede reconstruir el movimiento completo en 6 DOF.

---

## Paso 7 — Flash del firmware (variante TLV493D)

### Con 1 sensor:
```bash
# Modo BOOTSEL: mantener BOOT mientras enchufas USB
pio run -e rp2040-tlv493d-1 --target upload
```

### Con 3 sensores:
```bash
pio run -e rp2040-tlv493d-3 --target upload
```

**Verificar:** El dispositivo debe aparecer como `SpaceMouse Pro Wireless`  
VID: `256f` | PID: `c631`

---

## Paso 8 — Test con debug serial

En `src/main.cpp` linea ~83:
```cpp
#define DEBUG 1
```

Abrir Serial Monitor a **250000 baud**.  
Con TLV493D los valores son **flotantes** (no enteros como en Hall).  
Rango en reposo esperado: ~0.0 en los 3 ejes (Bx, By, Bz) de cada sensor.

**Verificar LED RGB:**
- Azul tenue -> Azul pulsante (calibracion ~1.2 seg) -> Azul fijo (listo)

---

## Paso 9 — Calibracion

El firmware promedia 3 lecturas al bootear para establecer el punto cero.  
El puck **debe estar en reposo** al conectar USB.

Si hay deriva, ajustar el deadzone en `src/main.cpp` linea ~202:
```cpp
#define DEADZONE 40   // para TLV493D usar valores menores (20-40) por mayor precision
```

El factor de escala TLV493D (linea ~77):
```cpp
#define TLV493D_SCALE 10.0f   // aumentar si el movimiento es muy pequeno
```

---

## Paso 10 — Prueba en CAD

1. Conectar el dispositivo (sin drivers extra en Windows)
2. Abrir FreeCAD -> Edit -> Preferences -> Input Devices -> Space Mouse debe aparecer
3. Verificar los 6 ejes con movimientos suaves del puck

---

## Referencia rapida de pines (TLV493D)

| GPIO | Funcion |
|---|---|
| GP2 | I2C1 SDA — Sensor 3 |
| GP3 | I2C1 SCL — Sensor 3 |
| GP4 | I2C0 SDA — Sensor 1 y 2 |
| GP5 | I2C0 SCL — Sensor 1 y 2 |
| GP0 | Boton izquierdo |
| GP1 | Boton derecho |
| GP6 | Boton frontal |
| GP23 | LED RGB (interno, no cablear) |
| 3V3 | VCC sensores (max 3.5V — no usar 5V) |
| GND | GND comun |

---

## Entornos PlatformIO

```ini
# 1 sensor TLV493D
pio run -e rp2040-tlv493d-1 --target upload

# 3 sensores TLV493D (recomendado)
pio run -e rp2040-tlv493d-3 --target upload
```

---

## Comparacion final: cual usar?

| Criterio | Hall Effect | TLV493D x3 |
|---|---|---|
| Costo | Bajo | Medio |
| Complejidad de cableado | Media (4 cables de signal) | Media (I2C compartido) |
| Precision 6-DOF | Buena | Muy buena |
| Firmware probado en HW | Si | No (solo compilado) |
| Recomendado para | Primer prototipo | Version final |
