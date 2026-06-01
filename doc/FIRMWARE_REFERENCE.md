# Firmware Reference Notes

## Repos de referencia

- ChromeBee/Hall-Effect-Sensor-CAD-Mouse-Spacemouse
- AndunHH/spacemouse
- Teaching Tech Open Source Spacemouse / Space Mushroom Remix

## Rol de cada referencia

| Referencia | Rol |
|---|---|
| ChromeBee | Base hardware/mecánica principal |
| AndunHH | Referencia fuerte para configuración HID / SpaceMouse |
| Teaching Tech | Inspiración original y compatibilidad con 3DConnexion |
| CAD Jungle | Inspiración CAD/mecánica |
| Rodrigo Alvarez teardown | Referencia conceptual sobre SpaceNavigator |

## Estrategia firmware recomendada

- No mezclar demasiados enfoques al principio.
- Partir del firmware compatible con ChromeBee/Teaching Tech.
- Validar sensores crudos antes de calibrar ejes 6DoF.
- Mantener una versión debug simple para diagnóstico.
- Mantener una versión release limpia para uso CAD.

## Debug mínimo deseado

Serial output con:

```text
raw0 raw1 raw2 raw3 raw6 raw7 raw8 raw9
center offsets
normalized X Y Z RX RY RZ
button states
```

## Parámetros que deberían quedar configurables

- Deadzone.
- Smoothing.
- Ganancia por eje.
- Inversión por eje.
- Offset de centro.
- Rango mínimo/máximo por sensor.
- Mapeo de botones.
