#!/usr/bin/env python3
"""AirSense v4 BLE protocol verification client (ticket 07).

Asserts the complete BLE contract from app/constants/ble-protocol.md against
a live AirSense device and prints a PASS/FAIL report per assertion. This is
the test harness ticket 09 runs against the assembled unit.

Usage:
  python ble_verify.py                 # full contract run (~2-3 min)
  python ble_verify.py --observe 15    # + AC-10 leg: write custom 15 s,
                                       #   disconnect, wait, reconnect, check
                                       #   history timestamp spacing (also
                                       #   accepts 60 / 300 / any 10..86400)
  python ble_verify.py --self-test     # decoder/assertion unit checks only
                                       #   (no device, no bleak import)

Needs: Python 3.10+, bleak >= 3.0 (pip install bleak), a BLE adapter.
The device forces continuous sampling (~5.5 s cycles) while connected, so
duty cadences are verified through the history characteristic after a
disconnect gap (--observe), never through live notification spacing.
"""
from __future__ import annotations

import argparse
import asyncio
import struct
import sys
import time

# ---------------------------------------------------------------------------
# Protocol constants — mirror app/constants/ble-protocol.md (the authoritative
# contract document). Change both together or neither.
# ---------------------------------------------------------------------------

DEVICE_NAME = "AirSense"

ENV_SERVICE = "0000181a-0000-1000-8000-00805f9b34fb"
CUSTOM_SERVICE = "12345678-1234-5678-1234-56789abcdef0"

# Frozen Phase-2 characteristics (app/constants/ble.ts ships these)
CO2 = "00002bd2-0000-1000-8000-00805f9b34fb"
TEMPERATURE = "00002a6e-0000-1000-8000-00805f9b34fb"
HUMIDITY = "00002a6f-0000-1000-8000-00805f9b34fb"
VOC = "12345678-1234-5678-1234-56789abcdef1"
NOX = "12345678-1234-5678-1234-56789abcdef2"

# v4 additions
PRESSURE = "00002a6d-0000-1000-8000-00805f9b34fb"
PM1_STD = "00002bd5-0000-1000-8000-00805f9b34fb"
PM25_STD = "00002bd6-0000-1000-8000-00805f9b34fb"
PM10_STD = "00002bd7-0000-1000-8000-00805f9b34fb"
PM1 = "12345678-1234-5678-1234-56789abcdef3"
PM25 = "12345678-1234-5678-1234-56789abcdef4"
PM10 = "12345678-1234-5678-1234-56789abcdef5"
CADENCE = "12345678-1234-5678-1234-56789abcdef6"
HISTORY = "12345678-1234-5678-1234-56789abcdef7"

CCCD = "00002902-0000-1000-8000-00805f9b34fb"

INVALID_U16 = 0xFFFF
INVALID_TEMP = 0x7FFF
INVALID_U32 = 0xFFFFFFFF

HISTORY_CAPACITY = 720
HISTORY_RECORD_FMT = "<IHhHHHHHHI"  # 24 bytes
HISTORY_STATUS_FMT = "<HHIII"  # 16 bytes
END_SEQ = 0xFFFF

CONTINUOUS_MS = 5500  # forced cycle while connected
SPACING_TOL_MS = 1500  # same tolerance the firmware sim checker uses

CADENCE_MODES = {0: "continuous", 1: "60s", 2: "300s", 3: "custom"}

# (uuid, label, service, writable) for the GATT-table assertions
ALL_CHARS = [
    (CO2, "CO2 (frozen)", ENV_SERVICE, False),
    (TEMPERATURE, "Temperature (frozen)", ENV_SERVICE, False),
    (HUMIDITY, "Humidity (frozen)", ENV_SERVICE, False),
    (PRESSURE, "Pressure (v4)", ENV_SERVICE, False),
    (PM1_STD, "PM1.0 std (v4)", ENV_SERVICE, False),
    (PM25_STD, "PM2.5 std (v4)", ENV_SERVICE, False),
    (PM10_STD, "PM10 std (v4)", ENV_SERVICE, False),
    (VOC, "VOC (frozen)", CUSTOM_SERVICE, False),
    (NOX, "NOx (frozen)", CUSTOM_SERVICE, False),
    (PM1, "PM1.0 custom (v4)", CUSTOM_SERVICE, False),
    (PM25, "PM2.5 custom (v4)", CUSTOM_SERVICE, False),
    (PM10, "PM10 custom (v4)", CUSTOM_SERVICE, False),
    (CADENCE, "Cadence (v4)", CUSTOM_SERVICE, True),
    (HISTORY, "History sync (v4)", CUSTOM_SERVICE, True),
]

# ---------------------------------------------------------------------------
# Decoders (all wire values little-endian)
# ---------------------------------------------------------------------------


def u16(data: bytes) -> int:
    return struct.unpack("<H", bytes(data[:2]))[0]


def i16(data: bytes) -> int:
    return struct.unpack("<h", bytes(data[:2]))[0]


def u32(data: bytes) -> int:
    return struct.unpack("<I", bytes(data[:4]))[0]


def decode_co2(data: bytes) -> int:
    return u16(data)  # ppm; 0 before the first valid sample


def decode_temp(data: bytes) -> float | None:
    raw = i16(data)
    return None if raw == INVALID_TEMP else raw / 100.0  # degC


def decode_humidity(data: bytes) -> float:
    return u16(data) / 100.0  # %RH


def decode_index(data: bytes) -> int:
    return u16(data)  # VOC/NOx gas index; 0 = warming up / invalid


def decode_pm_x10(data: bytes) -> float | None:
    raw = u16(data)
    return None if raw == INVALID_U16 else raw / 10.0  # ug/m3


def decode_pressure(data: bytes) -> float | None:
    raw = u32(data)
    return None if raw == INVALID_U32 else raw / 10.0  # Pa


def decode_sfloat(raw: int) -> float | None:
    """IEEE 11073 SFLOAT (medfloat16). None for NaN/NRes/±INF/reserved."""
    if raw in (0x07FF, 0x0800, 0x07FE, 0x0801, 0x0802):
        return None
    exp = raw >> 12
    if exp >= 8:
        exp -= 16
    mant = raw & 0x0FFF
    if mant >= 0x0800:
        mant -= 0x1000
    return mant * (10.0 ** exp)


def decode_pm_std(data: bytes) -> float | None:
    """Standard PM characteristic: SFLOAT in kg/m3 -> ug/m3 (or None)."""
    v = decode_sfloat(u16(data))
    return None if v is None else v * 1e9


def pm_std_custom_agree(std: float, cust: float) -> bool:
    """The standard SFLOAT quantizes to 10 ug/m3 steps, so it must sit
    within half a step of the custom value. The firmware rounds the raw
    float into each characteristic independently (and lroundf rounds
    halves away from zero while Python round() rounds to even), so exact
    re-rounding of the custom value would false-FAIL near 5 ug/m3
    half-boundaries — allow half a std step plus half a custom LSB."""
    return abs(std - cust) <= 5.05


def encode_cadence(mode: int, custom_seconds: int = 60) -> bytes:
    return struct.pack("<BI", mode, custom_seconds)


def parse_cadence(data: bytes) -> tuple[int, int]:
    mode, custom = struct.unpack("<BI", bytes(data[:5]))
    return mode, custom


def parse_history_status(data: bytes) -> dict:
    count, capacity, oldest, newest, now = struct.unpack(
        HISTORY_STATUS_FMT, bytes(data[:16])
    )
    return {
        "count": count,
        "capacity": capacity,
        "oldest_t": oldest,
        "newest_t": newest,
        "now_t": now,
    }


def parse_history_record(data: bytes) -> dict:
    (t, co2, temp, rh, voc, nox, pm1, pm25, pm10, press) = struct.unpack(
        HISTORY_RECORD_FMT, bytes(data[:24])
    )
    return {
        "t_seconds": t,
        "co2": None if co2 == INVALID_U16 else co2,
        "temp": None if temp == INVALID_TEMP else temp / 100.0,
        "rh": None if rh == INVALID_U16 else rh / 100.0,
        "voc": voc,
        "nox": nox,
        "pm1": None if pm1 == INVALID_U16 else pm1 / 10.0,
        "pm25": None if pm25 == INVALID_U16 else pm25 / 10.0,
        "pm10": None if pm10 == INVALID_U16 else pm10 / 10.0,
        "press": None if press == INVALID_U32 else press / 10.0,
    }


def parse_history_frame(data: bytes):
    """Returns ('end', sent, status) or ('record', seq, record_dict)."""
    seq = u16(data)
    if seq == END_SEQ:
        sent, status = struct.unpack("<HH", bytes(data[2:6]))
        return ("end", sent, status)
    return ("record", seq, parse_history_record(bytes(data[2:26])))


def spacing_deltas(timestamps: list[float]) -> list[float]:
    return [b - a for a, b in zip(timestamps, timestamps[1:])]


def spacing_ok(deltas_s: list[float], expect_s: float, tol_s: float) -> bool:
    return bool(deltas_s) and all(abs(d - expect_s) <= tol_s for d in deltas_s)


# ---------------------------------------------------------------------------
# Report
# ---------------------------------------------------------------------------


class Report:
    def __init__(self) -> None:
        self.checks: list[tuple[str, bool, str]] = []

    def check(self, name: str, ok: bool, detail: str = "") -> bool:
        self.checks.append((name, ok, detail))
        line = f"{'PASS' if ok else 'FAIL'}  {name}"
        if detail:
            line += f" — {detail}"
        print(line, flush=True)
        return ok

    def summary(self) -> bool:
        passed = sum(1 for _, ok, _ in self.checks if ok)
        total = len(self.checks)
        ok = passed == total and total > 0
        print()
        print(f"RESULT: {'PASS' if ok else 'FAIL'} ({passed}/{total} assertions)")
        for name, k, detail in self.checks:
            if not k:
                print(f"  failed: {name}" + (f" — {detail}" if detail else ""))
        return ok


# ---------------------------------------------------------------------------
# Live-device assertions
# ---------------------------------------------------------------------------


def plausible(rep: Report, tag: str, values: dict) -> None:
    """Range checks for one full set of quantities (AC-14 list)."""
    co2 = values["co2"]
    rep.check(f"{tag} CO2 plausible",
              co2 is None or co2 == 0 or 350 <= co2 <= 40000,
              "invalid" if co2 is None else f"{co2} ppm")
    t = values["temp"]
    rep.check(
        f"{tag} temperature plausible",
        t is None or t == 0.0 or -40.0 <= t <= 85.0,
        "invalid" if t is None else f"{t} C",
    )
    rh = values["rh"]
    rep.check(f"{tag} humidity plausible", rh is None or 0.0 <= rh <= 100.0,
              "invalid" if rh is None else f"{rh} %")
    rep.check(f"{tag} VOC index plausible", 0 <= values["voc"] <= 500,
              str(values["voc"]))
    rep.check(f"{tag} NOx index plausible", 0 <= values["nox"] <= 500,
              str(values["nox"]))
    for key, label in (("pm1", "PM1.0"), ("pm25", "PM2.5"), ("pm10", "PM10")):
        v = values[key]
        rep.check(f"{tag} {label} plausible", v is None or 0.0 <= v <= 1000.0,
                  "invalid" if v is None else f"{v} ug/m3")
    p = values["press"]
    rep.check(f"{tag} pressure plausible", p is None or 30000.0 <= p <= 110000.0,
              "invalid" if p is None else f"{p} Pa")


async def collect_history_dump(client, timeout_s: float = 30.0):
    """Start a dump and collect frames until the end frame (or timeout)."""
    frames: list[tuple] = []
    done = asyncio.Event()

    def on_notify(_char, data: bytearray) -> None:
        frame = parse_history_frame(bytes(data))
        frames.append(frame)
        if frame[0] == "end":
            done.set()

    await client.start_notify(HISTORY, on_notify)
    try:
        await client.write_gatt_char(HISTORY, b"\x01", response=True)
        try:
            await asyncio.wait_for(done.wait(), timeout=timeout_s)
        except asyncio.TimeoutError:
            pass
    finally:
        await client.stop_notify(HISTORY)
    return frames


def check_dump(rep: Report, tag: str, frames: list, expected_count: int) -> list:
    """Shared dump assertions; returns the records in seq order."""
    ends = [f for f in frames if f[0] == "end"]
    rep.check(f"{tag} dump ended with an end frame", len(ends) == 1,
              f"{len(ends)} end frames / {len(frames)} frames")
    records = [(f[1], f[2]) for f in frames if f[0] == "record"]
    if ends:
        _, sent, status = ends[0]
        rep.check(f"{tag} end-frame status ok", status == 0, f"status={status}")
        rep.check(
            f"{tag} sent count consistent",
            sent == len(records) and abs(sent - expected_count) <= 2,
            f"sent={sent} received={len(records)} status-count={expected_count}",
        )
    if not frames:
        # Timed out with nothing at all — the framing checks below would
        # pass vacuously, so fail once here instead.
        rep.check(f"{tag} frames received", False, "no frames before timeout")
        return []
    seqs = [s for s, _ in records]
    rep.check(f"{tag} seq contiguous from 0", seqs == list(range(len(seqs))),
              f"{len(seqs)} records")
    ts = [r["t_seconds"] for _, r in records]
    rep.check(f"{tag} timestamps non-decreasing",
              all(b >= a for a, b in zip(ts, ts[1:])),
              f"span {ts[0]}..{ts[-1]} s" if ts else "0 records (empty history)")
    return [r for _, r in records]


async def run_live(args, rep: Report) -> None:
    from bleak import BleakClient, BleakScanner

    # --- A: discovery ------------------------------------------------------
    device = await BleakScanner.find_device_by_name(
        args.name, timeout=args.scan_timeout
    )
    if not rep.check("A1 advertising: device found by name",
                     device is not None, f"name={args.name!r}"):
        return

    async with BleakClient(device, timeout=20.0) as client:
        rep.check("A2 connected", client.is_connected)
        mtu = client.mtu_size
        rep.check("A3 ATT MTU >= 29 (history-dump capable)", mtu >= 29,
                  f"mtu={mtu}")

        # --- B: GATT table (AC-8 layout + AC-9 additions) ------------------
        svcs = client.services
        rep.check("B1 environmental service present",
                  svcs.get_service(ENV_SERVICE) is not None, ENV_SERVICE)
        rep.check("B2 custom service present",
                  svcs.get_service(CUSTOM_SERVICE) is not None, CUSTOM_SERVICE)
        for uuid, label, service, writable in ALL_CHARS:
            char = svcs.get_characteristic(uuid)
            if not rep.check(f"B3 {label} present", char is not None, uuid):
                continue
            props = set(char.properties)
            need = {"read", "notify"} | ({"write"} if writable else set())
            rep.check(f"B4 {label} properties", need.issubset(props),
                      f"has {sorted(props)}")
            has_cccd = any(d.uuid.lower() == CCCD for d in char.descriptors)
            rep.check(f"B5 {label} CCCD descriptor", has_cccd)

        # --- C: every quantity readable in one session (AC-14) -------------
        values = {
            "co2": decode_co2(await client.read_gatt_char(CO2)),
            "temp": decode_temp(await client.read_gatt_char(TEMPERATURE)),
            "rh": decode_humidity(await client.read_gatt_char(HUMIDITY)),
            "voc": decode_index(await client.read_gatt_char(VOC)),
            "nox": decode_index(await client.read_gatt_char(NOX)),
            "pm1": decode_pm_x10(await client.read_gatt_char(PM1)),
            "pm25": decode_pm_x10(await client.read_gatt_char(PM25)),
            "pm10": decode_pm_x10(await client.read_gatt_char(PM10)),
            "press": decode_pressure(await client.read_gatt_char(PRESSURE)),
        }
        plausible(rep, "C1", values)
        # Standard vs custom PM cross-check: the standard SFLOAT quantizes to
        # 10 ug/m3 steps, so it must sit within half a step of the custom
        # value (see pm_std_custom_agree for the rounding tolerance).
        for std_uuid, cust_key, label in ((PM1_STD, "pm1", "PM1.0"),
                                          (PM25_STD, "pm25", "PM2.5"),
                                          (PM10_STD, "pm10", "PM10")):
            std = decode_pm_std(await client.read_gatt_char(std_uuid))
            cust = values[cust_key]
            if cust is None:
                ok = std is None
                detail = "both invalid" if ok else f"std={std} while custom invalid"
            else:
                ok = std is not None and pm_std_custom_agree(std, cust)
                detail = f"std={std} custom={cust} ug/m3"
            rep.check(f"C2 {label} standard/custom encoding agreement", ok, detail)

        # --- D: connection forces continuous (notification spacing) --------
        stamps: list[float] = []
        pm_notified = asyncio.Event()

        def on_co2(_char, _data: bytearray) -> None:
            stamps.append(time.monotonic())

        def on_pm(_char, _data: bytearray) -> None:
            pm_notified.set()

        await client.start_notify(CO2, on_co2)
        await client.start_notify(PM25, on_pm)
        want = 4  # 3 gaps
        deadline = time.monotonic() + want * (CONTINUOUS_MS / 1000.0) + 15.0
        while len(stamps) < want and time.monotonic() < deadline:
            await asyncio.sleep(0.2)
        await client.stop_notify(CO2)
        await client.stop_notify(PM25)
        deltas = spacing_deltas(stamps[:want])
        rep.check(
            "D1 connected spacing is forced-continuous (~5.5 s)",
            len(stamps) >= want
            and spacing_ok(deltas, CONTINUOUS_MS / 1000.0, SPACING_TOL_MS / 1000.0),
            f"deltas={[f'{d:.2f}s' for d in deltas]}",
        )
        rep.check("D2 v4 characteristic notifies (custom PM2.5)",
                  pm_notified.is_set())

        # --- E: cadence characteristic -------------------------------------
        orig = parse_cadence(await client.read_gatt_char(CADENCE))
        # custom_seconds is meaningful only for mode 3, so its range is
        # asserted only there.
        rep.check("E1 cadence readback sane",
                  orig[0] in CADENCE_MODES
                  and (orig[0] != 3 or 10 <= orig[1] <= 86400),
                  f"mode={CADENCE_MODES.get(orig[0], orig[0])} custom={orig[1]}s")

        cad_notes: list[bytes] = []

        def on_cad(_char, data: bytearray) -> None:
            cad_notes.append(bytes(data))

        await client.start_notify(CADENCE, on_cad)
        try:
            await client.write_gatt_char(CADENCE, bytes([2]), response=True)
            await asyncio.sleep(1.0)
            rb = parse_cadence(await client.read_gatt_char(CADENCE))
            rep.check("E2 1-byte write: mode 300 s persisted", rb[0] == 2,
                      f"readback mode={rb[0]}")

            await client.write_gatt_char(CADENCE, encode_cadence(3, 900),
                                         response=True)
            await asyncio.sleep(1.0)
            rb = parse_cadence(await client.read_gatt_char(CADENCE))
            rep.check("E3 5-byte write: custom 900 s persisted",
                      rb == (3, 900), f"readback={rb}")

            await client.write_gatt_char(CADENCE, encode_cadence(7, 60),
                                         response=True)
            await asyncio.sleep(1.0)
            rb2 = parse_cadence(await client.read_gatt_char(CADENCE))
            rep.check("E4 invalid mode rejected (state unchanged)", rb2 == rb,
                      f"readback={rb2}")

            await client.write_gatt_char(CADENCE, encode_cadence(3, 5),
                                         response=True)
            await asyncio.sleep(1.0)
            rb2 = parse_cadence(await client.read_gatt_char(CADENCE))
            rep.check("E5 out-of-range custom (5 s) rejected", rb2 == rb,
                      f"readback={rb2}")

            rep.check("E6 cadence change notified", len(cad_notes) >= 2,
                      f"{len(cad_notes)} notifications")
            await client.stop_notify(CADENCE)

            # --- F: history characteristic ---------------------------------
            status = parse_history_status(await client.read_gatt_char(HISTORY))
            rep.check("F1 history status well-formed",
                      status["capacity"] == HISTORY_CAPACITY
                      and status["count"] <= status["capacity"]
                      and status["oldest_t"] <= status["newest_t"]
                      <= status["now_t"],
                      str(status))
            frames = await collect_history_dump(client)
            records = check_dump(rep, "F2", frames, status["count"])
            if records:
                plausible(rep, "F3 last-record", records[-1])
        finally:
            # --- G: restore the persisted cadence, even when E or F raised -
            try:
                await client.write_gatt_char(
                    CADENCE, encode_cadence(orig[0], orig[1]), response=True
                )
                await asyncio.sleep(1.0)
                rb = parse_cadence(await client.read_gatt_char(CADENCE))
                rep.check("G1 original cadence restored", rb == orig,
                          f"readback={rb}")
            except Exception as exc:
                rep.check("G1 original cadence restored", False,
                          f"{type(exc).__name__}: {exc}")

    # --- Z: device re-advertises after disconnect ---------------------------
    await asyncio.sleep(2.0)  # firmware restarts advertising 500 ms after drop
    again = await BleakScanner.find_device_by_name(args.name, timeout=15.0)
    rep.check("Z1 re-advertising after disconnect", again is not None)

    # --- H (--observe): AC-10 leg — duty cadence honored while disconnected -
    if args.observe:
        n = args.observe
        mode, custom = (1, 60) if n == 60 else (2, 300) if n == 300 else (3, n)
        print(f"\n--observe {n}: writing cadence, then disconnecting for "
              f"{3.5 * n:.0f} s to let duty cycles accumulate...")
        restored = False
        try:
            async with BleakClient(device, timeout=20.0) as client:
                await client.write_gatt_char(
                    CADENCE, encode_cadence(mode, custom), response=True
                )
                await asyncio.sleep(1.0)
                h_rb = parse_cadence(await client.read_gatt_char(CADENCE))
                rep.check(f"H1 cadence {n} s accepted",
                          h_rb[0] == mode and (mode != 3 or h_rb[1] == custom),
                          f"readback={h_rb}")
            await asyncio.sleep(3.5 * n)
            device2 = await BleakScanner.find_device_by_name(
                args.name, timeout=20.0
            )
            if rep.check("H2 device found after observation gap",
                         device2 is not None):
                async with BleakClient(device2, timeout=20.0) as client:
                    status = parse_history_status(
                        await client.read_gatt_char(HISTORY)
                    )
                    frames = await collect_history_dump(client)
                    records = check_dump(rep, "H3", frames, status["count"])
                    ts = [r["t_seconds"] for r in records]
                    deltas = spacing_deltas([float(t) for t in ts])
                    matching = [d for d in deltas
                                if abs(d - n) <= max(2.0, n * 0.05)]
                    rep.check(
                        f"H4 history shows >=2 gaps at the {n} s cadence "
                        f"(AC-10)",
                        len(matching) >= 2,
                        f"matching deltas={matching[:6]}",
                    )
                    # restore the cadence captured at the start of the run
                    await client.write_gatt_char(
                        CADENCE, encode_cadence(orig[0], orig[1]),
                        response=True,
                    )
                    await asyncio.sleep(1.0)
                    rb = parse_cadence(await client.read_gatt_char(CADENCE))
                    restored = rep.check(
                        "H5 original cadence restored after observation",
                        rb == orig, f"readback={rb}")
        finally:
            if not restored:
                # Never leave the device at the observe cadence: best-effort
                # reconnect and restore when the normal path did not get there.
                try:
                    dev = await BleakScanner.find_device_by_name(
                        args.name, timeout=20.0
                    )
                    if dev is None:
                        rep.check("H5 original cadence restored after "
                                  "observation", False,
                                  "device not found for recovery restore")
                    else:
                        async with BleakClient(dev, timeout=20.0) as client:
                            await client.write_gatt_char(
                                CADENCE, encode_cadence(orig[0], orig[1]),
                                response=True,
                            )
                            await asyncio.sleep(1.0)
                            rb = parse_cadence(
                                await client.read_gatt_char(CADENCE)
                            )
                            rep.check("H5 original cadence restored after "
                                      "observation", rb == orig,
                                      f"readback={rb} (recovery path)")
                except Exception as exc:
                    rep.check("H5 original cadence restored after observation",
                              False,
                              f"recovery failed: {type(exc).__name__}: {exc}")


# ---------------------------------------------------------------------------
# Self-test: decoders + assertion engine against firmware-exact vectors
# ---------------------------------------------------------------------------


def run_self_test(rep: Report) -> None:
    # Values the ticket-05/06 firmware actually produces (Wokwi evidence):
    # co2=725, temp=24.30, rh=43.0, pm2_5=5.7, press=101325.
    rep.check("S1 co2 u16", decode_co2(struct.pack("<H", 725)) == 725)
    rep.check("S2 temp i16 x100", decode_temp(struct.pack("<h", 2430)) == 24.30)
    rep.check("S3 temp negative", decode_temp(struct.pack("<h", -550)) == -5.5)
    rep.check("S4 temp invalid sentinel",
              decode_temp(struct.pack("<h", INVALID_TEMP)) is None)
    rep.check("S5 humidity x100",
              decode_humidity(struct.pack("<H", 4300)) == 43.0)
    rep.check("S6 pm custom x10",
              decode_pm_x10(struct.pack("<H", 57)) == 5.7)
    rep.check("S7 pm custom invalid",
              decode_pm_x10(struct.pack("<H", INVALID_U16)) is None)
    rep.check("S8 pressure x10",
              decode_pressure(struct.pack("<I", 1013250)) == 101325.0)
    rep.check("S9 pressure invalid",
              decode_pressure(struct.pack("<I", INVALID_U32)) is None)

    # SFLOAT: firmware packPmSfloat(5.7 ug/m3) = (0x8 << 12) | round(0.57)
    # = 0x8001 = 1e-8 kg/m3 = 10 ug/m3 (the documented 10 ug/m3 floor).
    v = decode_pm_std(struct.pack("<H", 0x8001))
    rep.check("S10 sfloat 0x8001 -> 10 ug/m3",
              v is not None and abs(v - 10.0) < 1e-6, f"{v}")
    v = decode_pm_std(struct.pack("<H", 0x8064))  # m=100 -> 1000 ug/m3
    rep.check("S11 sfloat 0x8064 -> 1000 ug/m3",
              v is not None and abs(v - 1000.0) < 1e-6, f"{v}")
    rep.check("S12 sfloat NaN",
              decode_pm_std(struct.pack("<H", 0x07FF)) is None)
    rep.check("S13 sfloat zero",
              decode_pm_std(struct.pack("<H", 0x8000)) == 0.0)
    # C2's agreement rule against firmware pairs (std quantizes to 10s):
    # 5.7 ug/m3 -> custom 5.7, std 10 (lroundf(0.57)=1). 5.0 is the lroundf
    # half-away-from-zero boundary: custom 5.0, std 10 — must still agree.
    # 14.97 double-rounds: custom 15.0 (lroundf 149.7->150), std 10
    # (lroundf 1.497->1) — must still agree. A full step off must not.
    rep.check("S14 std/custom agreement (5.7 pair)",
              pm_std_custom_agree(10.0, 5.7))
    rep.check("S25 std/custom agreement at lroundf half-boundary (5.0 pair)",
              pm_std_custom_agree(10.0, 5.0))
    rep.check("S26 std/custom agreement under double rounding (14.97 pair)",
              pm_std_custom_agree(10.0, 15.0))
    rep.check("S27 std/custom agreement rejects a full-step mismatch",
              not pm_std_custom_agree(20.0, 5.7))
    rep.check("S28 sfloat reserved 0x0801 -> None",
              decode_pm_std(struct.pack("<H", 0x0801)) is None)

    rep.check("S15 cadence encode",
              encode_cadence(3, 900) == bytes([3, 0x84, 0x03, 0x00, 0x00]))
    rep.check("S16 cadence parse round-trip",
              parse_cadence(encode_cadence(2, 60)) == (2, 60))

    status = struct.pack(HISTORY_STATUS_FMT, 5, 720, 100, 340, 345)
    rep.check("S17 history status parse",
              parse_history_status(status) == {
                  "count": 5, "capacity": 720, "oldest_t": 100,
                  "newest_t": 340, "now_t": 345})

    rec = struct.pack(HISTORY_RECORD_FMT, 345, 725, 2430, 4300, 0, 0,
                      35, 57, 71, 1013250)
    parsed = parse_history_record(rec)
    rep.check("S18 history record parse",
              parsed["t_seconds"] == 345 and parsed["co2"] == 725
              and parsed["temp"] == 24.30 and parsed["rh"] == 43.0
              and parsed["pm25"] == 5.7 and parsed["press"] == 101325.0,
              str(parsed))
    rec_invalid = struct.pack(HISTORY_RECORD_FMT, 10, INVALID_U16,
                              INVALID_TEMP, INVALID_U16, 0, 0, INVALID_U16,
                              INVALID_U16, INVALID_U16, INVALID_U32)
    parsed = parse_history_record(rec_invalid)
    rep.check("S19 history record sentinels -> None",
              all(parsed[k] is None
                  for k in ("co2", "temp", "rh", "pm1", "pm25", "pm10",
                            "press")))

    frame = struct.pack("<H", 3) + rec
    kind, seq, r = parse_history_frame(frame)
    rep.check("S20 record frame parse", kind == "record" and seq == 3
              and r["co2"] == 725)
    end = struct.pack("<HHH", END_SEQ, 5, 0)
    rep.check("S21 end frame parse",
              parse_history_frame(end) == ("end", 5, 0))

    rep.check("S22 spacing accepts firmware deltas",
              spacing_ok([5.504, 5.509, 5.504], 5.5, 1.5))
    rep.check("S23 spacing rejects wrong cadence",
              not spacing_ok([10.0, 5.5], 5.5, 1.5))
    rep.check("S24 spacing rejects empty", not spacing_ok([], 5.5, 1.5))


# ---------------------------------------------------------------------------


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("--name", default=DEVICE_NAME,
                    help="advertised device name (default: AirSense)")
    ap.add_argument("--scan-timeout", type=float, default=15.0)
    ap.add_argument("--observe", type=int, default=None, metavar="SECONDS",
                    help="also verify a duty cadence via a disconnect gap + "
                         "history dump (AC-10); e.g. 15, 60, 300")
    ap.add_argument("--self-test", action="store_true",
                    help="run decoder/assertion unit checks only (no device)")
    args = ap.parse_args()

    if args.observe is not None and not 10 <= args.observe <= 86400:
        ap.error("--observe must be 10..86400 seconds")

    rep = Report()
    if args.self_test:
        run_self_test(rep)
    else:
        try:
            asyncio.run(run_live(args, rep))
        except Exception as exc:  # adapter missing, connection drop, ...
            rep.check("X0 run completed without client-side error", False,
                      f"{type(exc).__name__}: {exc}")
    return 0 if rep.summary() else 1


if __name__ == "__main__":
    sys.exit(main())
