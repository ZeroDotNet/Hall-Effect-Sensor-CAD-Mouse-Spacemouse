# Reporte Comparativo de Planes de Optimización

**Planes comparados:** `plan-claude.md` (Claude) vs `plan-codex.md` (Codex)  
**Proyecto:** `pio-migration/` — Firmware SpaceMouse con sensores Hall Effect

---

## Resumen Ejecutivo

Ningún plan es superior en todos los aspectos. `plan-codex.md` es más completo en alcance (8 fases vs 4), incluye pruebas unitarias y documentación — elementos críticos para firmware mantenible. `plan-claude.md` es más específico y accionable: cita líneas exactas de código, define firmas de función concretas e identifica dos bugs que `plan-codex.md` omite. La estrategia óptima es ejecutar un plan fusionado.

---

## Comparación por Dimensión

### 1. Cobertura de Bugs

| Bug | plan-claude.md | plan-codex.md |
|---|---|---|
| `keyChange` uint8_t overflow | ✅ Con línea exacta (línea 537) | ✅ Identificado |
| `configureAnalogReference()` ordering | ✅ Con orden correcto especificado | ❌ No mencionado |
| RP2040 pin duplication silenciosa | ✅ Fix + `#warning` + `hallSensorEnabled` | ⚠️ Mencionado como limitación, sin fix |
| `buttonReads` sin inicializar en `loop()` | ❌ No mencionado | ✅ Identificado |

**Ventaja: plan-claude.md** — detecta más bugs y proporciona correcciones concretas. `plan-codex.md` omite el bug de `configureAnalogReference()` y no propone un fix real para el RP2040.

---

### 2. Calidad de Código

| Ítem | plan-claude.md | plan-codex.md |
|---|---|---|
| Extraer helpers de debug | ✅ Con firmas definidas | ✅ Mencionado |
| Eliminar código comentado muerto | ✅ Con líneas específicas (491–512, 764–769) | ⚠️ Solo si el comportamiento está capturado en otro lugar |
| Const-correctness | ✅ Lista explícita de variables | ✅ Mencionado |
| Constantes nombradas para magic numbers | ✅ Con nombres concretos (`NUM_SENSORS`, etc.) | ❌ No mencionado |
| Estilo `if (flag == true)` | ✅ | ⚠️ Solo al tocar código adyacente |
| De-duplicar expresión `keyChange` | ✅ Con firma de helper propuesta | ✅ Incluido en bug fix |

**Ventaja: plan-claude.md** — más específico y accionable. El enfoque conservador de `plan-codex.md` ("solo al tocar código adyacente") deja pendiente deuda técnica innecesaria.

---

### 3. Arquitectura (Extracción de Módulos)

| Ítem | plan-claude.md | plan-codex.md |
|---|---|---|
| `config.h` | ✅ | ✅ (`Config.h`) |
| Abstracción HID por plataforma | ✅ Con `src_filter` en platformio.ini | ✅ (`HidDevice.h/.cpp`) |
| `sensors.h/.cpp` | ✅ Con 3 firmas definidas | ✅ |
| `buttons.h/.cpp` | ✅ Con variables de estado como `static` | ✅ |
| `movement.h/.cpp` | ✅ | ✅ (`Motion.h/.cpp`) |
| `debug_print.h/.cpp` | ✅ | ✅ (`DebugOutput.h/.cpp`) |
| `Settings.h/.cpp` | ❌ No separado | ✅ Módulo propio previo a EEPROM |
| `lib_deps` explícito para NicoHood/HID | ✅ | ❌ No mencionado |
| Firmas de función definidas | ✅ Para todos los módulos | ❌ Solo nombres de módulos |

**Ventaja: plan-claude.md** en especificidad. **Ventaja: plan-codex.md** por incluir `Settings.h/.cpp` como módulo propio — separación más limpia antes de agregar EEPROM.

---

### 4. Pruebas

| Ítem | plan-claude.md | plan-codex.md |
|---|---|---|
| Fase dedicada a pruebas unitarias | ❌ Ausente | ✅ Fase 4 completa |
| Tests de state machine de botones | ❌ | ✅ Con casos específicos |
| Tests de matemática de movimiento | ❌ | ✅ Con casos específicos |
| Tests de empaquetado HID | ❌ | ✅ |
| Entorno `native` en PlatformIO | ❌ | ✅ Mencionado |

**Ventaja: plan-codex.md** — esta es su diferencia más importante. Para firmware embebido, tener tests de la lógica pura (botones, movimiento, HID packing) antes de la extracción de módulos es invaluable. `plan-claude.md` carece completamente de esta fase.

---

### 5. EEPROM / Persistencia

| Ítem | plan-claude.md | plan-codex.md |
|---|---|---|
| Struct `StoredConfig` definido | ✅ Con todos los campos | ✅ Con campos equivalentes |
| API definida (`load/save/reset`) | ✅ Con firmas completas | ✅ Mencionado |
| Version byte | ✅ | ✅ |
| CRC / checksum de validación | ❌ Solo versión | ✅ Magic value + versión + CRC |
| Abstracción por plataforma | ✅ Con tabla AVR/RP2040/ESP32 | ✅ Con verificación de compatibilidad |
| Desgaste de EEPROM (write wear) | ❌ No mencionado | ✅ Guardar solo en paths explícitos |
| Backend stub para plataformas no validadas | ❌ | ✅ Con documentación explícita |

**Ventaja: plan-codex.md** — el CRC/checksum y la consideración de write wear son mejoras significativas de robustez. Un solo version byte no detecta corrupción parcial de EEPROM.

---

### 6. Fases Adicionales

`plan-codex.md` incluye tres fases que `plan-claude.md` no tiene:

**Fase 6 — Configuración board-aware via `platformio.ini`:**  
Permite ajustar `DEADZONE`, `movement3DC`, `debug` con `-D SPACEMOUSE_DEADZONE=40` sin editar C++. Esto es ergonómicamente superior a editar `config.h` para cada plataforma.

**Fase 7 — Portabilidad HID:**  
Reconoce explícitamente que build exitoso ≠ USB compatible. Valida enumeración real con 3DConnexion software por plataforma. Importante porque RP2040 y ESP32-S3 no tienen VID/PID configurado consistentemente.

**Fase 8 — Documentación:**  
Actualiza `pio-migration/README.md` con tabla de soporte por plataforma, pin mapping, debug modes, y checklist de validación de hardware.

---

## Tabla de Veredicto por Categoría

| Categoría | Ganador | Razón |
|---|---|---|
| Bugs identificados | plan-claude.md | +2 bugs detectados y corregidos |
| Especificidad / accionabilidad | plan-claude.md | Líneas exactas, firmas de función |
| Alcance y completitud | plan-codex.md | 8 fases vs 4, cubre testing y docs |
| Pruebas unitarias | plan-codex.md | Fase completa, ausente en plan-claude |
| Robustez EEPROM | plan-codex.md | CRC + write wear consideration |
| Configuración por plataforma | plan-codex.md | `build_flags` en platformio.ini |
| Validación HID multi-plataforma | plan-codex.md | Fase dedicada |
| Documentación | plan-codex.md | Fase dedicada |

---

## Recomendación: Plan Fusionado

El plan a ejecutar debe tomar lo mejor de ambos:

### De plan-claude.md incorporar:
- Los 3 bug fixes específicos (incluyendo `configureAnalogReference` y RP2040 fix con `hallSensorEnabled`)
- Las líneas exactas de código muerto a eliminar
- Las firmas de función concretas para cada módulo
- `lib_deps = NicoHood/HID` explícito en `platformio.ini`
- `NUM_SENSORS`, `NUM_BTNS`, `NUM_BTN_VALUES` como constantes nombradas

### De plan-codex.md incorporar:
- `Settings.h/.cpp` como módulo propio antes de EEPROM
- **Fase de pruebas unitarias** (Phase 4) — la omisión más importante de plan-claude.md
- CRC/checksum en EEPROM además del version byte
- Consideración de write wear en EEPROM
- Fase de configuración board-aware via `platformio.ini` build_flags
- Fase de validación HID por plataforma con registro de resultados reales
- Fase de documentación de `pio-migration/README.md`
- Backend stub con documentación para plataformas no validadas en EEPROM

### Orden de fases recomendado (8 fases):

| Fase | Contenido | Fuente |
|---|---|---|
| 1 | Bug fixes (todos los identificados en ambos planes) | Fusión |
| 2 | Code quality cleanup | Fusión (especificidad de Claude, conservadurismo de Codex donde aplique) |
| 3 | Extracción modular pesada | Fusión (+Settings.h de Codex, +firmas concretas de Claude) |
| 4 | Pruebas unitarias de lógica pura | plan-codex.md |
| 5 | EEPROM persistencia | Fusión (+CRC de Codex, +API concreta de Claude) |
| 6 | Configuración board-aware via build_flags | plan-codex.md |
| 7 | Validación HID multi-plataforma | plan-codex.md |
| 8 | Documentación README | plan-codex.md |

---

*Reporte generado el 2026-05-17*
