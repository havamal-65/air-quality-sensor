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
- Schematic populated and wired (`16` placed symbols), ERC clean (`0` errors / `0` warnings)
- Schematic/PCB component consistency: matched (`16/16`), no footprint/value mismatches
- PCB nets synced from schematic via MCP (`56` pad net assignments)
- PCB routing completed via MCP auto-routing (`134` routed tracks)
- DRC clean (`0` errors / `0` warnings)
- Manufacturing outputs generated via MCP:
  - `manufacturing/gerbers/` (`13` files)
  - `manufacturing/drill/` (`2` files)
  - `manufacturing/air-quality-sensor-mcp-bom.csv`
  - `manufacturing/air-quality-sensor-mcp-pnp.csv`

## Open in KiCad
```powershell
"C:\Program Files\KiCad\9.0\bin\kicad.exe" "D:\GitHub\Portfolio\Projects\air-quality-sensor\hardware\kicad_mcp\air-quality-sensor-mcp.kicad_pro"
```
