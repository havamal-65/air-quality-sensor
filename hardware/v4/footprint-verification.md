# AirSense v4 — Footprint verification record (ticket 01, AC-3)

**Date:** 2026-10-12
**Method:** Every footprint compared dimension-by-dimension against the
manufacturer land-pattern drawing. Drawings were rendered from the
manufacturer PDFs and read page-by-page. Authored footprints were verified
two ways after import: (1) programmatic pad-table parse of the imported
`.kicad_mod` against the drawing values — all pads PASS exact; (2) visual
render via `kicad-cli fp export svg` — geometry, pin-1 marks, paste
apertures confirmed.

**Project library:** `airsense_v4.pretty` + `airsense_v4.kicad_sym`,
registered in `fp-lib-table` / `sym-lib-table` with `${KIPRJMOD}` paths.

---

## 1. SCD41-D-R2 (CO2) — stock `Sensor:Sensirion_SCD4x-1EP_10.1x10.1mm_P1.25mm_EP4.8x4.8mm`

Drawing: Sensirion SCD4x datasheet v1.5 July 2023, Figure 4 (page 22).

| Dimension | Drawing | Footprint | Verdict |
|---|---|---|---|
| Perimeter pad pitch | 1.25 | 1.25 | PASS |
| Pads per side / numbering | 5 per side, pin 1 top-left, counterclockwise | same (1–5 left top→bottom, 6–10 bottom, 11–15 right, 16–20 top) | PASS |
| Perimeter pad size | 1.50 × 0.80 | 1.90 × 0.85 | PASS with deviation — IPC fillet elongation (+0.4 length toward board edge and EP, +0.05 width); official KiCad library practice; Sensirion note: geometries "must be adapted to the customer soldering processes" |
| Center pad 21 (EP) | 4.80 × 4.80 | 4.80 × 4.80 | PASS exact |
| EP paste | 4 windows ~1.8 sq | 4 windows 1.93 sq (~65% coverage) | PASS (equivalent coverage) |
| Pattern extent | 10.10 | 10.10 (pad outer edges ±5.05 → 5.10 with elongation) | PASS |

## 2. SGP41-D-R4 (VOC/NOx) — AUTHORED `airsense_v4:Sensirion_SGP41_DFN-6-1EP_2.44x2.44mm_P0.8mm`

Drawing: Sensirion SGP41 datasheet v1.0 Dec 2021, Figure 18 (page 19) +
Figure 17 (page 18) + Table 6 (page 7). No SGP footprint exists in the
stock KiCad 10 libraries (checked: Sensor.pretty, Sensor_Humidity DFN-4
candidates are 2×2 mm SHT4x — wrong package).

**REWORK 2026-10-13 (found by Phase-5 post-route DRC):** the original
authoring read Figure 18's 2.3 mm span as pad OUTER-edge span and placed
side-pad centers at x=±0.875 — that put the die pad (half-width 0.625)
0.025 mm INTO the side pads' inner edges (0.6), shorting pads 3/5/6 to
the GND die pad on the board. Pixel-measurement of Figure 18 (217 px/mm,
calibrated on the 1.05 stencil dimension) shows the 2.3 extension lines
drop from the pad CENTERS: correct centers are x=±1.15, giving the
0.25 mm pad-to-die-pad gap the drawing shows. Fixed in the library and
re-verified with a pairwise pad-gap check (all 21 pairs ≥ 0.25 mm).
Lesson: a transcription check cannot catch a misread datum line — the
verification must also compute inter-pad gaps and fail on ≤ 0.

| Dimension | Drawing | Footprint | Verdict |
|---|---|---|---|
| Side pads (6) | 0.55 × 0.4, pitch 0.8, span 2.3 center-to-center | 0.55 × 0.4 at x=±1.15, y = −0.8/0/+0.8 (outer edge ±1.425) | PASS (corrected 2026-10-13; pairwise gaps all ≥ 0.25) |
| Pin order | 1 VDD, 2 VSS, 3 SDA left top→bottom; 6 SCL, 5 VDDH, 4 n/a right top→bottom; pin-1 top-left (Table 6) | same | PASS |
| Die pad (GND, pad 7) | 1.25 × 1.7 | 1.25 × 1.7 | PASS exact |
| Die-pad paste aperture | 1.05 × 1.5 | paste margin −0.1 → 1.05 × 1.5 | PASS exact |
| Pin-1 chamfer on die pad | 0.3 × 45° | chamfer_ratio 0.24 (= 0.3/1.25), top-left | PASS |
| Mask type | NSMD recommended | copper-defined pads, board-level mask clearance (NSMD behavior) | PASS |

Assembly constraints carried to the build record: stencil 125–150 µm;
reflow ≤245 °C ≤30 s; NO vapor phase, NO manual soldering, NO board wash
or ultrasonic cleaning; place SGP41 in the LAST solder cycle.

## 3. SEN50 cable connector — stock `Connector_JST:JST_ZH_B6B-ZR-SM4-TF_1x06-1MP_P1.50mm_Vertical`

Drawing: JST eZH.pdf, "PC board layout / SMT type / SM4 type", top entry
(page 2).

| Dimension | Drawing | Footprint | Verdict |
|---|---|---|---|
| Signal pad pitch | 1.5 ±0.05 | 1.5 | PASS |
| Signal pad size | ≥0.7 wide ("0.7 min."), ≥3.7 long ("3.7 min.") | 0.7 × 3.7 | PASS |
| Tab (MP) pad | 1.5 wide × 2.3 ±0.1, near edge 1.3 ±0.1 from last signal pad center → center at ±5.8 | 1.5 × 2.3 at x=±5.8 | PASS exact |
| Signal-top to tab-bottom offset | 0.9 ±0.1 | 0.9 | PASS exact |

## 4. Pressure sensor — AUTHORED `airsense_v4:Bosch_BMP390_LGA-10_2x2mm`

Drawing: Bosch BMP390 datasheet BST-BMP390-DS002-07 rev 1.7, Figure 26
bottom view + Figure 23/Table 52 (pages 46, 49). Section 7.2: outline
dimensions used 1:1 as landing pattern. No Bosch LGA-10 2×2 exists in
stock libs (LGA-10 candidates are ST/AMS/Rohm layouts — wrong).

| Dimension | Drawing (bottom view, mirrored to top) | Footprint | Verdict |
|---|---|---|---|
| Side-column pads (6): 3/4/5 left, 8/9/10 right | 0.275 × 0.250, pitch 0.5, columns 1.525 c-c, 0.100 from edge | 0.275 × 0.250 at x=±0.7625, y=−0.5/0/+0.5 | PASS exact (programmatic: all PASS) |
| Row pads (4): 1/2 top, 6/7 bottom | 0.250 × 0.275, 0.500 c-c, rows at ±0.7625 | 0.25 × 0.275 at x=±0.25, y=±0.7625 | PASS exact |
| Pin-1 position | top-right in top view (1 VDDIO right of 2 SCK) | pad 1 at (+0.25, −0.7625), silk dot top-right | PASS |
| Pin map | 1 VDDIO, 2 SCK/SCL, 3 VSS, 4 SDI/SDA, 5 SDO/SA0, 6 CSB, 7 INT, 8/9 VSS, 10 VDD | same numbering | PASS |

**BMP384 compatibility (substitution insurance):** BMP384 datasheet
BST-BMP384-DS003-11 verified directly — identical pin-out (Table 50),
same 10-pin metal-lid LGA 2.0×2.0 (0.95 high vs 0.75), landing pattern
within 5–20 µm of BMP390 (columns 1.515 c-c vs 1.525; pads 0.285×0.270
vs 0.275×0.250), inside Bosch's own ±50 µm general tolerance. This
footprint serves either part unchanged.

## 5. TPS61023DRLR (5 V boost) — stock `Package_TO_SOT_SMD:SOT-563`

Drawing: TI TPS61023 datasheet SLVSF14B, DRL0006A "LAND PATTERN EXAMPLE"
(page 26).

| Dimension | Drawing | Footprint | Verdict |
|---|---|---|---|
| Pad pitch | 0.5 | 0.5 | PASS exact |
| Pad size | 0.67 × 0.3 | 0.675 × 0.35 | PASS with deviation (+0.005/+0.05, IPC alternate; TI note 5 allows) |
| Column span | 1.48 c-c | 1.425 c-c (outer edge 1.05 vs TI 1.075) | PASS with deviation ≤0.055 |
| Numbering | 1 top-left, 3 bottom-left, 4 bottom-right, 6 top-right | same | PASS |
| Mask | NSMD preferred | copper-defined, board mask clearance | PASS |

## 6. OLED header — stock `Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical`

Generic 2.54 mm header (0.64 mm square posts): pitch 2.54 exact, pad 1.7,
drill 1.0 (> 0.91 post diagonal). PASS — generic part, no manufacturer
drawing applies.

---

## Pressure-part stock re-check (ticket requirement, checked 2026-10-12)

- **BMP390** (pinned in spec): DigiKey — out of stock, backorder, 16-week
  manufacturer lead. Mouser — conflicting signals (aggregator claims
  ~16k in stock; a 62-day-old EEVblog report says majors show 2027
  delivery). Distributor pages block direct live reads; signals are
  cached snapshots.
- **BMP390L**: obsolete. **BMP388**: obsolete (EOL 2021).
- **BMP384**: consistently stocked in multiple snapshots (DigiKey ~7k,
  Mouser ~13.8k T&R), ~$3.50 at qty 1. DigiKey's own suggested
  substitute for BMP388. Same footprint, same pin-out, same Bosch BMP3
  firmware API; relative accuracy ±9 Pa vs BMP390's ±3 Pa (both fine
  for ambient indication). Verified against its own datasheet, not
  assumed.

**Layout impact: none** — the authored footprint and schematic pin map
cover both parts. The buy decision (BMP390 if live stock exists at order
time, else BMP384) lands in ticket 04 (order package) and needs user
confirmation since the spec pins BMP390.

## Verification evidence

- Programmatic: pad-table parse of both imported `.kicad_mod` files vs
  drawing values — 17/17 pads PASS exact (2026-10-12).
- Visual: `kicad-cli fp export svg` renders of both authored footprints
  read and confirmed (pads, pin-1 marks, chamfer, paste aperture,
  courtyard) (2026-10-12).
- Stock footprints: pad tables extracted from the installed KiCad 10
  library files and compared against the rendered manufacturer drawings
  above (2026-10-12).
