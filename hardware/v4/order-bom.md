# AirSense v4 — Order BOM (ticket 04)

**Date of price check:** 2026-10-13 (all prices are live-checked snapshots from
this date; distributor pages cache and change — confirm each line in the cart
before paying).
**Build target:** one staged unit, plus spares on cheap passives and the two
substituted discretes.
**Currency:** USD. Qty-1 prices unless stated.

## Decisions baked into this BOM

1. **BMP384 replaces BMP390** (user decision 2026-10-13). BMP390 is 16-week
   backorder at DigiKey; BMP384 is footprint-, pinout-, and API-identical
   (verified against both datasheets in ticket 01). U7 value updated in the
   schematic and board.
2. **CDBW140-G replaces 1N5819HW-7-F** (D1, D2). The 1N5819HW-7-F is out of
   stock at DigiKey with a 40-week lead; the Comchip CDBW140-G is the same
   class (Schottky, 40 V, 1 A, SOD-123) with ~600 k in stock at $0.16. D1/D2
   values updated in the schematic and board.
3. **NRS4018T1R0NDGJ replaces NR4018T1R0N** (L1). The NR4018 series passed its
   last-time-buy date (2026-03-31); the NRS4018 is Taiyo Yuden's successor in
   the same 4.0 × 4.0 mm land pattern, 1 µH. No design change needed.

## Order 1 — DigiKey

| Ref(s) | Part | MPN | Order qty | Unit price | Line | Stock seen 2026-10-13 |
|---|---|---|---|---|---|---|
| U5 | CO2/T/RH sensor | SCD41-D-R2 | 1 | $21.77 | $21.77 | ~12,956 |
| U6 | VOC/NOx sensor | SGP41-D-R4 | 1 | $9.26 | $9.26 | ~2,639 |
| U7 | Pressure sensor | BMP384 | 2 | $3.52 | $7.04 | ~7,028 |
| — | **PM sensor module** | **SEN50-SDN-T** | 1 | $22.10 | $22.10 | **~300–700 — THIN. ORDER FIRST.** |
| U4 | MCU module | ESP32-C3-WROOM-02-N4 | 2 | $3.38 | $6.76 | ~18,374 |
| U3 | 5 V boost | TPS61023DRLR | 2 | $1.22 | $2.44 | ~24,559 |
| U1 | LiPo charger | MCP73831T-2ACI/OT | 2 | $0.76 | $1.52 | ~33,000+ |
| U2 | 3.3 V LDO | AP2112K-3.3TRG1 | 2 | $0.28 | $0.56 | ~43,000+ |
| D1, D2 | Schottky SOD-123 | CDBW140-G | 10 | $0.099 @10 | $0.99 | ~600,370 |
| L1 | 1 µH 4×4 inductor | NRS4018T1R0NDGJ | 2 | $0.36 | $0.72 | ~4,590 |
| LED1 | 0603 green LED | LTST-C190KGKT | 10 | ~$0.05 @10 | $0.50 | ~148,000 (20-wk mfr lead — buy from stock) |
| J2 | JST PH 2-pin SMT | S2B-PH-SM4-TB(LF)(SN) | 2 | $0.57 | $1.14 | ~14,000 |
| J3, J5 | 1×04 2.54 mm male header | generic breakaway strip (any 1×40, snap to 4) | 2 strips | ~$0.60 | $1.20 | commodity |
| — | 1×04 2.54 mm female socket (OLED mates J5) | PPPC041LFBN-RC | 2 | ~$0.39 | $0.78 | ~6,000 (aggregate) |
| R, C, 0R | passives, see jellybean table below | Yageo RC0603 / CC0603-0805, KEMET/Samsung | 10 of each value (14 values) | ~$0.10 avg | ~$14.00 | commodity |

**DigiKey subtotal ≈ $90.80** (of which spares/stock ≈ $25).

**SEN50 warning (spec flag, re-confirmed 2026-10-13):** DigiKey showed only
~300–700 units (snapshots disagree: 286, 721), Mouser ~107, 5–15 week factory
lead. This is the thin line of the whole build — place the SEN50 order before
or with everything else, not after.

### Passive values (10 pcs each, 0603 unless noted)

Resistors: 5k1 (R1,R2), 2k (R3), 1k (R4), 1M1 (R5), 150k (R6), 100k (R7),
10k (R8–R11,R13,R14), 4R7 (R12).
Capacitors: 100n (C10–C12,C16,C17), 1u (C3,C4,C8,C14,C15), 4u7 (C1,C2),
10u 0805 (C5,C9,C13), 22u 0805 (C6,C7).
Jumpers: 0R 0805 (JP1,JP2).
14 value lines × 10 pcs ≈ $14 at DigiKey catalog cut-tape pricing; the board
consumes 33 of the 140 pieces (≈ $3.30 per device).

## Order 2 — LCSC

The USB-C connector is an LCSC-only part; the ZH connector and switch are
cheapest there, so they ride in the same cart.

| Ref(s) | Part | MPN (LCSC #) | Order qty | Unit price | Line | Stock seen |
|---|---|---|---|---|---|---|
| J1 | USB-C 16P receptacle | TYPE-C-31-M-12 (C165948) | 5 (MOQ) | $0.186 @5 | $0.93 | ~156,000 |
| J4 | JST ZH 6-pin SMT | B6B-ZR-SM4-TFT(LF)(SN) (C265005) | 2 | $0.74 | $1.48 | ~1,570 |
| SW1 | SKQG tactile, with stem | SKQGAFE010 (C202424) | 5 | $0.137 @5 | $0.69 | ~1,000–5,900 |

**LCSC subtotal ≈ $3.10** + shipping (~$7 economy). DigiKey's own listings for
these: B6B-ZR-SM4-TFT no stock / 16-wk, SKQGAFE010 not listed — LCSC is the
in-stock source, not just the cheap one.

## Order 3 — off-distributor items

| Item | Source | Price | Note |
|---|---|---|---|
| 2000 mAh protected LiPo, JST-PH | Adafruit #2011 — **out of stock** ($12.50, notify-me set) → in-stock fallback: Jameco #2507799 | $12.50 / **$16.55** | Protection circuit built in (3.0 V cutout, short protection), genuine JST-PH — both verified on the Adafruit page |
| SSD1306 0.96″ 128×64 I2C OLED module | Amazon/AliExpress multi-pack (e.g. AITRIP 10-pack $22.99) or single ~$3 | ~$2.30–3.00 | Any 4-pin I2C variant (GND-VCC-SCL-SDA); pin order varies by seller — check against J5 before plugging |
| SEN50 cable (ZHR-6, 1.5 mm) | SparkFun "Breadboard to JST-ZHR 6-pin" ×2 | $2.25 ea, $4.50 | Board J4 and the SEN50 both take ZHR-6 housings; two breadboard cables spliced back-to-back make the harness, and the Dupont ends serve bring-up probing first |
| M2 standoff/screw kit (nylon) | Amazon assortment | ~$9 (estimate) | For H1–H4; optional until an enclosure exists |
| Solder stencil (frameless) | PCBWay, added to board order | ~$10–15 | SGP41 needs 125–150 µm foil — note it in the order comments |

## Order 4 — PCBWay boards

Upload `fab/airsense_v4_pcbway_gerbers.zip` (12 files: 2× copper, 2× mask,
2× silk, 2× paste, edge cuts, PTH + NPTH drill, job file) to the PCBWay quote
tool. Settings: 2 layers, 100 × 40 mm, FR-4 1.6 mm, HASL or ENIG, 5 pcs
(smallest tier). Indicative 2026 pricing: boards ~$5–10 / 5 pcs, stencil
~$10–15, shipping ~$15–20 → **~$30–45 landed** with stencil. The design rules
were written against PCBWay's standard 2-layer limits (see `design-rules.md`),
so no special options are needed.

## Per-device parts cost (AC-15, BOM half)

One assembled device, at the qty-1 prices above:

| Block | Cost |
|---|---|
| Sensors (SCD41 + SGP41 + BMP384 + SEN50) | $56.65 |
| MCU module | $3.38 |
| Power ICs + diodes + inductor (U1–U3, D1–D2, L1) | $2.82 |
| Connectors + switch + LED (J1–J5, SW1, LED1, socket) | $3.23 |
| Passives consumed (33 pcs) | $3.30 |
| OLED module | $3.00 |
| Battery (in-stock price) | $16.55 |
| Board share (5-board order ÷ 5, incl. stencil + ship) | ~$7.00 |
| **Per-device total** | **≈ $96** |

Consistent with the spec's ~$100 estimate. The recurring (board-revision) cost
is the board share plus passives — the sensors, modules, battery, and OLED all
carry over on connectors or get reflowed once.

## Price sources (checked 2026-10-13)

- [SCD41-D-R2 — DigiKey](https://www.digikey.com/en/products/detail/sensirion-ag/SCD41-D-R2/13684005)
- [SGP41-D-R4 — DigiKey](https://www.digikey.com/en/products/detail/sensirion-ag/SGP41-D-R4/15652788)
- [BMP384 — DigiKey](https://www.digikey.com/en/products/detail/bosch-sensortec/BMP384/13681277)
- [SEN50-SDN-T — DigiKey](https://www.digikey.com/en/products/detail/sensirion-ag/SEN50-SDN-T/16342755)
- [ESP32-C3-WROOM-02-N4 — DigiKey](https://www.digikey.com/en/products/detail/espressif-systems/ESP32-C3-WROOM-02-N4/14553031)
- [TPS61023DRLR — DigiKey](https://www.digikey.com/en/products/detail/texas-instruments/TPS61023DRLR/11310629)
- [MCP73831T-2ACI/OT — DigiKey](https://www.digikey.com/en/products/detail/microchip-technology/MCP73831T-2ACI-OT/964301)
- [AP2112K-3.3TRG1 — DigiKey](https://www.digikey.com/en/products/detail/diodes-incorporated/AP2112K-3-3TRG1/4470746)
- [CDBW140-G — DigiKey](https://www.digikey.com/en/products/detail/comchip-technology/CDBW140-G/3308555)
- [1N5819HW-7-F (backordered original) — DigiKey](https://www.digikey.com/en/products/detail/diodes-incorporated/1N5819HW-7-F/814970)
- [NRS4018T1R0NDGJ — distributor aggregate](https://electronicsdatasheets.com/manufacturers/taiyo-yuden/parts/nrs4018t1r0ndgj)
- [LTST-C190KGKT — DigiKey](https://www.digikey.co.nz/en/products/detail/liteon/LTST-C190KGKT/386815)
- [TYPE-C-31-M-12 — LCSC C165948](https://lcsc.com/product-detail/C165948.html)
- [B6B-ZR-SM4-TFT — LCSC C265005](https://www.lcsc.com/product-image/C265005.html)
- [SKQGAFE010 — LCSC C202424](https://www.lcsc.com/product-image/C202424.html)
- [S2B-PH-SM4-TB — DigiKey via aggregate](https://www.flux.ai/lcsc/s2b-ph-sm4-tblfsn.md)
- [LiPo 2000 mAh — Adafruit #2011](https://www.adafruit.com/product/2011) / [Jameco #2507799](https://www.jameco.com/z/2011-Adafruit-Industries-Lithium-Ion-Battery-3-7V-2000mAh_2507799.html)
- [SEN50 ZHR-6 cable — SparkFun](https://www.sparkfun.com/breadboard-to-jst-zhr-cable-6-pin-x-1-5mm-pitch.html)
- [PCBWay pricing reference](https://www.pcbway.com/project/question/pricing/?type=PCB)
