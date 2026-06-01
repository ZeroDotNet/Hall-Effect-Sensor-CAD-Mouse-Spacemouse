# Mechanical Notes — Spacemouse

## Base mecánica

El diseño de referencia ChromeBee usa:

- Placa inferior con Arduino Pro Micro.
- Placa de sensores.
- Placa de imanes móvil.
- Resortes de tensión.
- Tornillo M4 central.
- Perillas/carcasa impresas en 3D.
- Distancia inicial sugerida entre placa de sensores y placa de imanes: **5–6 mm** como punto de partida.

## Imanes

### Imanes originales del proyecto base

```text
10×5×3 mm neodimio
Cantidad: 4
```

### Imanes disponibles de Nico

```text
6×2 mm
11×8×1.7 mm
```

## Evaluación de imanes 11×8×1.7 mm

Pros:
- Área mayor.
- Campo potencialmente usable aunque sean más finos.
- Probablemente funcionen si se ajusta distancia.

Contras:
- Geometría diferente al diseño original.
- Pueden saturar o sesgar demasiado el sensor si quedan muy cerca.
- La menor altura de 1.7 mm puede cambiar el gradiente de campo.
- Puede requerir modificar alojamiento o distancia sensor-imán.

Recomendación pragmática:
- Probarlos antes de rediseñar.
- Montar temporalmente sin pegamento definitivo.
- Medir lecturas crudas de cada sensor en reposo y extremos.
- Ajustar distancia hasta que el movimiento útil no sature.

## Evaluación de imanes 6×2 mm

Pros:
- Más fáciles de ubicar.
- Menor riesgo de saturación.
- Útiles para pruebas.

Contras:
- Pueden dar señal débil.
- Puede bajar resolución efectiva.
- Quizás requieran menor distancia sensor/imán.

## Distancia inicial

```text
Comenzar con 5–6 mm entre placa de sensores y placa de imanes.
```

Luego ajustar según lecturas:

```text
Si satura demasiado rápido → aumentar distancia.
Si la variación es muy baja → reducir distancia.
Si un eje responde asimétrico → revisar centrado mecánico e imanes.
```

## Resortes / TPU

El documento base menciona variantes con resortes y versiones posteriores con TPU spring / TPU wave spring. Para Nico conviene:

1. Terminar versión funcional con el hardware ya comprado.
2. Validar sensores + firmware.
3. Recién después evaluar cambio a TPU si la mecánica queda dura, ruidosa o desbalanceada.

## Base pesada

El diseño de referencia contempla una base que puede rellenarse con material pesado no metálico:

- Cemento.
- Resina epoxi.
- Arcilla/modeling clay.

Evitar materiales ferromagnéticos cerca de sensores/imanes.
