# AirSense BLE Protocol

**Version:** v4 (ticket 06, 2026-10-14). Extends the frozen Phase-2 contract.
Device firmware: `firmware/src/v4/ble_protocol.h` mirrors this document —
change both together or neither.

All multi-byte values are **little-endian**. Every characteristic listed is
READ + NOTIFY with a Client Characteristic Configuration descriptor (0x2902),
unless a WRITE property is noted. Device name: `AirSense`. Advertising
carries both service UUIDs (scan response on).

## Environmental Sensing service — `0000181a-0000-1000-8000-00805f9b34fb`

| Characteristic | UUID | Format | Meaning |
|---|---|---|---|
| CO2 *(frozen)* | `00002bd2-…` | uint16 | ppm |
| Temperature *(frozen)* | `00002a6e-…` | int16 | °C × 100 |
| Humidity *(frozen)* | `00002a6f-…` | uint16 | %RH × 100 |
| Pressure *(v4)* | `00002a6d-…` | uint32 | Pa × 10 (standard 0.1 Pa unit); `0xFFFFFFFF` = invalid |
| PM1.0 *(v4)* | `00002bd5-…` | medfloat16 | kg/m³ (standard; see note) |
| PM2.5 *(v4)* | `00002bd6-…` | medfloat16 | kg/m³ (standard; see note) |
| PM10 *(v4)* | `00002bd7-…` | medfloat16 | kg/m³ (standard; see note) |

All short UUIDs expand with the Bluetooth base `…-0000-1000-8000-00805f9b34fb`.

**PM resolution note.** The standard PM characteristics follow the GATT
Specification Supplement exactly: medfloat16 (IEEE 11073 SFLOAT:
mantissa × 10^exponent, 4-bit exponent limited to −8…+7) in kg/m³. The
smallest nonzero encodable value is 10⁻⁸ kg/m³ = **10 µg/m³**, so these
characteristics cannot carry indoor-air resolution. They exist so generic
Environmental Sensing clients (e.g. nRF Connect) decode *correct, coarse*
values. **The AirSense app must read the custom PM characteristics below
instead.** SFLOAT NaN (`0x07FF`) = invalid.

## Custom service — `12345678-1234-5678-1234-56789abcdef0`

| Characteristic | UUID suffix | Format | Meaning |
|---|---|---|---|
| VOC index *(frozen)* | `…def1` | uint16 | Sensirion VOC index (0 = warming up / invalid) |
| NOx index *(frozen)* | `…def2` | uint16 | Sensirion NOx index (0 = warming up / invalid) |
| PM1.0 *(v4)* | `…def3` | uint16 | µg/m³ × 10; `0xFFFF` = invalid |
| PM2.5 *(v4)* | `…def4` | uint16 | µg/m³ × 10; `0xFFFF` = invalid |
| PM10 *(v4)* | `…def5` | uint16 | µg/m³ × 10; `0xFFFF` = invalid |
| Cadence *(v4)* | `…def6` | 5 bytes, +WRITE | see below |
| History sync *(v4)* | `…def7` | variable, +WRITE | see below |

Frozen characteristics keep Phase-2 semantics: they always hold a number
(0 before the first valid sample). The v4 additions use the sentinels above.

### Cadence — `…def6`

Layout (5 bytes): `uint8 mode` + `uint32 custom_seconds`.
Modes: `0` continuous, `1` 60 s (default), `2` 300 s, `3` custom.
`custom_seconds` is meaningful only for mode 3; valid range 10–86400.

- **READ / NOTIFY:** the *persisted* setting (not the live override).
- **WRITE:** 5 bytes, or 1 byte (mode only) for modes 0–2. A valid write is
  persisted to flash and notified back. Invalid writes are ignored.
- **While the app is connected the device samples continuously** regardless
  of this setting (~5.5 s cycles). The persisted cadence resumes on
  disconnect. A cadence write while connected therefore changes what happens
  *after* disconnect.

### History sync — `…def7`

The device keeps a ring buffer of the most recent **720** samples (one per
completed measurement cycle; 12 h at the 60 s default). Timestamps are
**uptime seconds** (uint32); map them to wall-clock time using `now_t` from
the status read. The buffer does not survive a reboot.

- **READ** → 16-byte status: `uint16 count`, `uint16 capacity`,
  `uint32 oldest_t`, `uint32 newest_t`, `uint32 now_t`.
- **WRITE `0x01`** → start a dump; **WRITE `0x00`** → abort.
- **Dump** (notifications on this characteristic, oldest first):
  - Record frame (26 B): `uint16 seq` (0…count−1) + 24-byte record.
  - End frame (6 B): `uint16 0xFFFF` + `uint16 sent` + `uint16 status`
    (0 = ok, 1 = MTU too small).
- While a dump streams, new samples are **not** added to the ring (the frame
  order stays stable); the app receives those cycles live via the value
  notifications. Frame `seq` numbers are contiguous — a gap means a dropped
  notification; abort and re-request.
- **The client must negotiate ATT MTU ≥ 29 before starting a dump** (26-byte
  notification + 3-byte ATT overhead). Request 185 — both platforms support
  it (`react-native-ble-plx`: `requestMTU(185)` on Android; iOS negotiates
  automatically). A dump started with a too-small MTU is refused with an end
  frame, status 1.

Record layout (24 bytes, little-endian, same sentinels as the live
characteristics):

| Offset | Field | Format |
|---|---|---|
| 0 | `t_seconds` | uint32, uptime s |
| 4 | `co2_ppm` | uint16, `0xFFFF` invalid |
| 6 | `temp_x100` | int16 °C×100, `0x7FFF` invalid |
| 8 | `rh_x100` | uint16 %×100, `0xFFFF` invalid |
| 10 | `voc_index` | uint16, 0 invalid |
| 12 | `nox_index` | uint16, 0 invalid |
| 14 | `pm1_x10` | uint16 µg/m³×10, `0xFFFF` invalid |
| 16 | `pm25_x10` | uint16 µg/m³×10, `0xFFFF` invalid |
| 18 | `pm10_x10` | uint16 µg/m³×10, `0xFFFF` invalid |
| 20 | `press_pa_x10` | uint32 Pa×10, `0xFFFFFFFF` invalid |

## Connection behavior

- On connect the device switches to continuous sampling and notifies every
  characteristic each cycle (~5.5 s).
- On disconnect the device restores the persisted cadence and resumes
  advertising after 500 ms (frozen Phase-2 pattern).
