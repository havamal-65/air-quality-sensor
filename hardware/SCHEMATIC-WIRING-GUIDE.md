# Schematic Wiring Guide - Air Quality Sensor

## Status: Legacy Manual Wiring Guide

> Note (2026-02-21): This document is kept for legacy/manual workflow reference.
> The current MCP-generated KiCad design lives in `hardware/kicad_mcp/` and includes
> a populated schematic and placed PCB. See `hardware/kicad_mcp/README.md`.

The schematic file `hardware/kicad/air-quality-sensor.kicad_sch` currently contains all the component symbols but **no wire connections**. This guide will walk you through completing the schematic wiring in KiCad.

**Why Manual Wiring?**: For this legacy path, manual wiring in KiCad GUI is the fallback workflow.

---

## Prerequisites

1. **Open KiCad 9.0.7**
2. **Open the project**: `D:\GitHub\Portfolio\Projects\air-quality-sensor\hardware\kicad\air-quality-sensor.kicad_pro`
3. **Open Schematic Editor** (click "Open Schematic" or press Ctrl+E)

---

## Current Schematic Layout

The schematic already contains these components at these approximate positions:

| Component | Reference | Type | Position (X, Y mm) |
|-----------|-----------|------|--------------------|
| ESP32-C3 | U1 | MCU Module | (50.8, 88.9) |
| SCD41 | U2 | CO2 Sensor | (127, 76.2) |
| SGP41 | U3 | VOC Sensor | (177.8, 76.2) |
| OLED | U4 | Display | (127, 111.76) |
| TP4056 | U5 | Battery Charger | (50.8, 149.86) |
| Battery | BT1 | LiPo 3.7V 500mAh | (101.6, 149.86) |
| Resistor R1 | R1 | 4.7kΩ 0603 | (88.9, 81.28) |
| Resistor R2 | R2 | 4.7kΩ 0603 | (88.9, 93.98) |
| Capacitor C1 | C1 | 100nF 0603 | (106.68, 76.2) |
| GND Symbol | #PWR01 | Power | (35.56, 104.14) |
| +3V3 Symbol | #PWR02 | Power | (35.56, 71.12) |

---

## Step-by-Step Wiring Instructions

### Phase 1: I2C Bus Power Rails

#### 1.1 Create +3V3 Power Rail

1. **Add +3V3 Power Symbols**:
   - Press `P` → Search for "power" → Select "+3V3"
   - Place above R1 and R2 (around position X=88.9, Y=75)
   - Place near each sensor's VDD pin

2. **Wire +3V3 from ESP32-C3**:
   - Press `W` (Wire tool)
   - Click on U1 pin 3 (3V3 output)
   - Draw wire to the right
   - Connect to +3V3 power symbol

3. **Connect +3V3 to Pullup Resistors**:
   - Draw wires from +3V3 symbol to:
     - R1 pin 1 (top pin)
     - R2 pin 1 (top pin)

4. **Connect +3V3 to Sensors**:
   - Draw wires from +3V3 symbols to:
     - U2 (SCD41) pin 1 (VDD)
     - U3 (SGP41) pin 1 (VDD)
     - U3 (SGP41) pin 5 (VDDH - heater power)
     - U4 (OLED) pin 2 (VDD)

#### 1.2 Create GND Power Rail

1. **Add GND Power Symbols**:
   - Press `P` → Search for "power" → Select "GND"
   - Place below each component that needs ground
   - Place below C1 capacitor

2. **Wire GND from ESP32-C3**:
   - Press `W`
   - Click on U1 pin 2 (GND)
   - Draw wire down
   - Connect to GND power symbol

3. **Connect GND to All Components**:
   - Draw wires from GND symbols to:
     - U2 (SCD41) pin 2 (GND)
     - U3 (SGP41) pin 2 (VSS)
     - U4 (OLED) pin 1 (GND)
     - C1 pin 2 (bottom pin)

---

### Phase 2: I2C Signal Lines

#### 2.1 I2C SDA (Data Line)

**Net Name**: `I2C_SDA` or just leave as implicit connection

1. **ESP32-C3 GPIO4 (SDA) → R2 (Pullup)**:
   - Press `W`
   - Click on U1 pin 8 (D4/GPIO4/SDA)
   - Draw wire to the right
   - Connect to R2 pin 2 (bottom pin)

2. **R2 → SCD41 SDA**:
   - Continue wire from R2 pin 2
   - Draw to the right
   - Connect to U2 (SCD41) pin 3 (SDA)

3. **Branch to SGP41 SDA**:
   - From the horizontal wire between R2 and SCD41:
     - Right-click wire → "Add Junction" (or it auto-adds)
     - Press `W` again
     - Draw wire upward/rightward to U3 (SGP41) pin 3 (SDA)

4. **Branch to OLED SDA**:
   - From the same horizontal wire:
     - Add junction
     - Draw wire downward to U4 (OLED) pin 4 (SDA)

**Pro Tip**: Use Net Labels to make debugging easier:
   - Press `L` (Label tool)
   - Click on the SDA wire
   - Type: `I2C_SDA`
   - Press Enter

#### 2.2 I2C SCL (Clock Line)

**Net Name**: `I2C_SCL`

1. **ESP32-C3 GPIO5 (SCL) → R1 (Pullup)**:
   - Press `W`
   - Click on U1 pin 9 (D5/GPIO5/SCL)
   - Draw wire to the right
   - Connect to R1 pin 2 (bottom pin)

2. **R1 → SCD41 SCL**:
   - Continue wire from R1 pin 2
   - Draw to the right
   - Connect to U2 (SCD41) pin 4 (SCL)

3. **Branch to SGP41 SCL**:
   - From the horizontal wire between R1 and SCD41:
     - Add junction (auto or manual)
     - Draw wire to U3 (SGP41) pin 4 (SCL)

4. **Branch to OLED SCL**:
   - From the same horizontal wire:
     - Add junction
     - Draw wire downward to U4 (OLED) pin 3 (SCL)

**Add Net Label**:
   - Press `L`
   - Click on SCL wire
   - Type: `I2C_SCL`

---

### Phase 3: Power Decoupling

#### 3.1 Connect Decoupling Capacitor C1

C1 is already placed near the I2C bus. Wire it between power rails:

1. **C1 Top Pin → +3V3**:
   - Press `W`
   - Connect C1 pin 1 (top) to the nearby +3V3 power symbol

2. **C1 Bottom Pin → GND**:
   - Already should be near a GND symbol
   - Connect C1 pin 2 (bottom) to GND symbol

**Purpose**: C1 provides local charge reservoir for I2C bus transients, improving signal integrity.

---

### Phase 4: Unused/Optional Pins

#### 4.1 SCD41 Additional Pins

- **Pin 5 (SEL)**: Address select - leave unconnected (NC) or tie to GND for default address 0x62
- **Pin 6 (RDY)**: Ready output - leave unconnected (optional interrupt pin)
- **Pin 7 (PWM)**: PWM select - leave unconnected

**If you want to connect SEL to GND**:
- Add "No Connect" flag: Press `Q` → click on pin 5
- Or wire pin 5 to GND symbol

#### 4.2 SGP41 Additional Pins

- **Pin 6 (NC)**: No connect - add "No Connect" flag (Press `Q`)

#### 4.3 ESP32-C3 Unused GPIO Pins

For pins 4, 5, 6, 7, 10-14 (D0-D3, D6-D10):
- Leave unconnected for now (available for future expansion)
- Add "No Connect" flags if desired (Press `Q` → click each pin)

---

### Phase 5: Battery and Charging Circuit

#### 5.1 Battery to TP4056

1. **Battery + to TP4056 BAT**:
   - Press `W`
   - Connect BT1 pin 1 (+) to U5 (TP4056) pin 5 (BAT)

2. **Battery - to TP4056 GND**:
   - Connect BT1 pin 2 (-) to U5 pin 3 (GND)

#### 5.2 TP4056 to USB Power (from ESP32-C3)

The ESP32-C3 has USB-C built-in which provides 5V when connected.

1. **ESP32-C3 5V → TP4056 VCC**:
   - Connect U1 pin 1 (5V) to U5 pin 4 (VCC)
   - **Label this net**: `USB_5V`

2. **TP4056 GND → Common Ground**:
   - Already connected to BT1
   - Add GND symbol near TP4056 pin 3

#### 5.3 Battery Output to ESP32-C3 (Optional Circuit)

**Note**: The Seeed XIAO ESP32-C3 has onboard charging, so the TP4056 might be redundant. You have two options:

**Option A - Use TP4056 (Recommended for external battery)**:
- Wire TP4056 BAT output to ESP32-C3 power input
- Requires adding a diode or load switch

**Option B - Use ESP32-C3 Built-in Charging (Simpler)**:
- Remove TP4056 from schematic
- Connect battery directly to ESP32-C3 BAT pads (check Seeed datasheet)

For this guide, we'll **simplify and remove TP4056** since ESP32-C3 XIAO has built-in battery management:

1. In schematic, select U5 (TP4056)
2. Press `Delete`
3. Wire battery directly:
   - BT1 pin 1 (+) → ESP32-C3 BAT+ pad (if available on your footprint)
   - BT1 pin 2 (-) → GND

---

## Phase 6: Add Power Symbols Everywhere

For professional schematics, add power symbols to avoid long wires:

1. **For every VDD/3V3 pin**: Add local "+3V3" symbol nearby
2. **For every GND pin**: Add local "GND" symbol nearby
3. **KiCad automatically connects** all symbols with the same name (no wires needed across the page)

This makes the schematic much cleaner!

**Example**:
```
U2 (SCD41):
  Pin 1 (VDD) --- [short wire] --- +3V3 symbol
  Pin 2 (GND) --- [short wire] --- GND symbol
  Pin 3 (SDA) --- [wire to I2C bus]
  Pin 4 (SCL) --- [wire to I2C bus]
```

---

## Phase 7: Verify Connections

### 7.1 Visual Check

Look for:
- ✅ All power pins connected to +3V3 or GND symbols
- ✅ All I2C pins (SDA/SCL) connected to bus
- ✅ Pullup resistors between +3V3 and I2C lines
- ✅ No floating pins (except intentional NC pins)
- ✅ Junctions where wires cross/branch (green dots)

### 7.2 Electrical Rules Check (ERC)

1. Click **Inspect → Electrical Rules Checker** (or press F8)
2. Click **Run ERC**
3. Review warnings/errors:
   - **Power pin not driven**: Check if power symbols are placed correctly
   - **Pin connected to other pin of same type**: Usually OK for power nets
   - **Unconnected pins**: Add "No Connect" flags where appropriate
4. Fix any real errors (warnings about power symbols can often be ignored)

---

## Phase 8: Assign Footprints

Before generating the PCB, every component needs a physical footprint assigned.

### 8.1 Open Footprint Assignment Tool

1. Click **Tools → Assign Footprints** (or press Ctrl+F)
2. This opens the "Assign Footprints to Symbols" window

### 8.2 Assign Each Component

| Reference | Symbol | Recommended Footprint | Library |
|-----------|--------|-----------------------|---------|
| U1 | ESP32-C3 XIAO | `Module:Seeed_XIAO` | RF_Module (or create custom) |
| U2 | SCD41 | `Sensor:Sensirion_SCD4x` | Sensor |
| U3 | SGP41 | `Sensor:Sensirion_SFM_DFN-6_2x2mm_P0.65mm` | Sensor |
| U4 | OLED 0.96" | `Display_OLED:OLED_128x64_0.96inch_SSD1306` | Display |
| BT1 | Battery | `Battery:BatteryHolder_Keystone_1042_1x18650` | Battery |
| R1, R2 | 4.7kΩ | `Resistor_SMD:R_0603_1608Metric` | Resistor_SMD |
| C1 | 100nF | `Capacitor_SMD:C_0603_1608Metric` | Capacitor_SMD |

**To assign**:
1. Select component in middle pane (e.g., U1)
2. Browse footprints in right pane
3. Double-click footprint to assign
4. Repeat for all components
5. Click **OK** when done

**Note**: If footprints don't exist (like for SCD41/SGP41), you may need to:
- Download footprint libraries from manufacturers
- Or create custom footprints (File → New Library → Footprint Editor)

---

## Phase 9: Update PCB from Schematic

Once schematic wiring and footprints are complete:

1. **Save schematic**: Ctrl+S
2. **Update PCB**: Click **Tools → Update PCB from Schematic** (or press F8)
3. **Review changes**:
   - New components will be added
   - Nets (wire connections) will be created as "ratsnest" (thin lines)
4. Click **Update PCB**
5. **Arrange components** on PCB as shown in `PCB-LAYOUT-COMPLETE.md`
6. **Route traces** manually or with auto-router

---

## Phase 10: Generate Gerber Files

After PCB routing is complete:

1. **Run DRC**: Inspect → Design Rules Checker
   - Fix all errors
   - Warnings about copper pours are usually OK
2. **Generate Gerbers**: File → Fabrication Outputs → Gerbers
   - Output directory: `hardware/gerbers/`
   - Include all layers: F.Cu, B.Cu, F.SilkS, B.SilkS, F.Mask, B.Mask, Edge.Cuts
3. **Generate Drill Files**: File → Fabrication Outputs → Drill Files
   - Same output directory
   - Format: Excellon
4. **Zip files**: Compress all gerber + drill files into `gerbers.zip`
5. **Upload to JLCPCB/PCBWay** for manufacturing quote

---

## Quick Reference: KiCad Schematic Shortcuts

| Shortcut | Action |
|----------|--------|
| `W` | Start wire |
| `P` | Add symbol (Place) |
| `L` | Add net label |
| `Q` | Add "No Connect" flag |
| `M` | Move component |
| `R` | Rotate component |
| `E` | Edit component properties |
| `Del` | Delete selected |
| `Esc` | Cancel current operation |
| `F8` | Run ERC |
| `Ctrl+S` | Save |
| `Ctrl+F` | Assign footprints |

---

## Troubleshooting

### "No Connect" Warnings in ERC

**Symptom**: ERC shows "Pin not connected" warnings

**Solution**:
- For intentionally unused pins: Press `Q` → click pin to add "No Connect" flag
- For power pins: Make sure power symbols (+3V3, GND) are placed and connected

### Rats Nest Lines Don't Appear in PCB

**Symptom**: After updating PCB from schematic, no connection lines visible

**Solution**:
- In PCB editor, press `Ctrl+B` to show ratsnest
- Or: View menu → Ratsnest

### Footprint Not Found

**Symptom**: "Footprint not in any library" error

**Solution**:
- Open Footprint Libraries Manager: Preferences → Manage Footprint Libraries
- Ensure all standard libraries are enabled
- Download missing libraries from GitHub (kicad-footprints repository)
- Or create custom footprint

### Symbol Pins Won't Connect

**Symptom**: Wire tool won't snap to pin

**Solution**:
- Zoom in closer (mouse wheel or `+` key)
- Make sure you're clicking exactly on the pin connection point (small circle at end of pin)
- Pins must be on 1.27mm grid - press `Ctrl+Shift+G` to show grid

---

## Expected Time

- **Phase 1-2 (Power & I2C wiring)**: 10-15 minutes
- **Phase 3-5 (Decoupling & battery)**: 5 minutes
- **Phase 6-7 (Cleanup & ERC)**: 5 minutes
- **Phase 8 (Footprints)**: 10 minutes
- **Phase 9-10 (PCB update & Gerbers)**: 20-30 minutes

**Total**: ~50-65 minutes for complete schematic-to-gerber workflow

---

## Next Steps

After completing this guide:

1. ✅ Schematic fully wired and passing ERC
2. ✅ All footprints assigned
3. ✅ PCB updated from schematic
4. ⏭️ Route PCB traces (see `PCB-LAYOUT-COMPLETE.md`)
5. ⏭️ Run DRC on PCB
6. ⏭️ Generate gerbers and order PCBs!

---

## Need Help?

- **KiCad Documentation**: https://docs.kicad.org/9.0/en/
- **KiCad Forum**: https://forum.kicad.info/
- **Tutorial Videos**: Search YouTube for "KiCad 9 schematic tutorial"
- **This Project Issues**: https://github.com/YOUR_USERNAME/air-quality-sensor/issues

---

**Created**: 2026-01-18
**KiCad Version**: 9.0.7
**Status**: Ready for manual wiring ✅
