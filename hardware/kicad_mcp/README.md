# KiCad MCP Design

This directory contains the KiCad MCP design iteration for this project (updated 2026-02-21/2026-02-22).

## Primary files
- `air-quality-sensor-mcp.kicad_pro`
- `air-quality-sensor-mcp.kicad_sch`
- `air-quality-sensor-mcp.kicad_pcb`
- `sym-lib-table`
- `fp-lib-table`

## Validation outputs
- `mcp_build_summary.json` - initial PCB placement summary
- `mcp_schematic_build_summary.json` - schematic build + ERC + schematic/PCB compare summary

## Current status
- Schematic populated and explicitly wired (`34` wire segments)
- ERC (`kicad-cli`): `2` errors / `0` warnings (power-rule semantics on `USB_5V` input drive and `BAT+` power_out-to-power_out)
- Schematic/PCB component consistency: matched (16/16)
- PCB component placement complete
- PCB routing: not started (tracks pending)

## Open in KiCad
```powershell
"C:\Program Files\KiCad\9.0\bin\kicad.exe" "D:\GitHub\Portfolio\Projects\air-quality-sensor\hardware\kicad_mcp\air-quality-sensor-mcp.kicad_pro"
```
