# Calibration & Test Plan

## Fase 1 — Test eléctrico sin imanes

1. Conectar Pro Micro por USB.
2. Medir VCC-GND en el Pro Micro.
3. Medir VCC-GND en cada perfboard.
4. Confirmar continuidad de GND común.
5. Medir OUT de cada sensor sin imán.

Resultado esperado:

```text
OUT ≈ VCC / 2
```

Ejemplos:

```text
VCC = 5.0V  → OUT ≈ 2.5V
VCC = 3.3V  → OUT ≈ 1.65V
```

## Fase 2 — Test sensor por sensor

Para cada sensor:

1. Abrir debug serial.
2. Acercar imán lentamente.
3. Alejar imán.
4. Invertir polo.
5. Confirmar que la lectura sube/baja.
6. Registrar el pin correcto.

Tabla de registro:

| Sensor | Pin | Valor reposo | Min | Max | Responde | Observaciones |
|---:|---|---:|---:|---:|---|---|
| 0 | A0 | | | | | |
| 1 | A1 | | | | | |
| 2 | A2 | | | | | |
| 3 | A3 | | | | | |
| 6 | A6 | | | | | |
| 7 | A7 | | | | | |
| 8 | A8 | | | | | |
| 9 | A9 | | | | | |

## Fase 3 — Test con placa de imanes

1. Montar imanes temporalmente.
2. No pegar definitivo todavía.
3. Dejar distancia inicial de 5–6 mm.
4. Mover knob en X/Y/Z.
5. Rotar en pitch/roll/yaw.
6. Observar si los sensores saturan.

Criterio:

```text
Reposo cerca del centro.
Movimiento suave.
Sin saltos bruscos.
Sin valores clavados en 0 o 1023.
```

## Fase 4 — Calibración mecánica

Si hay saturación:

```text
Aumentar distancia sensor-imán.
```

Si hay poca señal:

```text
Reducir distancia sensor-imán.
```

Si un eje responde más que otro:

```text
Revisar centrado de imanes.
Revisar simetría de resortes.
Revisar que sensores estén a igual altura.
```

## Fase 5 — Calibración firmware

- Guardar offsets de reposo.
- Aplicar deadzone pequeña.
- Aplicar smoothing moderado.
- Normalizar amplitudes por eje.
- Invertir ejes si hace falta.
- Confirmar que la respuesta en CAD sea natural.

## Señales de problema

| Síntoma | Causa probable | Acción |
|---|---|---|
| OUT fijo en 0V | pinout mal, corto a GND, sensor dañado | revisar cableado |
| OUT fijo en VCC | corto a VCC, sensor dañado | revisar cableado |
| OUT no cambia con imán | sensor invertido, imán lejos, pin incorrecto | probar manual |
| Lecturas muy ruidosas | cable largo, falta desacople, GND malo | agregar 100nF OUT-GND, revisar GND |
| Lectura satura rápido | imán muy cerca/fuerte | aumentar distancia |
| Un eje invertido | polaridad/firmware | invertir eje en firmware |
