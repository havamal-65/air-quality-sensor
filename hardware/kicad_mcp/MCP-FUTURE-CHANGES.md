# MCP-Only Future Changes

Date: 2026-02-22

This project requires schematic/PCB changes to be made via KiCad MCP workflows only.
Direct manual edits to `.kicad_sch`/`.kicad_pcb` are out of scope.

## Current baseline (KiCad 9 only)

- `kicad-cli` used for validation: `9.0.7`
- Schematic file generator version: `9.0`
- PCB file generator/version references: KiCad 9 (`KICAD9_3DMODEL_DIR`)

There is no evidence in this project fileset of KiCad 8 being invoked.

## Validation snapshot

### ERC (`hardware/kicad_mcp/air-quality-sensor-mcp.kicad_sch`)
- `2` errors, `0` warnings
- Errors:
  - `U1` pin `14` (`5V`) is a power input not driven by a power output.
  - `J1` pin `1` (`VBAT`) and `U5` pin `3` (`B+`) are both `power_out`.

### DRC (`hardware/kicad_mcp/air-quality-sensor-mcp.kicad_pcb`)
- `59` violations
- Major blockers are structural:
  - Multiple `<no net>` pads on placed components
  - Footprint/courtyard overlaps near `U1` and passives (`R1/R2/C1/C2/C3`)
  - Edge clearance issue at `U5`
  - Resulting solder mask bridge/clearance errors

## Required MCP fixes (not manual board edits)

1. Add/repair MCP step to propagate schematic nets into PCB pads.
1. Add MCP placement rules/checks to prevent courtyard overlap and edge violations.
1. Add MCP routing phase for required nets, then run DRC and enforce zero hard errors.
1. Add MCP ERC handling for power-pin semantics (power flags or pin-type policy).
1. Keep MCP execution pinned to KiCad 9 binaries/libs in one configured path.

## Definition of done

1. ERC: `0` errors (warnings only if explicitly accepted).
1. DRC: `0` errors, no unresolved unconnected required nets.
1. Schematic/PCB sync check shows net and component consistency.
