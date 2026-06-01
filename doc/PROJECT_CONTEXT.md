# Project Context — Spacemouse

## Descripción

Proyecto DIY para construir un **SpaceMouse / CAD Mouse 6DoF** casero con sensores Hall lineales. El diseño base analizado y usado como referencia principal es **ChromeBee Hall-Effect-Sensor-CAD-Mouse-Spacemouse**.

El objetivo es obtener un dispositivo funcional para CAD / Fusion 360 / modelado 3D, capaz de detectar desplazamientos y rotaciones mediante pares de sensores Hall y una placa móvil con imanes.

## Proyecto base elegido

Base principal:

- ChromeBee / Hall-Effect-Sensor-CAD-Mouse-Spacemouse.
- Usa sensores Hall lineales baratos.
- Usa Arduino Pro Micro.
- Tiene firmware derivado/inspirado en Teaching Tech.
- Emula un dispositivo reconocido por el stack 3DConnexion usado en varios proyectos DIY.

Alternativa analizada:

- AndunHH/spacemouse.
- Muy útil como referencia de firmware/HID y configuración.
- No es la base de hardware principal actual, porque Nico compró hardware pensando en ChromeBee.

## Estado físico actual

- Nico ya montó los sensores Hall.
- Hay **8 sensores en total**.
- Distribución: **2 sensores por lado**.
- Montaje: **4 perfboards**, una por cada par de sensores.
- Cada perfboard/sensor ya tiene desacople con capacitores cerámicos de 100nF entre VCC y GND.
- Todavía se está cerrando el cableado correcto hacia el Pro Micro.

## Hardware confirmado

- Arduino Pro Micro ATmega32U4.
- 8 sensores Hall lineales 49E/SS49E en uso.
- Stock registrado: 10 sensores Hall SS49E/OH49E/49E.
- Capacitores cerámicos 100nF.
- Capacitores cerámicos 10nF.
- Resistencias 220Ω, 1KΩ, 10KΩ.
- ADS1015 I2C 12-bit 4 canales disponible, pero no necesario para la arquitectura ChromeBee original.
- Imanes disponibles:
  - 6×2 mm.
  - 11×8×1.7 mm.
- Imanes del proyecto original: 10×5×3 mm.

## Decisiones tomadas

- Mantener Arduino Pro Micro por compatibilidad HID/USB.
- Usar sensores Hall 49E/SS49E para la primera versión.
- No migrar todavía a ESP32-S3, RP2040 ni sensores 3D tipo TLV493D.
- Priorizar que el hardware actual funcione antes de rediseñar mecánica/firmware.
- Usar el pinout real confirmado por Nico, no asumir pinouts de internet.

## Información crítica

Pinout real de los sensores Hall de Nico:

```text
Vista:
- Cara angular/plana mirando hacia el usuario.
- Patas hacia abajo.

Pines:
1 = VCC
2 = GND
3 = DATA / OUT
```

Advertencia: este pinout debe respetarse. Un pinout incorrecto puede dañar sensores o entregar lecturas inválidas.

## Riesgos principales

- Pinout invertido en sensores Hall.
- Imanes más grandes/delgados que los originales pueden saturar los sensores.
- Distancia sensor/imán incorrecta puede reducir resolución útil.
- Cableado largo o mal desacoplado puede meter ruido.
- GND no común puede invalidar lecturas.
- Orden físico de sensores debe coincidir con el firmware.
- Pro Micro debe configurarse correctamente para HID compatible.
