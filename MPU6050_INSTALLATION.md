# Guía de Instalación: MPU-6050 para HE_Spacemouse

## Resumen de la Integración

Se ha agregado soporte para **MPU-6050 (IMU de 6 ejes)** al SpaceMouse Hall-Effect. El giroscopio se usa para mejorar la detección de **rotaciones (RX, RY, RZ)** con un complementary filter, mientras que los 8 sensores Hall continúan detectando **translaciones (TX, TY, TZ)**.

### Cambios Realizados

1. **Includes:** `Wire.h` + `MPU6050.h`
2. **Variables globales:** `MPU6050 mpu`, offsets, valores integrados
3. **4 Funciones nuevas:**
   - `initMPU()` — Inicializa I2C y sensor
   - `calibrateGyro()` — Calibración al startup (100 muestras, ~2s)
   - `readMPU()` — Lee giroscopio en °/s
   - `integrateAndFuseGyro()` — Integra + aplica complementary filter
4. **Setup modificado:** Ahora llama `initMPU()` + `calibrateGyro()`
5. **Loop modificado:** Lee MPU + aplica fusión a rotaciones
6. **Debug level 7:** Nuevo modo de diagnóstico para giroscopio

---

## Instalación de Librería

### Paso 1: Instalar "MPU6050 by Electronic Cats"

En **Arduino IDE**:
1. **Sketch** → **Include Library** → **Manage Libraries...**
2. Busca: `MPU6050`
3. Selecciona: **"MPU6050 by Electronic Cats"** (versión más reciente)
4. Haz click en **Install**

![Captura de Manage Libraries](./docs/manage_libraries.png)

### Paso 2: Verificar Conexión Física

**Pines del MPU-6050 → Arduino Pro Micro:**

| Pin MPU-6050 | Pin Arduino Pro Micro | Descripción |
|--------------|----------------------|-------------|
| VCC          | 3.3V                 | Alimentación (CRÍTICO: 3.3V, NO 5V) |
| GND          | GND                  | Tierra común |
| SDA          | 2 (SDA)              | Línea de datos I2C |
| SCL          | 3 (SCL)              | Línea de reloj I2C |
| AD0          | GND                  | Dirección I2C = 0x68 (default) |

**Nota importante:** Si tu breakboard del MPU-6050 no tiene resistencias pull-up integradas en SDA/SCL, necesitarás agregar resistencias de 10kΩ entre SDA↔VCC y SCL↔VCC.

### Paso 3: Cargar el Código

1. Abre **HE_Spacemouse.ino** en Arduino IDE
2. Selecciona **Tools** → **Board** → **Arduino AVR Boards (in Sketchbook)** → **Spacemouse**
3. Selecciona **Tools** → **Port** → (tu puerto USB)
4. Haz click en **Upload** (→)

### Paso 4: Verificar Funcionamiento

**Monitor Serial (115200 baud):**

```
set debug = 4 en el código para ver:
MPU-6050 initialized
Starting gyro calibration (hold still for 2 seconds)...
Gyro calibration complete. Offsets: 15.34, -8.92, 3.21
```

**Para diagnóstico de giroscopio:**

```cpp
int debug = 7;  // Ver datos de gyro en tiempo real
```

Salida esperada:
```
GX_DPS:0.52,GY_DPS:-0.18,GZ_DPS:0.02|GX_INT:12.34,GY_INT:-5.67,GZ_INT:0.43|RX:179,RY:-164,RZ:8
```

### Paso 5: Calibración en 3DConnexion Software

1. Abre **3DConnexion driver** (si está instalado)
2. Conecta el Arduino
3. Deberías ver un **SpaceMouse Pro Wireless** reconocido
4. Prueba rotaciones lentas y rápidas — deberían ser suaves y sin jitter
5. Compara con el comportamiento anterior (solo Hall) para verificar mejoría

---

## Parámetros Ajustables

### Factor de Fusión (GYRO_ALPHA)

En [HE_Spacemouse.ino](HE_Spacemouse.ino) línea ~61:

```cpp
const float GYRO_ALPHA = 0.97;  // 97% gyro, 3% Hall (drift correction)
```

**Cómo ajustar:**
- **Si oscila mucho:** Aumentar a 0.98 o 0.99 (más confianza en giroscopio)
- **Si tiene drift:** Disminuir a 0.95 o 0.93 (más corrección del Hall)
- **Rango recomendado:** 0.93–0.99

### Rango del Giroscopio

Actualmente configurado a **±250°/s** (línea ~106):

```cpp
mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
```

**Opciones:**
- `MPU6050_GYRO_FS_250` ← Default (menor ruido, suficiente para CAD)
- `MPU6050_GYRO_FS_125` (más sensible)
- `MPU6050_GYRO_FS_500` (rápidas rotaciones, más ruido)

---

## Solución de Problemas

### "MPU-6050 not found" (dirección 0x68 no detectada)

**Causa:** Problema con I2C

**Solución:**
1. Verifica conexiones SDA/SCL (pines 2/3 del Pro Micro)
2. Agrega resistencias pull-up 10kΩ si faltan
3. Usa 3.3V, NO 5V para VCC

### Giroscopio con drift evidente

**Causa:** Calibración incorrecta o temperatura

**Solución:**
1. Mantén el Arduino quieto durante calibración (~2s al startup)
2. Disminuye GYRO_ALPHA a 0.95 para más corrección Hall
3. Réinicia el Arduino en ambiente temperado

### Rotaciones lentas o sin respuesta

**Causa:** Posible problema de timing o integración

**Solución:**
1. Habilita `debug = 7` para verificar que gx_dps ≠ 0 en rotaciones
2. Comprueba que gyro_integrated[axis] cambia
3. Verifica que la fórmula complementary filter esté aplicándose

---

## Configuración Recomendada para CAD

```cpp
int debug = 5;              // Debug normal (hall + rotaciones)
const float GYRO_ALPHA = 0.97;   // Balance: suavidad vs. precisión
```

Para sesiones de tuning:
```cpp
int debug = 7;              // Ver datos de gyro en tiempo real
```

---

## Validación Técnica

| Aspecto | Estado | Notas |
|--------|--------|-------|
| **Memoria** | ✅ OK | ~3KB librería + 200B state |
| **I2C Pins** | ✅ OK | SDA=2, SCL=3 disponibles |
| **Compilación** | ✅ OK | Sin errores tras instalar librería |
| **Funcionalidad** | ✅ Listo | Calibración + fusión implementados |
| **Compatibilidad** | ✅ OK | Pro Micro + 3DConnexion compatible |

---

## Próximas Mejoras (Opcional)

1. **Acelerómetro:** Agregar datos de aceleración para tilt detection (estática)
2. **Calibración dinámica:** Re-calibrar giroscopio cada minuto para mayor precisión
3. **Kalman Filter:** Algoritmo más sofisticado que complementary filter (costo: +2KB memoria)

---

## Referencias

- **MPU-6050 Datasheet:** https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Datasheet1.pdf
- **Electronic Cats MPU6050 Library:** https://github.com/ElectronicCats/mpu6050
- **Complementary Filter:** https://en.wikipedia.org/wiki/Sensor_fusion#Complementary_filter

---

**Versión:** C010  
**Fecha:** May 19, 2026  
**Autor:** *JC (Hall-Effect sensor integration)
