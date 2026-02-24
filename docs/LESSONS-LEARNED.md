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

## 2026-02-23 - KiCad MCP Routed Demo Board

### What worked
- Running `sync_schematic_to_pcb` before routing propagated schematic connectivity to PCB pad nets and removed most manual net bookkeeping.
- MCP auto-routing pipeline (`autoroute`: preflight, DSN export, FreeRouting, SES import) produced a usable routed board quickly once preflight passed.
- Re-running MCP validation in a tight loop (`run_erc`, `run_drc`, `compare_schematic_pcb`) kept the board in a continuously verifiable state while making changes.
- Export tools (`export_gerbers`, `export_drill`, `export_bom`, `export_pick_and_place`) were stable and produced a complete demo manufacturing bundle.

### Problems encountered
- Auto-routing preflight rejected a valid rectangle board outline in `Edge.Cuts` as "open" because closed-shape geometry (for `gr_rect`) was not treated as a closed contour.
- A board can show `DRC` pass with `0` tracks if pads are still unrouted, so DRC alone is not enough to claim "working board" demo status.

### Workarounds used
- Updated MCP routing preflight logic in `D:/GitHub/KiCad_MCP/src/kicad_mcp/tools/routing.py` to recognize closed outline shapes in both direct and subprocess preflight paths.
- Added explicit board complexity metrics (`components`, `nets`, `tracks`) after routing to verify that the design is physically connected, not just rules-clean.
- Kept MCP log/change-log paths in repo-local `.kicad-mcp/logs` to avoid permission issues on user home directories in constrained environments.

### Follow-up recommendations
- Add a first-class MCP "routed board readiness" check that combines DRC with minimum routing completeness metrics.
- Add regression tests in KiCad MCP for rectangle (`gr_rect`) and polygonal `Edge.Cuts` outlines in routing preflight.
- Preserve generated manufacturing outputs (`gerbers`, `drill`, `BOM`, `PnP`) with validation reports for demo snapshots.
