# Armado del Circuito — Backend Hall Effect

Plataforma target: **YD-RP2040** con 4 sensores Hall Effect analogicos.  
Esta es la opcion mas completa y probada del proyecto.

---

## Paso 0 — Componentes necesarios

| # | Componente | Cantidad |
|---|---|---|
| 1 | YD-RP2040 (o Raspberry Pi Pico) | 1 |
| 2 | Sensor Hall Effect (A1302, A1303 o SS495) | 4 |
| 3 | Iman de neodimio cilindrico (~6mm diametro) | 1 |
| 4 | Puck/knob mecanico (parte movil que agarra el usuario) | 1 |
| 5 | Pulsadores momentaneos (push button) | 3 |
| 6 | Cables Dupont M-F o M-M | varios |
| 7 | Protoboard o PCB | 1 |
| 8 | Cable USB-C | 1 |

---

## Paso 1 — Pinout del YD-RP2040

El LED RGB WS2812B esta **integrado en la placa en GPIO23** — no hace falta cablear nada extra para el LED.

```
YD-RP2040 — pines del proyecto
+----------------------------------------------+
|  GP26 (ADC0) -> Sensor Hall HES0             |
|  GP27 (ADC1) -> Sensor Hall HES1             |
|  GP28 (ADC2) -> Sensor Hall HES2             |
|  GP29 (ADC3) -> Sensor Hall HES3             |
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

## Paso 2 — Conexion de sensores Hall Effect

Cada sensor Hall (A1302/SS495) tiene **3 pines**: VCC, GND, OUTPUT.

**Cableado por sensor:**
```
VCC    -> 3V3 del RP2040
GND    -> GND del RP2040
OUTPUT -> GP26 / GP27 / GP28 / GP29  (uno por sensor)
```

**Disposicion fisica bajo el puck** (vista desde arriba):
```
         [HES1 - GP27]
              ^
[HES2-GP28] <-  -> [HES0-GP26]
              v
         [HES3 - GP29]
```

Los 4 sensores van a **90 grados entre si** en el mismo plano horizontal, apuntando hacia arriba al iman que esta debajo del puck. Distancia inicial recomendada entre sensor e iman: **3-5mm**.

> **Importante:** El iman debe quedar centrado sobre los 4 sensores cuando el puck esta en reposo. Esa es la posicion de calibracion.

---

## Paso 3 — Conexion de botones

Los botones usan `INPUT_PULLUP` interno — activo en LOW (presionar = cortocircuito a GND). No se necesitan resistencias externas.

```
BTN0 (izquierdo) -> pin al GP0,  otro pin a GND
BTN1 (derecho)   -> pin al GP1,  otro pin a GND
BTN2 (frontal)   -> pin al GP6,  otro pin a GND
```

---

## Paso 4 — Diagrama completo

```
YD-RP2040
|
+-- GP26 ---- HES0 OUTPUT
+-- GP27 ---- HES1 OUTPUT
+-- GP28 ---- HES2 OUTPUT
+-- GP29 ---- HES3 OUTPUT
|
+-- GP0  ---- BTN0 ---- GND
+-- GP1  ---- BTN1 ---- GND
+-- GP6  ---- BTN2 ---- GND
|
+-- 3V3  --+- HES0 VCC
|          +- HES1 VCC
|          +- HES2 VCC
|          +- HES3 VCC
|
+-- GND  --+- HES0 GND
           +- HES1 GND
           +- HES2 GND
           +- HES3 GND
           +- BTN0
           +- BTN1
           +- BTN2
```

---

## Paso 5 — Flash del firmware

```bash
# Conectar YD-RP2040 en modo BOOTSEL (mantener BOOT mientras enchufas USB)
# Copiar el .uf2 al drive que aparece (letra J: u otra)
cp .pio/build/rp2040/firmware.uf2 /J/
```

**Verificar:** El dispositivo debe aparecer como `SpaceMouse Pro Wireless`  
VID: `256f` | PID: `c631`

---

## Paso 6 — Test de sensores con debug serial

En `src/main.cpp` linea ~83, activar debug nivel 1:
```cpp
#define DEBUG 1
```

Reconstruir y flashear. Abrir Serial Monitor a **250000 baud**.  
Mover el puck y verificar que los 4 sensores cambian sus valores ADC.  
Rango esperado en reposo: ~512 (mitad del rango de 0-1023).

**Verificar LED RGB:**
- Azul tenue -> Azul pulsante (calibracion ~1.2 seg) -> Azul fijo (listo)

---

## Paso 7 — Calibracion mecanica

El firmware calibra automaticamente al bootear (3 lecturas promediadas).  
El puck **debe estar en reposo** al conectar el USB.

Si hay deriva sin tocar el puck, aumentar el deadzone en `src/main.cpp` linea ~202:
```cpp
#define DEADZONE 40   // aumentar a 60-80 si hay ruido
```

---

## Paso 8 — Prueba en CAD

1. Conectar el dispositivo (sin drivers extra en Windows)
2. Abrir FreeCAD -> Edit -> Preferences -> Input Devices -> Space Mouse debe aparecer
3. Movimientos esperados:

| Movimiento fisico | Eje HID |
|---|---|
| Empujar/tirar horizontalmente | TX (pan horizontal) |
| Inclinar adelante/atras | TY (pan vertical) |
| Rotar/twist | RZ (rotar vista) |
| Presionar hacia abajo | TZ (zoom) |

---

## Referencia rapida de pines

| GPIO | Funcion |
|---|---|
| GP26 | Hall Sensor 0 (derecha) |
| GP27 | Hall Sensor 1 (arriba) |
| GP28 | Hall Sensor 2 (izquierda) |
| GP29 | Hall Sensor 3 (abajo) |
| GP0 | Boton izquierdo |
| GP1 | Boton derecho |
| GP6 | Boton frontal |
| GP23 | LED RGB (interno, no cablear) |
| 3V3 | VCC sensores |
| GND | GND comun |

---

## Entorno PlatformIO a usar

```ini
# platformio.ini -> environment: rp2040
pio run -e rp2040 --target upload
```
