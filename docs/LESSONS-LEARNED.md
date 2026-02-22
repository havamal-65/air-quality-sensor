# Lessons Learned

## 2026-02-21 - KiCad MCP Design Pass

### What worked
- Building a new KiCad project in a separate folder (`hardware/kicad_mcp`) avoided disturbing the existing `hardware/kicad` design.
- For schematic generation, KiCad MCP `file` backend was reliable for:
  - creating schematic/project files
  - adding symbols, wires, labels, and no-connect markers
  - running ERC and schematic/PCB comparison checks
- Using explicit net labels (`I2C_SDA`, `I2C_SCL`, `+3V3`, `GND`, `VBAT`, `USB_5V`) made automated verification easier and reduced ambiguous nets.

### Problems encountered
- `place_component` with Windows-style absolute paths (backslashes) can fail in `kicad_mcp` with regex escape errors.
- Project-library footprint IDs (for custom footprints) were not always resolved for geometry insertion when placing directly by `Library:Footprint` name in `file` backend.
- Some generated temporary files were created during KiCad/MCP runs (`*.lck`, `fp-info-cache`, backup zips, local MCP log folders).

### Workarounds used
- Use POSIX-style absolute footprint paths (`D:/...`) when placing custom footprints via MCP file backend.
- After placement, normalize footprint IDs back to project library IDs (for readability/portability).
- Add `.gitignore` rules for `hardware/kicad_mcp` temp artifacts and `.kicad-mcp/`.
- Keep schematic and PCB in sync with:
  - `run_erc`
  - `compare_schematic_pcb`
  - `sync_schematic_to_pcb` (value/property consistency)

### Follow-up recommendations
- Prefer running with IPC backend for richer KiCad-native operations when a stable KiCad session is available.
- Add an explicit routing phase checklist (net assignment, tracks, vias, DRC) before manufacturing outputs.
- Keep one canonical hardware target directory and archive previous attempts to reduce drift.
