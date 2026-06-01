# Proyecto RP2040: Anillo LED FZ1556, servo y sensor TLV493D

Este proyecto demuestra cómo controlar un **anillo de LEDs FZ1556** (compatible con WS2812), un **servo** y leer un **sensor magnético 3D TLV493D** usando un microcontrolador RP2040 (por ejemplo, Raspberry Pi Pico) con MicroPython.  El objetivo es ofrecer un ejemplo práctico de cómo integrar distintos periféricos en un mismo programa manteniendo un control de **timing** preciso gracias al hardware del RP2040.

## Componentes utilizados

### Sensor TLV493D

El **TLV493D** de Infineon es un sensor magnético 3D que mide el campo magnético en los ejes X, Y y Z.  Destaca por su **bajo consumo (~10 µA)**, resolución de **12 bits** en cada dirección y soportar rangos de medición hasta ±130 mT【796762022328058†L26-L46】.  La comunicación se realiza mediante **I²C** a velocidades de hasta 1 Mbit/s y dispone de modos de bajo consumo y apagado【796762022328058†L26-L46】.

En este proyecto se implementa un driver minimalista (`tlv493d.py`) basado en el algoritmo de decodificación de la librería CircuitPython de Adafruit.  La función `read_magnetic()` devuelve una tupla `(x, y, z)` en microteslas (µT) tras combinar los registros de 8 y 4 bits y multiplicar por 98 según la referencia【160945116428272†L169-L181】.

### Servo

Para mover un eje mecánico se utiliza un servo estándar de 9 g.  Los servos esperan pulsos de control de entre ~1 ms (0°) y ~2 ms (180°) dentro de un periodo de 20 ms (50 Hz).  El RP2040 posee hardware PWM dedicado; sin embargo, la tecnología **PIO** permite crear interfaces personalizadas de temporización muy precisa, como servos, UART o SPI de software【937902857186587†L86-L90】.  En este ejemplo se usa el PWM integrado para generar la señal a 50 Hz.

### Anillo LED FZ1556

El anillo FZ1556 es una tira circular de **LEDs WS2812** (a veces llamados NeoPixels).  Cada LED contiene un controlador que permite fijar el color RGB de forma individual mediante un bus de datos de un solo hilo.  En el código se usa el módulo `neopixel` de MicroPython para enviar los datos al anillo.

## Conexiones recomendadas

| Componente | Pin del RP2040 | Descripción |
|-----------|----------------|-------------|
| **Servo** | GP0 | Señal PWM de 50 Hz para el servo |
| **Anillo LED** | GP28 | Línea de datos del anillo WS2812 |
| **TLV493D SDA** | GP0* | Línea de datos I²C (use un pin distinto si su servo comparte GP0) |
| **TLV493D SCL** | GP1 | Línea de reloj I²C |
| **Alimentación** | 3,3 V (sensor) / 5 V (anillo y servo) | Conecte la alimentación según las especificaciones de cada dispositivo |
| **Tierra** | GND | Conecte todas las tierras comunes |

\*Si su servo utiliza el mismo pin (GP0), cambie `SERVO_PIN` o `SDA_PIN` en `main.py` para evitar conflictos.

Para el TLV493D se recomienda añadir resistencias **pull‑up** (4,7 kΩ) en SDA y SCL.  Asegúrese de no superar los rangos de tensión especificados por cada componente.

## Archivos del proyecto

* **`main.py`** – Programa principal que inicializa el sensor, el servo y el anillo LED.  Lee continuamente el campo magnético, asigna la componente X a un ángulo de servo (0–180°) y ajusta el color del anillo LED según las magnitudes de los campos X, Y y Z.  El ciclo se repite cada 50 ms para suavizar el bus I²C y la actualización del anillo.
* **`tlv493d.py`** – Driver minimalista del sensor TLV493D escrito en MicroPython.  Se comunica por I²C y decodifica los valores de 12 bits en microteslas【160945116428272†L169-L181】.
* **`README.md`** – Este documento explicativo con instrucciones de uso y referencias.

## Cómo usar el proyecto

1. **Instale MicroPython en su RP2040.** Consulte la documentación oficial de Raspberry Pi Pico para cargar el firmware MicroPython.  Luego, use el explorador de archivos o un entorno como Thonny para copiar los archivos `main.py` y `tlv493d.py` al directorio raíz del dispositivo.
2. **Ajuste los pines y número de LEDs.** Si utiliza un número distinto de LEDs o pines diferentes, modifique las constantes `NUM_PIXELS`, `SERVO_PIN`, `LED_PIN`, `SDA_PIN` y `SCL_PIN` al inicio de `main.py`.
3. **Conecte los dispositivos según la tabla de conexiones.** Respete la alimentación de cada módulo y utilice resistencias pull‑up en SDA y SCL.
4. **Reinicie el RP2040.** Al iniciar, el programa leerá continuamente los valores del TLV493D y actuará sobre el servo y el anillo LED.  Mueva un imán alrededor del sensor para observar la reacción: el servo girará en función de la componente X del campo y el anillo cambiará de color según los ejes X (rojo), Y (verde) y Z (azul).

## Explicación del código

### `tlv493d.py`

La clase `TLV493D` inicializa el sensor en el modo de medición por defecto.  El método `_read_raw()` realiza una lectura de 10 bytes desde el registro 0x00; los tres primeros bytes contienen los 8 bits menos significativos de las componentes X, Y y Z, y los tres siguientes contienen los 4 bits más significativos en sus nibble superiores.  La función estática `_unpack_12bit()` combina estos bytes en un entero con signo de 12 bits y `read_magnetic()` multiplica los valores por 98,0 para convertirlos a microteslas【160945116428272†L169-L181】.

### `main.py`

* **Inicialización:** se crea el bus I²C, se instancian el sensor y el PWM para el servo y se inicializa el anillo LED con el número de píxeles indicado.
* **Conversión a ángulo:** se lee el campo magnético `(x, y, z)`.  La componente X se recorta a ±16 000 µT y se mapea linealmente a un ángulo de 0–180° para controlar el servo.  La conversión de ángulo a ciclo de trabajo PWM usa un rango de 1–2 ms dentro de un periodo de 20 ms (50 Hz).
* **Colores del anillo LED:** cada color RGB se calcula a partir del valor absoluto de X, Y y Z (clipeado) y se escala a 0–255.  Todos los LEDs muestran el mismo color para simplificar el ejemplo.

### Extensiones posibles

* **Agregar filtros o promedios** para suavizar lecturas.
* **Controlar más servos o efectos LED** mediante el subsistema PIO del RP2040 para obtener más canales sin jitter【937902857186587†L86-L90】.
* **Enviar datos por UART o USB** para registrar el campo magnético o ajustar parámetros en tiempo real.

## Créditos y referencias

* La lista de características del TLV493D proviene del artículo de *RP2040 Learning* sobre el sensor, que menciona la medición 3D, consumo de 10 µA, resolución de 12 bits y la interfaz I²C hasta 1 Mbit/s【796762022328058†L26-L46】.
* El uso del PIO del RP2040 para generar interfaces como servos, UARTs o SPI sin jitter está descrito en un tutorial de Hackster.io【937902857186587†L86-L90】.
* La decodificación de los datos del TLV493D en microteslas se basa en la implementación de Adafruit para CircuitPython【160945116428272†L169-L181】.
