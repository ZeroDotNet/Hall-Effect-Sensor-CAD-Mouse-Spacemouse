# Spacemouse — Paquete de traspaso

Fecha de export: 2026-05-18

Este paquete permite migrar el proyecto **Spacemouse** a otro agente, repo, asistente o sistema de documentación sin depender de la conversación original.

## Objetivo

Construir un **DIY SpaceMouse / CAD Mouse 6DoF** basado principalmente en el proyecto **ChromeBee Hall-Effect-Sensor-CAD-Mouse-Spacemouse**, usando:

- Arduino Pro Micro / ATmega32U4.
- 8 sensores Hall lineales 49E / SS49E.
- Imanes de neodimio.
- Mecanismo elástico con resortes / TPU.
- Emulación HID compatible con el flujo de driver 3DConnexion usado por proyectos open source similares.

## Estado actual del proyecto de Nico

- Sensores Hall ya montados.
- Configuración física: **2 sensores por lado**.
- Montaje actual: **4 perfboards**, una por cada par de sensores.
- Ya se colocaron capacitores cerámicos de **100nF entre VCC y GND**.
- Se está trabajando en la conexión final hacia Arduino Pro Micro.
- Punto crítico: revisar bien el esquema de conexión antes de soldar/cerrar el montaje.

## Archivos incluidos

| Archivo | Uso |
|---|---|
| `PROJECT_CONTEXT.md` | Resumen completo del proyecto |
| `HARDWARE_BOM.csv` | Lista de materiales y hardware confirmado |
| `WIRING_MAP.md` | Cableado recomendado y mapa de sensores |
| `SENSOR_PINOUT.md` | Pinout confirmado de sensores Hall de Nico |
| `MECHANICAL_NOTES.md` | Notas mecánicas, imanes, distancia y montaje |
| `FIRMWARE_NOTES.md` | Firmware, HID, debug y compatibilidad |
| `CALIBRATION_TEST_PLAN.md` | Plan de test y calibración |
| `OPEN_DECISIONS.md` | Decisiones pendientes |
| `NEXT_STEPS.md` | Próximos pasos ordenados |
| `agent_prompt.md` | Prompt listo para otro agente |
| `project_context.json` | Contexto estructurado para herramientas/LLM |
| `project_context.yaml` | Contexto estructurado en YAML |
| `GITHUB_PUBLISH_NOTES.md` | Notas para publicar en GitHub |
| `CHANGELOG.md` | Historial de este export |
