# Schematic Overview - Air Quality Sensor PCB

## Block Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                      POWER SUBSYSTEM                            │
│                                                                 │
│  USB-C ──┬──► TP4056 ──┬──► Battery (3.7V LiPo 500mAh)         │
│          │   Charger   │                                        │
│          │             └──► ESP32-C3 ──► 3.3V Regulator ──┐    │
│          │                   (Built-in)                    │    │
│          └───────────────────────────────────────────────┘    │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
                                │
                                │ 3.3V Power Rail
                                │
                ┌───────────────┼───────────────┐
                │               │               │
                ▼               ▼               ▼
┌──────────────────┐  ┌──────────────────┐  ┌──────────────────┐
│   SCD41 Sensor   │  │   SGP41 Sensor   │  │  SSD1306 OLED    │
│  CO2/Temp/RH     │  │    VOC / NOx     │  │   0.96" 128x64   │
│                  │  │                  │  │                  │
│  I2C: 0x62       │  │  I2C: 0x59       │  │  I2C: 0x3C       │
│  VDD ──► 3.3V    │  │  VDD ──► 3.3V    │  │  VDD ──► 3.3V    │
│  SDA ◄─► I2C Bus │  │  SDA ◄─► I2C Bus │  │  SDA ◄─► I2C Bus │
│  SCL ◄─── I2C Bus│  │  SCL ◄─── I2C Bus│  │  SCL ◄─── I2C Bus│
│  GND ──► GND     │  │  VSS ──► GND     │  │  GND ──► GND     │
└──────────────────┘  └──────────────────┘  └──────────────────┘
                                │
                                │
                ┌───────────────┴───────────────┐
                │                               │
                │         I2C BUS               │
                │    (SDA = GPIO4, SCL = GPIO5) │
                │                               │
                │    R1: 4.7kΩ to 3.3V (SDA)    │
                │    R2: 4.7kΩ to 3.3V (SCL)    │
                │                               │
                └───────────────┬───────────────┘
                                │
                                ▼
                    ┌────────────────────┐
                    │   ESP32-C3 MCU     │
                    │  Seeed XIAO        │
                    │                    │
                    │  GPIO4 (SDA) ◄──┐  │
                    │  GPIO5 (SCL) ◄──┤  │
                    │  3.3V        ──►│  │
                    │  GND         ──►│  │
                    │  USB-C (Prog)   │  │
                    │  BLE Radio      │  │
                    └────────────────────┘
```

---

## Detailed Pin Connections

### ESP32-C3 (Seeed XIAO) Pinout

```
Left Side (Pins 1-7):          Right Side (Pins 8-14):
1  ── 5V (from USB)            8  ── D4/GPIO4 (I2C SDA)
2  ── GND                      9  ── D5/GPIO5 (I2C SCL)
3  ── 3V3 (output)             10 ── D6/GPIO6 (unused)
4  ── D0/GPIO0 (unused)        11 ── D7/GPIO7 (unused)
5  ── D1/GPIO1 (unused)        12 ── D8/GPIO8 (unused)
6  ── D2/GPIO2 (unused)        13 ── D9/GPIO9 (unused)
7  ── D3/GPIO3 (unused)        14 ── D10/GPIO10 (unused)

USB-C Connector (integrated)
```

**Used Pins**:
- **Pin 3 (3V3)**: Powers all sensors and display
- **Pin 8 (GPIO4/SDA)**: I2C Data line
- **Pin 9 (GPIO5/SCL)**: I2C Clock line
- **Pin 2 (GND)**: Common ground

---

### SCD41 Sensor (CO2/Temperature/Humidity)

```
Pin Configuration:
1 ── VDD   ──► 3.3V (from ESP32-C3)
2 ── GND   ──► Ground
3 ── SDA   ◄─► I2C Data (GPIO4 with 4.7kΩ pullup)
4 ── SCL   ◄── I2C Clock (GPIO5 with 4.7kΩ pullup)
5 ── SEL   ──► Not connected (NC)
6 ── RDY   ──► Optional: Data ready signal (not used)
7 ── PWM   ──► Not connected (NC)
```

**I2C Address**: `0x62` (fixed, not configurable)

**Power Requirements**:
- Supply voltage: 3.3V (±10%)
- Idle current: ~0.5mA
- Measurement current: ~18mA (5 seconds)
- Measurement interval: Every 5 seconds (configurable)

**Notes**:
- Requires airflow for accurate CO2 readings
- Self-calibration over 7 days
- Temperature compensation built-in

---

### SGP41 Sensor (VOC and NOx Index)

```
Pin Configuration:
1 ── VDD   ──► 3.3V (from ESP32-C3)
2 ── VSS   ──► Ground
3 ── SDA   ◄─► I2C Data (GPIO4 with 4.7kΩ pullup)
4 ── SCL   ◄── I2C Clock (GPIO5 with 4.7kΩ pullup)
5 ── VDDH  ──► 3.3V (heater supply, same as VDD)
6 ── NC    ──► Not connected
```

**I2C Address**: `0x59` (fixed, not configurable)

**Power Requirements**:
- Supply voltage: 3.3V (±10%)
- Idle current: ~2.8mA
- Measurement current: ~48mA (during conditioning)
- Conditioning time: 10 seconds on startup

**Notes**:
- MOx (Metal Oxide) sensor requires warm-up
- Returns VOC Index (0-500) and NOx Index (0-500)
- Needs temperature/humidity compensation from SCD41

---

### SSD1306 OLED Display (128x64 pixels)

```
Pin Configuration:
1 ── GND   ──► Ground
2 ── VDD   ──► 3.3V (from ESP32-C3)
3 ── SCL   ◄── I2C Clock (GPIO5 with 4.7kΩ pullup)
4 ── SDA   ◄─► I2C Data (GPIO4 with 4.7kΩ pullup)
```

**I2C Address**: `0x3C` or `0x3D` (check module jumper/solder bridge)

**Power Requirements**:
- Supply voltage: 3.3V - 5V
- Current: ~20mA (all pixels on)
- Typical: ~10mA (mixed content)

**Notes**:
- 0.96" diagonal screen
- High contrast OLED (white on black)
- Can be powered down to save energy

---

### TP4056 Battery Charger

```
Pin Configuration:
1 ── TEMP  ──► NTC thermistor (optional, leave NC)
2 ── PROG  ──► 1.2kΩ resistor to GND (sets 1A charge)
3 ── GND   ──► Ground
4 ── VCC   ──► 5V from USB-C
5 ── BAT   ──► Battery positive
6 ── STDBY ──► Green LED (standby indicator)
7 ── CHRG  ──► Red LED (charging indicator)
8 ── CE    ──► Ground (chip enable, always on)
```

**Charging Specifications**:
- Input: 5V from USB-C
- Output: 4.2V (LiPo standard)
- Charge current: 1000mA (with 1.2kΩ PROG resistor)
- Protection: Over-charge, over-discharge, short circuit

**Alternative**: Use a TP4056 module instead of discrete IC (easier)

---

### Battery Connector (JST PH 2-pin)

```
Pin Configuration:
1 ── BAT+  ──► Red wire (3.7V LiPo positive)
2 ── BAT-  ──► Black wire (Battery ground)
```

**Battery Specifications**:
- Type: Lithium Polymer (LiPo)
- Voltage: 3.7V nominal (4.2V max, 3.0V min)
- Capacity: 500mAh (or 750mAh / 1000mAh for longer runtime)
- Connector: JST PH 2mm pitch

---

## I2C Bus Details

### Bus Configuration

```
           3.3V
            │
       ┌────┴────┐
       │         │
      R1        R2
    4.7kΩ     4.7kΩ
       │         │
       │         │
   ────┴─────────┴──── SDA (GPIO4)
   │    │    │    │
   │    │    │    └──► SSD1306 (0x3C)
   │    │    └───────► SGP41 (0x59)
   │    └────────────► SCD41 (0x62)
   └─────────────────► ESP32-C3 (Master)


   ──────────────────── SCL (GPIO5)
   │    │    │    │
   │    │    │    └──► SSD1306
   │    │    └───────► SGP41
   │    └────────────► SCD41
   └─────────────────► ESP32-C3 (Master)
```

### I2C Address Summary

| Device | Address | Binary | Configurable? |
|--------|---------|--------|---------------|
| SCD41 | 0x62 | 1100010 | No |
| SGP41 | 0x59 | 1011001 | No |
| SSD1306 | 0x3C | 0111100 | Yes (0x3C or 0x3D) |

**No Address Conflicts!** All devices have unique I2C addresses.

### Pullup Resistor Calculation

**Why 4.7kΩ?**
- Standard I2C pullup value
- Works with 400kHz Fast Mode I2C
- Total bus capacitance: ~100pF (3 devices + traces)
- Rise time: ~1μs (within spec)

**Formula**: R = (Vdd - 0.4V) / (0.003A × N)
- Where N = number of devices = 3
- Result: 4.7kΩ is optimal

---

## Power Distribution

### Power Tree

```
USB-C 5V ──┬──► TP4056 Charger ──┬──► Battery 3.7V
           │                     │
           │                     └──► ESP32-C3 Input (VBAT)
           │                              │
           └──────────────────────────────┘
                                          │
                                    LDO Regulator
                                    (built into ESP32-C3)
                                          │
                                        3.3V Rail
                                          │
                          ┌───────────────┼───────────────┐
                          │               │               │
                      SCD41           SGP41          SSD1306
                      18mA            48mA            20mA
                          │               │               │
                          └───────────────┴───────────────┘
                                          │
                                         GND
```

### Power Budget Analysis

| Component | Idle | Active | Peak | Duty Cycle | Avg Power |
|-----------|------|--------|------|------------|-----------|
| ESP32-C3 | 20mA | 80mA | 120mA | 100% | 80mA |
| SCD41 | 0.5mA | 18mA | 18mA | 10% (5s/50s) | 2mA |
| SGP41 | 2.8mA | 48mA | 48mA | 20% (10s/50s) | 12mA |
| SSD1306 | 5mA | 20mA | 20mA | 50% (on/off) | 10mA |
| **Total** | **28mA** | **166mA** | **206mA** | - | **104mA** |

**Battery Life Estimate**:
- 500mAh / 104mA = **4.8 hours** continuous use
- With sleep modes (ESP32 deep sleep): **12-24 hours**

### Decoupling Capacitors

| Capacitor | Location | Value | Purpose |
|-----------|----------|-------|---------|
| C1 | ESP32-C3 VDD pin | 100nF | High-frequency noise filtering |
| C2 | SCD41 VDD pin | 100nF | Sensor power stability |
| C3 | SGP41 VDD pin | 100nF | Heater transient suppression |
| C4 | 3.3V rail | 10μF | Bulk capacitance for load steps |

**Placement**: Place as close as possible to IC power pins (<5mm)

---

## PCB Layout Recommendations

### Component Placement

**Priority Zones**:
```
┌─────────────────────────────────────┐
│  TOP EDGE - USB-C Access            │
│  ┌───────────────────────────┐      │
│  │     Seeed XIAO ESP32-C3   │      │
│  │      (Back Side)          │      │
│  └───────────────────────────┘      │
│                                     │
│  ┌──────┐           ┌──────┐        │
│  │SCD41 │ AIRFLOW→  │SGP41 │        │
│  │ CO2  │           │ VOC  │        │
│  └──────┘           └──────┘        │
│     ↑                   ↑           │
│     └─── Ventilation ───┘           │
│                                     │
│       ┌──────────────┐              │
│       │  SSD1306     │              │
│       │  Display     │              │
│       └──────────────┘              │
│                                     │
│  Battery (Back Side)                │
│  TP4056 (Back Side)                 │
│                                     │
│  BOTTOM EDGE                        │
└─────────────────────────────────────┘
```

### Critical Routing Rules

1. **I2C Traces**
   - Keep SDA and SCL parallel and same length (±10mm)
   - Route on same layer (avoid vias if possible)
   - Minimum trace width: 0.25mm (10 mil)
   - Keep away from noisy signals (PWM, USB data)

2. **Power Traces**
   - 3.3V rail: 0.5mm minimum width
   - Ground: pour on both layers
   - Battery positive: 1.0mm width (handles 500mA+)

3. **Sensor Isolation**
   - Keep ESP32-C3 heat away from sensors
   - Add thermal vias under ESP32-C3
   - Route sensor power traces separately

4. **Ground Strategy**
   - Single point ground (star topology)
   - Battery GND connects at one point
   - Sensor GNDs connect to ground pour

---

## Testing and Validation

### Initial Power-On Checklist

1. **Visual Inspection**
   - [ ] No solder bridges
   - [ ] All components oriented correctly
   - [ ] No cold solder joints

2. **Electrical Tests (Power Off)**
   - [ ] Continuity: 3.3V rail to all VDD pins
   - [ ] Continuity: GND to all GND pins
   - [ ] No shorts: 3.3V to GND (should be open)
   - [ ] Pullup resistors: 4.7kΩ ±5% on SDA and SCL

3. **Power-On Tests (USB Connected, No Battery)**
   - [ ] Measure 3.3V rail (should be 3.25V - 3.35V)
   - [ ] Check current draw (<50mA idle expected)
   - [ ] TP4056 LEDs working (if module used)

4. **I2C Bus Tests**
   - [ ] Run I2C scanner on ESP32-C3
   - [ ] Detect SCD41 at 0x62
   - [ ] Detect SGP41 at 0x59
   - [ ] Detect SSD1306 at 0x3C or 0x3D

5. **Functional Tests**
   - [ ] SCD41 reads CO2 (should be ~400-600ppm in room)
   - [ ] SGP41 returns VOC index (takes 10s to stabilize)
   - [ ] SSD1306 displays text/graphics
   - [ ] BLE advertising works

6. **Battery Tests**
   - [ ] Connect battery, device powers on
   - [ ] Charging works when USB connected
   - [ ] Measure runtime (target: 4-5 hours)

---

## Firmware Integration Notes

### I2C Initialization (PlatformIO)

```cpp
#include <Wire.h>

#define I2C_SDA 4  // GPIO4
#define I2C_SCL 5  // GPIO5
#define I2C_FREQ 100000  // 100kHz (standard mode)

void setup() {
  Wire.begin(I2C_SDA, I2C_SCL, I2C_FREQ);

  // Scan I2C bus
  Serial.println("Scanning I2C bus...");
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("Found device at 0x%02X\n", addr);
    }
  }
}
```

### Expected I2C Scan Output

```
Scanning I2C bus...
Found device at 0x59  ← SGP41
Found device at 0x62  ← SCD41
Found device at 0x3C  ← SSD1306
```

---

## Revision History

| Version | Date | Changes |
|---------|------|---------|
| 1.0 | 2026-01-15 | Initial PCB design |

---

## References

- [I2C Bus Specification](https://www.nxp.com/docs/en/user-guide/UM10204.pdf)
- [PCB Design Best Practices](https://www.ti.com/lit/an/slva959/slva959.pdf)
- Project README: `/README.md`
- Full Design Guide: `/hardware/PCB-DESIGN-GUIDE.md`
