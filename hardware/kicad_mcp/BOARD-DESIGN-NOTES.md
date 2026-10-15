# AQS Board — Design Notes & Open Items

Board-specific status and open design items for `air-quality-sensor-mcp`, captured during the
2026-06 work session (adding the PMSA003I, then the U1 through-hole + 5V-boost power respin).

KiCad MCP *tooling* issues hit during this work (footprint-library resolution, bridge
instability, autosave clobbering, etc.) are tracked separately in the KiCad-MCP repo:
`D:\GitHub\KiCad-MCP\MCP-KNOWN-ISSUES.md`. Issue numbers below (e.g. #7, #9, #14) refer to
entries in that file.

## Status / resolved

- **U1 footprint swap — RESOLVED (2026-06, second attempt).** Swapped to
  `Seeed_XIAO:XIAO-ESP32C3-DIP` (through-hole, for soldered header pins) by a **direct
  `.kicad_pcb` line-splice** (user-approved; board-footprint instance built from the official
  Seeed DIP `.kicad_mod`, preserving U1's position/UUID). First attempt was silently reverted
  by autosave recovery (MCP issue #14); the second attempt followed the validated recovery
  sequence documented there.
  Final verified-on-disk state: U1 DIP present / SMD gone, **160 routed tracks**, 41 THT pads,
  DRC `0` errors / `0` warnings / `0` unconnected / `0` schematic-parity, `0` courtyard
  overlaps. Manufacturing outputs regenerated from this board. Temp scripts/backups removed.
- **5V-boost power tree (done in schematic, ERC 0 errors):** LiPo → TP4056 → `VBAT` → U7 boost →
  `+5V` → { PMSA003I VIN ; D1 Schottky → XIAO `5V` pin }. XIAO `B+`/`B-` no-connect (unreachable
  when header-mounted). `USB_5V` and `+5V` carry PWR_FLAGs. D1 = SS14, U7 = fixed 5V boost
  (TPS61023-class) on a 1×03 header, always-on.
- **PMSA003I VIN** moved from 3.3 V to the new 5 V rail (better for the fan/laser).
- Upstream footprint caveat (confirmed on the Seeed forum): the official XIAO ESP32C3 KiCad
  footprints **omit the B+/B- battery pads** — fine here, since header-mounting makes B+/B-
  unreachable anyway (no-connected; the XIAO is powered via its 5V pin).

## Known cosmetic residuals

- D1/U6/U7 board `Value` fields show footprint names instead of part names (MCP issues #7/#14
  make fixing them not worth the clobber risk). The **BOM is generated from the schematic**,
  which carries the correct values (SS14, Adafruit_PMSA003I, Boost_5V_TPS61023).
- **7 cosmetic off-grid ERC warnings** on D1/U7 stubs (MCP issue #9) — tidy by re-placing on grid.

## Open items

- **Verify boost power budget:** PMSA003I fan (~100 mA @5V) + XIAO+sensors (via 5V→3V3 LDO).
  Boost must carry ~250–300 mA @5V. Consider GPIO-gating the boost/PM fan for battery life (deferred).
