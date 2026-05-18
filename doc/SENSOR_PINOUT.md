# Sensor Pinout — 49E / SS49E de Nico

## Pinout confirmado

Este pinout fue confirmado por Nico para sus sensores Hall 49E/SS49E.

```text
Orientación:
- Cara angular/plana mirando hacia el usuario.
- Patas hacia abajo.

Vista frontal:

   ┌───────────────┐
   │ 49E / SS49E   │
   │ cara frontal  │
   └───────────────┘
      │   │   │
      1   2   3

1 = VCC
2 = GND
3 = DATA / OUT
```

## Conexión por sensor

```text
Pin 1 / VCC  → VCC del Pro Micro
Pin 2 / GND  → GND común
Pin 3 / OUT  → Entrada analógica individual
```

## Capacitores

Para cada sensor:

```text
100nF cerámico entre VCC y GND, lo más cerca posible del sensor.
```

Recomendado si hay ruido:

```text
100nF cerámico entre OUT y GND, lo más cerca posible del sensor.
```

## Advertencias

- No confiar en colores de cables tipo servo.
- En conectores servo comunes, el pin central suele ser 5V, pero en estos sensores el pin central es GND.
- Revisar continuidad antes de conectar USB.
- Medir VCC-GND en cada perfboard antes de leer sensores.
- OUT sin imán debería quedar aproximadamente a VCC/2.
