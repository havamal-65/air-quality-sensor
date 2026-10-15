# KiCad MCP Design

This directory contains the KiCad MCP design iteration for this project (updated 2026-02-23).

## Primary files
- `air-quality-sensor-mcp.kicad_pro`
- `air-quality-sensor-mcp.kicad_sch`
- `air-quality-sensor-mcp.kicad_pcb`
- `sym-lib-table`
- `fp-lib-table`

## Validation outputs
- `mcp_build_summary.json` - initial PCB placement summary
- `mcp_schematic_build_summary.json` - schematic build + ERC + schematic/PCB compare summary
- `erc-mcp-validation.json` - latest ERC report from MCP `run_erc`
- `drc-mcp-validation.json` - latest DRC report from MCP `run_drc`
- `MCP-FUTURE-CHANGES.md` - MCP-only remediation backlog and validation baseline

## Current status
- Schematic populated and wired (`19` placed symbols), ERC `0` errors (`7` cosmetic off-grid warnings on the D1/U7 auto-stubs)
- Components: U1 XIAO ESP32-C3 (**through-hole DIP** footprint for soldered header pins), U2 SCD41, U3 SGP41, U4 SSD1306, U5 TP4056, **U6 Adafruit PMSA003I PM2.5**, **U7 5 V boost (TPS61023-class)**, **D1 Schottky (SS14)**, J1 JST-PH, R1/R2 I²C pull-ups, R3/R4, C1–C4, LED1/LED2
- Power tree (battery-only wearable): LiPo → TP4056 (charge) → `VBAT` → U7 boost → `+5V` → { PMSA003I `VIN` ; D1 Schottky → XIAO `5V` pin }. Sensors/OLED on `+3V3` from the XIAO regulator. XIAO `B+`/`B-` no-connect (unreachable on header pins)
- Schematic/PCB consistency: `19/19` present, `0` footprint mismatches. Cosmetic only: D1/U6/U7 PCB `Value` fields show the placed footprint name (autoroute SES-import resets them); the schematic + BOM carry the real part names — so the **BOM is generated from the schematic**
- PCB nets synced from schematic via MCP (`65` pad net assignments)
- PCB routing completed via MCP auto-routing (`160` routed tracks); DRC clean (`0` errors / `0` warnings, `0` unconnected, `0` schematic-parity); `0` courtyard overlaps. U1 through-hole DIP confirmed on the board (41 THT pads total incl. U1's 14)
- Manufacturing outputs regenerated via MCP:
  - `manufacturing/gerbers/` (incl. NPTH + PTH drills for the through-hole pins)
  - `manufacturing/drill/`
  - `manufacturing/air-quality-sensor-mcp-bom.csv` (from schematic)
  - `manufacturing/air-quality-sensor-mcp-pnp.csv`
- See `BOARD-DESIGN-NOTES.md` for board-specific status/open items (incl. the U1 footprint-swap method via direct board edit). MCP tooling issues hit during this work are tracked in the KiCad-MCP repo: `D:\GitHub\KiCad-MCP\MCP-KNOWN-ISSUES.md`.

## Open in KiCad
```powershell
"C:\Program Files\KiCad\9.0\bin\kicad.exe" "D:\GitHub\Portfolio\Projects\air-quality-sensor\hardware\kicad_mcp\air-quality-sensor-mcp.kicad_pro"
```
