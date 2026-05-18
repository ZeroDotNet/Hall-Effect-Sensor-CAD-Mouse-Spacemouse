# Open Decisions

## 1. Imanes definitivos

Opciones:

1. Usar imanes originales 10×5×3 mm.
2. Adaptar los 11×8×1.7 mm disponibles.
3. Usar 6×2 mm solo para prueba.
4. Rediseñar magnet holder si los 11×8×1.7 mm funcionan bien.

Recomendación actual:
**Probar 11×8×1.7 mm temporalmente antes de comprar o rediseñar.**

## 2. Capacitor OUT-GND

Ya hay 100nF entre VCC y GND.

Pendiente:
- Decidir si agregar 100nF entre OUT y GND para cada sensor.

Recomendación:
- No agregar hasta medir ruido real.
- Si las lecturas saltan mucho, agregarlo cerca del sensor.

## 3. Usar ADC externo

ADS1015 disponible, pero no recomendado en primera versión.

Decisión pendiente:
- Mantener ADC interno ATmega32U4.
- Solo migrar a ADC externo si la resolución/ruido lo exige.

## 4. Variante mecánica

Opciones:
- Resortes originales.
- TPU spring.
- TPU wave spring.
- Híbrido.

Recomendación:
- Terminar versión funcional con la arquitectura actual.
- Evaluar TPU luego de validar electrónica.

## 5. Repo definitivo

Todavía falta confirmar el repositorio GitHub exacto donde debe vivir la carpeta `doc/`.

Candidato esperado:
- Un repo específico de Spacemouse, si existe.
- Si no existe, crear uno o elegir el repo técnico correcto.

No publicar en un repo no confirmado.
