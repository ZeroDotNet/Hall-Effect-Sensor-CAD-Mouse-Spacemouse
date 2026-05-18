# Next Steps — Spacemouse

## Prioridad 1 — Cableado seguro

1. Confirmar pinout físico de los 8 sensores.
2. Etiquetar cada perfboard.
3. Etiquetar cada OUT.
4. Unir VCC común.
5. Unir GND común.
6. Llevar cada OUT al pin analógico correspondiente.
7. Verificar con multímetro antes de conectar por USB.

## Prioridad 2 — Test eléctrico

1. Medir VCC-GND.
2. Medir OUT-GND de cada sensor sin imán.
3. Confirmar OUT ≈ VCC/2.
4. Acercar imán y confirmar variación.

## Prioridad 3 — Debug firmware

1. Cargar firmware base.
2. Activar debug nivel 1.
3. Mapear cada sensor.
4. Activar debug nivel 2.
5. Verificar movimiento de placa magnética.

## Prioridad 4 — Mecánica

1. Montar imanes sin pegamento definitivo.
2. Usar distancia inicial 5–6 mm.
3. Ajustar altura según saturación.
4. Recién pegar imanes cuando las lecturas sean correctas.

## Prioridad 5 — CAD

1. Configurar Pro Micro como HID requerido.
2. Confirmar detección del dispositivo.
3. Probar en Fusion 360.
4. Ajustar ejes, deadzone y sensibilidad.

## Orden recomendado

```text
1. Cableado
2. Medición con multímetro
3. Debug serial sensor por sensor
4. Montaje temporal de imanes
5. Calibración mecánica
6. Calibración firmware
7. Prueba CAD
8. Cierre mecánico
```
