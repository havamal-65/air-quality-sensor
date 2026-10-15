# AirSense v4 — Board design rules (ticket 03, AC-2)

**Date applied:** 2026-10-13
**Source:** PCBWay "PCB Capabilities — Standard PCB" page,
https://www.pcbway.com/capabilities.html — fetched 2026-10-13.
**Tier:** Standard 2-layer FR-4.

The project was created with the `fab_jlcpcb` preset, which is known
defective (it carries JLCPCB multilayer via limits — see the open
KiCad-MCP lesson "A fab preset must encode the limits of the exact
service tier it names"). All values below were written explicitly with
`set_board_design_rules(preset="")`, replacing the preset, before the
board was first opened in pcbnew.

## DRC minimums (board.design_settings.rules)

| Rule | Applied | PCBWay published limit (2026-10-13) | Margin |
|---|---|---|---|
| Min track width | 0.15 mm | 0.1 mm (4 mil) | 1.5× |
| Min clearance (copper–copper) | 0.15 mm | 0.1 mm (4 mil) | 1.5× |
| Min via diameter | 0.6 mm | no separate pad min published; annular ring 0.15 mm + drill ⇒ 0.6 with 0.3 drill | meets ring |
| Min via annular width | 0.15 mm | 0.15 mm (6 mil) | at limit |
| Min through-hole drill | 0.2 mm | 0.15 mm (holes < 0.2 mm carry a surcharge) | above surcharge line |
| Min hole-to-copper clearance | 0.15 mm | **not published by PCBWay**; ≥ their 0.1 mm copper spacing | see note |
| Min copper-to-edge clearance | 0.3 mm | 0.25 mm (Advanced table, CNC milling) | 1.2× |
| Min hole-to-hole | 0.25 mm | not published | conservative |

**Hole-to-copper note:** PCBWay's capabilities page states no
NPTH-to-copper limit (checked 2026-10-13; a web search found no PCBWay
figure either — 0.2–0.3 mm is the common industry guideline from other
fabs). The stock KiCad USB-C footprint
(`USB_C_Receptacle_HRO_TYPE-C-31-M-12`) has a fixed internal
NPTH-to-copper distance of 0.1501 mm, which is manufacturer geometry on
a mass-produced connector. The rule is therefore set to 0.15 mm so this
footprint passes; all board-level routing additionally respects the
0.15 mm copper clearance.

## Default netclass (routing values)

| Value | Applied |
|---|---|
| Clearance | 0.15 mm *(0.2 until 2026-10-13, see note)* |
| Track width | 0.25 mm |
| Via diameter | 0.7 mm |
| Via drill | 0.35 mm |

**Netclass clearance note (2026-10-13):** the initial 0.2 mm netclass
clearance made DRC flag the TPS61023's stock SOT-563 footprint — TI's
own land pattern has fixed 0.15 mm gaps between adjacent pads (0.5 mm
pitch). The netclass clearance was lowered to 0.15 mm, equal to the
DRC minimum above and 1.5× PCBWay's published 0.1 mm spacing limit.
This is a rule the chosen parts can physically satisfy; it is not a
relaxation below any fab limit.

All netclass values sit at or above the DRC minimums, which sit at or
above every limit PCBWay publishes for the standard 2-layer tier.
