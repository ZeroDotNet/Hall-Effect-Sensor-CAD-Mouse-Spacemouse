# Firmware Notes — Spacemouse

## MCU

```text
Arduino Pro Micro / ATmega32U4
```

Motivo:
- USB nativo.
- Puede actuar como dispositivo HID.
- Compatible con el enfoque de proyectos DIY SpaceMouse.

## Proyecto base

El firmware de ChromeBee deriva/inspira su flujo en Teaching Tech y otros proyectos open-source de SpaceMouse.

El objetivo es que el equipo sea reconocido por software compatible con el flujo de 3DConnexion usado por estos proyectos.

## Lectura de sensores

Cada sensor Hall entrega una señal analógica.

Comportamiento esperado:

```text
Sin imán:
OUT ≈ VCC / 2

Con campo magnético:
OUT sube o baja según polo y distancia
```

En el proyecto base, durante debug se espera que al acercar el polo norte al sensor, los números bajen. Usar esto para identificar polaridad y validar orientación.

## Debug recomendado

Modo debug 1:
- Validar sensor por sensor.
- Acercar imán.
- Confirmar que el sensor responde.
- Confirmar pin correcto.

Modo debug 2:
- Mover placa magnética.
- Confirmar que todos los sensores suben/bajan.
- Validar simetría general.

## Orden de sensores

No cambiar el orden lógico sin actualizar el firmware.

Mapa base esperado:

```text
Frente: sensores 0 y 1
Lateral derecho: sensores 2 y 3
Atrás: sensores 6 y 7
Lateral izquierdo: sensores 8 y 9
```

## Pulsadores

El proyecto original usa 3 botones.

Mapeo funcional mencionado para Fusion 360:

```text
Botón derecho  → vista superior
Botón medio    → vista derecha
Botón izquierdo→ vista frontal
Izq + der      → menú/configuración 3DConnexion del programa actual
```

Esto puede cambiarse en firmware.

## ADS1015

Nico tiene un ADS1015 I2C 12-bit de 4 canales, pero:

- No es necesario para replicar ChromeBee.
- Solo tiene 4 canales; el proyecto usa 8 sensores.
- Para 8 sensores harían falta 2 ADS1015 o un ADC de más canales.
- Aporta resolución, pero complica firmware y latencia.
- Recomendación: no usarlo en la primera versión salvo que el ADC interno sea claramente insuficiente.

## Criterio de éxito firmware

- Los 8 canales leen valores estables.
- En reposo, valores cercanos al centro.
- Con movimiento, cada eje tiene variación clara y repetible.
- No hay saturación prematura.
- El dispositivo se reconoce como HID esperado.
- Fusion 360 / CAD responde a traslación y rotación.
