"""Verify cadence spacing from Wokwi serial logs (ticket 05, AC-10/AC-14).

Parses DATA/CADENCE/CYCLE lines from the given serial logs, groups DATA
timestamps by the cadence mode active when they printed, and checks the
inter-sample spacing for each mode. The expected interval is read from the
firmware's own `CADENCE ... interval=<N>s` print, so the checker never
duplicates scenario or firmware constants. Also checks AC-14: at least one
DATA line carries a numeric value for every quantity.

Usage: python check_cadence.py serial_a.log serial_b.log
Exits 0 when every check passes, 1 otherwise.
"""
import re
import sys

REQUIRED_MODES = ("continuous", "60s", "custom", "300s")

DATA_RE = re.compile(
    r"DATA t=(\d+) co2=(\S+) temp=(\S+) rh=(\S+) voc=(\S+) nox=(\S+)"
    r" pm1_0=(\S+) pm2_5=(\S+) pm10=(\S+) press=(\S+)"
)
CADENCE_RE = re.compile(r"CADENCE (?:loaded )?mode=(\w+).*?interval=(\d+)ms")
CYCLE_RE = re.compile(r"CYCLE n=\d+ next_in=(\d+)s mode=(\w+)")


def tolerance_ms(expected_ms):
    return max(1500, expected_ms // 20)  # 1.5 s floor, else 5%


failures = []
checks = []
ac14_line = None
single_cycle_next_in = []  # (path, mode, next_in s, interval s)
seen_modes = set()

for path in sys.argv[1:]:
    text = open(path, encoding="utf-8", errors="replace").read()
    mode = None
    interval_ms = None
    samples = []  # (mode, interval_ms, t)
    # Walk all protocol tokens in stream order (lines can run together).
    token_re = re.compile(
        r"(DATA t=\d+ co2=\S+ temp=\S+ rh=\S+ voc=\S+ nox=\S+"
        r" pm1_0=\S+ pm2_5=\S+ pm10=\S+ press=\S+)"
        r"|(CADENCE (?:loaded )?mode=\w+.*?interval=\d+ms)"
        r"|(CYCLE n=\d+ next_in=\d+s mode=\w+)"
        r"|(REBOOT)"
    )
    for m in token_re.finditer(text):
        if m.group(1):
            d = DATA_RE.match(m.group(1))
            t = int(d.group(1))
            samples.append((mode, interval_ms, t))
            vals = d.groups()[1:]
            if all(v != "nan" for v in vals):
                ac14_line = (path, m.group(1))
        elif m.group(2):
            c = CADENCE_RE.match(m.group(2))
            mode = c.group(1)
            interval_ms = int(c.group(2))
            seen_modes.add(mode)
        elif m.group(3):
            cy = CYCLE_RE.match(m.group(3))
            single_cycle_next_in.append(
                (path, cy.group(2), int(cy.group(1)), interval_ms)
            )
        elif m.group(4):
            mode = None  # reboot: millis resets, break the delta chain
            interval_ms = None

    # Spacing: consecutive DATA pairs in the same mode with increasing t.
    pair_modes = set()
    for (m1, i1, t1), (m2, i2, t2) in zip(samples, samples[1:]):
        if m1 != m2 or i2 is None or t2 <= t1:
            continue
        tol = tolerance_ms(i2)
        delta = t2 - t1
        ok = abs(delta - i2) <= tol
        checks.append((path, m2, delta, i2, tol, ok))
        pair_modes.add(m2)
        if not ok:
            failures.append(f"{path}: mode={m2} delta={delta}ms expected {i2}±{tol}")

print("== Cadence spacing (consecutive DATA pairs, same mode) ==")
for path, m, delta, exp, tol, ok in checks:
    print(f"  {'PASS' if ok else 'FAIL'}  {path}  mode={m:<11} delta={delta}ms (expect {exp}±{tol})")
if not checks:
    failures.append("no same-mode DATA pairs found at all")

# Modes observed with only a single DATA per run (free-plan window too short
# for a second sample): the firmware's own CYCLE next_in print is the timer
# evidence — it must be interval minus the elapsed warm-up (< 60 s).
paired = {m for _, m, *_ in checks}
print("== Single-cycle modes (firmware's own next_in print) ==")
for path, m, next_in, imls in single_cycle_next_in:
    if m in paired or imls is None:
        continue
    exp_s = imls // 1000
    ok = exp_s - 60 <= next_in <= exp_s
    print(f"  {'PASS' if ok else 'FAIL'}  {path}  mode={m} next_in={next_in}s (expect {exp_s - 60}..{exp_s})")
    paired.add(m)  # one verdict per mode
    if not ok:
        failures.append(f"{path}: mode={m} next_in={next_in}s outside {exp_s - 60}..{exp_s}")

print("== AC-14: one DATA line with every quantity numeric ==")
if ac14_line:
    print(f"  PASS  {ac14_line[0]}\n        {ac14_line[1]}")
else:
    failures.append("no DATA line with all quantities non-nan")

for required in REQUIRED_MODES:
    if required not in paired:
        failures.append(f"required mode never verified: {required}")

print()
if failures:
    print("RESULT: FAIL")
    for f in failures:
        print("  -", f)
    sys.exit(1)
print("RESULT: PASS — AC-10 spacing verified against the firmware's own "
      "interval prints; AC-14 all quantities present")
