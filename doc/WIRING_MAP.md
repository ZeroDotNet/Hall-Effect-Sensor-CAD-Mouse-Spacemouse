# Wiring Map — Spacemouse

## Arquitectura general

```text
Arduino Pro Micro
├── VCC común → VCC de los 8 sensores Hall
├── GND común → GND de los 8 sensores Hall + pulsadores
├── Entradas analógicas → OUT individual de cada sensor
└── Entradas digitales → 3 pulsadores
```

## Mapa de sensores según proyecto ChromeBee

El proyecto base numera los sensores por el pin ADC/analógico usado. La disposición está pensada con el USB del Pro Micro hacia atrás.

| Sensor lógico | Ubicación física esperada | Señal | Pin Arduino Pro Micro aproximado |
|---:|---|---|---|
| 0 | Frente, par inferior | OUT | A0 |
| 1 | Frente, par inferior | OUT | A1 |
| 2 | Lateral derecho | OUT | A2 |
| 3 | Lateral derecho | OUT | A3 |
| 6 | Parte trasera | OUT | A6 |
| 7 | Parte trasera | OUT | A7 |
| 8 | Lateral izquierdo | OUT | A8 |
| 9 | Lateral izquierdo | OUT | A9 |

> Verificar contra el firmware final antes de soldar definitivo.

## Conexión recomendada por perfboard

Nico tiene 4 perfboards, cada una con 2 sensores.

Por cada perfboard:

```text
Sensor A VCC ┐
             ├── VCC común de esa perfboard ──→ VCC común global
Sensor B VCC ┘

Sensor A GND ┐
             ├── GND común de esa perfboard ──→ GND común global
Sensor B GND ┘

Sensor A OUT ────────────────────────────────→ pin analógico individual
Sensor B OUT ────────────────────────────────→ pin analógico individual
```

## Alimentación

Primera versión recomendada:

```text
Pro Micro por USB
VCC de sensores desde VCC del Pro Micro
GND común único
```

Si el Pro Micro es 5V/16MHz:

```text
VCC sensores = 5V
OUT sin imán esperado ≈ 2.5V
ADC central esperado ≈ 512 en lectura de 10 bits
```

Si el Pro Micro es 3.3V/8MHz:

```text
VCC sensores = 3.3V
OUT sin imán esperado ≈ 1.65V
ADC central esperado ≈ 512 en lectura de 10 bits
```

## Pulsadores

El proyecto original usa 3 pulsadores.

Recomendación:

```text
Un lado de cada pulsador → GND
Otro lado → pin digital configurado como INPUT_PULLUP
```

Estado esperado:

```text
Sin presionar = HIGH
Presionado = LOW
```

## Checklist antes de conectar por USB

- No hay corto entre VCC y GND.
- Cada perfboard tiene VCC y GND correctos.
- OUT no está conectado accidentalmente a VCC/GND.
- Todos los GND están unidos.
- Los capacitores 100nF están entre VCC y GND.
- No hay puente de estaño entre patas de sensores.
- La orientación del pinout fue verificada sensor por sensor.
