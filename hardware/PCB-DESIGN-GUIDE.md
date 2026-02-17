# Custom PCB Design Guide
## Wearable Air Quality Sensor

### Overview
This document describes the custom PCB design for the air quality sensor badge. The PCB integrates all components from the breadboard prototype into a compact, wearable form factor.

---

## PCB Specifications

### Physical Dimensions
- **Size**: 80mm x 60mm (rectangular for airflow)
- **Thickness**: 1.6mm standard PCB
- **Layers**: 2-layer (Front + Back copper)
- **Weight**: ~25g (PCB + components, excluding battery)

### Form Factor Rationale
- **Rectangular shape**: Allows airflow past sensors (SCD41, SGP41)
- **Dimensions**: Sized between credit card (85x54mm) and business card
- **Wearable**: Can be badge clipped or lanyard mounted
- **Heat dissipation**: ESP32-C3 on back, sensors on front for thermal separation

---

## Component Placement Strategy

### Front Side (Sensor Side)
```
┌─────────────────────────────────────────┐
│  [USB-C on ESP32-C3]         TOP        │
│                                         │
│  ┌──────┐    AIRFLOW →    ┌──────┐     │
│  │SCD41 │                 │SGP41 │     │
│  │ CO2  │                 │ VOC  │     │
│  └──────┘                 └──────┘     │
│                                         │
│           ┌──────────┐                  │
│           │  OLED    │                  │
│           │ Display  │                  │
│           └──────────┘                  │
│                                         │
│              [POWER LED]     BOTTOM    │
└─────────────────────────────────────────┘
```

### Back Side (Electronics Side)
```
┌─────────────────────────────────────────┐
│            [ESP32-C3 Module]            │
│                                         │
│  [R1] [R2]                              │
│  4.7k 4.7k  (I2C Pullups)               │
│                                         │
│  [C1] [C2] [C3]                         │
│  (Decoupling caps)                      │
│                                         │
│  ┌──────────────┐                       │
│  │   Battery    │                       │
│  │   Holder     │                       │
│  │   LiPo       │                       │
│  └──────────────┘                       │
│                                         │
│  [TP4056 Charge IC]                     │
└─────────────────────────────────────────┘
```

---

## Bill of Materials (BOM)

### Main Components

| Ref | Component | Part Number | Quantity | Unit Price | Total | Source | Notes |
|-----|-----------|-------------|----------|------------|-------|---------|-------|
| U1 | ESP32-C3 Module | Seeed XIAO ESP32-C3 | 1 | $5.00 | $5.00 | Seeed Studio | Has USB-C built-in |
| U2 | CO2 Sensor | Sensirion SCD41 | 1 | $49.95 | $49.95 | Adafruit 5190 | **YOU HAVE** |
| U3 | VOC/NOx Sensor | Sensirion SGP41 | 1 | $21.95 | $21.95 | Adafruit 4829 | Need to order |
| U4 | OLED Display | SSD1306 0.96" I2C | 1 | $5.95 | $5.95 | Generic | 128x64 pixels |
| U5 | Battery Charger | TP4056 Module | 1 | $1.00 | $1.00 | Generic | Or discrete IC |
| BT1 | Battery | LiPo 3.7V 500mAh | 1 | $6.95 | $6.95 | Adafruit 1578 | JST connector |

### Passive Components

| Ref | Component | Value | Package | Quantity | Unit Price | Total | Notes |
|-----|-----------|-------|---------|----------|------------|-------|-------|
| R1, R2 | Resistor | 4.7kΩ | 0603 | 2 | $0.10 | $0.20 | I2C pullups |
| R3 | Resistor | 1.2kΩ | 0603 | 1 | $0.10 | $0.10 | Charge current |
| R4 | Resistor | 1kΩ | 0603 | 1 | $0.10 | $0.10 | LED current limit |
| C1, C2, C3 | Capacitor | 100nF | 0603 | 3 | $0.10 | $0.30 | Decoupling |
| C4 | Capacitor | 10μF | 0805 | 1 | $0.20 | $0.20 | Power supply |
| LED1 | LED | Red | 0805 | 1 | $0.15 | $0.15 | Charging indicator |
| LED2 | LED | Green | 0805 | 1 | $0.15 | $0.15 | Power indicator |

### Connectors & Mechanical

| Ref | Component | Type | Quantity | Unit Price | Total | Notes |
|-----|-----------|------|----------|------------|-------|-------|
| J1 | Battery Connector | JST PH 2-pin | 1 | $0.50 | $0.50 | For LiPo battery |
| - | PCB | Custom 80x60mm | 1 | $5.00 | $5.00 | From JLCPCB/PCBWay |
| - | Standoffs | M2.5 x 5mm | 4 | $0.10 | $0.40 | For enclosure |

### **Current Total Cost**: ~$98
*(Assuming you already have the ESP32-C3 and SCD41)*

### **Additional Components Needed**:
- SGP41 sensor: $21.95
- OLED display: $5.95
- TP4056 module or IC: $1.00
- Battery: $6.95
- Passive components: ~$2.00
- PCB fabrication: ~$5.00

**New Components Total**: ~$43

---

## Electrical Design

### Power Architecture

```
USB-C (5V)
    │
    ├─── TP4056 ───┬─── Battery (3.7V LiPo)
    │              │
    │              └─── ESP32-C3 (has built-in 3.3V regulator)
    │                        │
    └────────────────────────┴─── 3.3V Rail
                                      │
                                      ├─── SCD41 (3.3V)
                                      ├─── SGP41 (3.3V)
                                      └─── SSD1306 (3.3V)
```

**Power Budget**:
- ESP32-C3: ~80mA (active BLE)
- SCD41: ~18mA (during measurement)
- SGP41: ~48mA (during measurement)
- SSD1306: ~20mA (active display)
- **Total**: ~166mA peak, ~100mA average

**Battery Life**:
- 500mAh battery / 100mA average = **~5 hours continuous use**
- With sleep modes: **12-24 hours** typical use

### I2C Bus Configuration

All sensors share the same I2C bus (SDA/SCL):
- **SDA**: GPIO4 (ESP32-C3 pin 8)
- **SCL**: GPIO5 (ESP32-C3 pin 9)

**I2C Addresses**:
- SCD41: `0x62` (fixed)
- SGP41: `0x59` (fixed)
- SSD1306: `0x3C` (typical, may be `0x3D`)

**Pullup Resistors**:
- R1, R2: 4.7kΩ to 3.3V (standard I2C pullups)

---

## PCB Layout Guidelines

### Critical Design Rules

1. **Sensor Airflow**
   - Keep SCD41 and SGP41 exposed to ambient air
   - No components blocking sensor openings
   - Consider adding ventilation holes near sensors
   - Mount sensors on edge of PCB if possible

2. **Thermal Management**
   - ESP32-C3 on back side (generates most heat)
   - Sensors on front side (isolated from ESP32 heat)
   - Consider thermal vias under ESP32-C3
   - Keep battery away from hot components

3. **I2C Routing**
   - Keep I2C traces (SDA/SCL) short and parallel
   - Route away from noisy signals (PWM, switching)
   - Add ground plane under I2C traces
   - Place pullup resistors near ESP32-C3

4. **Power Distribution**
   - Wide traces for 3.3V and GND (minimum 0.5mm)
   - Star ground configuration (single point to battery)
   - Decoupling capacitors close to each IC's power pins
   - Separate analog and digital grounds if possible

5. **Battery Safety**
   - Add reverse polarity protection (diode or MOSFET)
   - Fuse or PTC on battery positive line
   - Keep battery traces thick (1mm for 500mA)
   - Add mounting holes for secure battery placement

### Layer Stack (2-Layer PCB)

**Front Copper (Top)**:
- Component pads (sensors, display)
- I2C routing
- 3.3V power traces
- Ground pour

**Back Copper (Bottom)**:
- ESP32-C3 pads
- Battery connections
- Additional ground pour
- Return paths

---

## Manufacturing Files

### What You Need to Order PCBs

When you finalize the design in KiCad, you'll generate:

1. **Gerber Files** (manufacturing data)
   - `.gtl` - Top copper layer
   - `.gbl` - Bottom copper layer
   - `.gts` - Top solder mask
   - `.gbs` - Bottom solder mask
   - `.gto` - Top silkscreen
   - `.gbo` - Bottom silkscreen
   - `.gko` - Board outline

2. **Drill Files**
   - `.drl` or `.txt` - Drill holes and vias

3. **BOM (Bill of Materials)**
   - CSV or Excel file listing all components

4. **CPL (Component Placement List)**
   - For SMT assembly (if ordering assembled boards)

### Recommended PCB Manufacturers

| Vendor | Price (5 pcs) | Lead Time | Assembly | Notes |
|--------|---------------|-----------|----------|-------|
| **JLCPCB** | ~$5 | 2-5 days | Yes | Cheapest, good quality |
| **PCBWay** | ~$10 | 3-7 days | Yes | Better customer service |
| **OSH Park** | ~$25 | 12 days | No | USA-made, purple PCBs |
| **Seeed Fusion** | ~$8 | 3-7 days | Yes | Good for prototypes |

**Recommended**: JLCPCB for first prototype (cheap, fast)

### PCB Specifications to Order

```
- Layers: 2
- Dimensions: 80mm x 60mm
- PCB Thickness: 1.6mm
- Copper Weight: 1 oz (35μm)
- Surface Finish: HASL (or ENIG for better quality)
- Solder Mask Color: Green (or your preference)
- Silkscreen Color: White
- Minimum Track/Spacing: 0.25mm/0.25mm
- Minimum Drill Hole: 0.3mm
```

---

## Assembly Options

### Option 1: Hand Solder (Easiest)
- Order PCBs only (no assembly)
- Use breakout boards for sensors (Adafruit modules)
- Solder through-hole components
- **Time**: 1-2 hours
- **Difficulty**: Beginner
- **Cost**: Lowest ($43 + PCB)

### Option 2: SMT with Breakouts (Recommended)
- Order PCBs with ESP32-C3 pre-soldered
- Hand solder sensor breakout boards
- Use SMT stencil for passives
- **Time**: 30-60 minutes
- **Difficulty**: Intermediate
- **Cost**: Medium ($60 + PCB + assembly)

### Option 3: Full SMT Assembly
- Order fully assembled PCBs from JLCPCB
- They solder everything except sensors
- You add sensors and battery
- **Time**: 10 minutes
- **Difficulty**: Easy
- **Cost**: Highest ($100 + PCB + assembly)

---

## Design Considerations for Airflow

### Sensor Placement Rules

**SCD41 (CO2 Sensor)**:
- Needs continuous airflow for accurate readings
- Mount on edge of PCB or near ventilation holes
- Keep away from user's breath (≥5cm clearance)
- Avoid enclosed spaces (must be exposed)

**SGP41 (VOC/NOx Sensor)**:
- Less sensitive to airflow than SCD41
- Can be slightly more recessed
- Needs warm-up time (conditioning phase)
- Heat from electronics affects readings - keep separated

**Best Practice**:
```
Front view with airflow direction:

    AIRFLOW →

┌────────┐         ┌────────┐
│ SCD41  │ ←air→  │ SGP41  │
└────────┘         └────────┘
    ↓                  ↓
   [PCB surface with slight gap]
```

---

## Testing Plan

### Pre-Assembly Checks
- [ ] Visual inspection of bare PCB (shorts, opens)
- [ ] Continuity test all power rails
- [ ] Check I2C pullup resistors with multimeter

### Assembly Validation
- [ ] Power-on test with USB-C (measure 3.3V rail)
- [ ] I2C bus scan (should detect all 3 addresses)
- [ ] Flash test firmware to ESP32-C3
- [ ] Verify each sensor reads data

### Final Testing
- [ ] Battery charging test (TP4056 LED indicators)
- [ ] Runtime test (log data for 1 hour on battery)
- [ ] BLE connectivity test with mobile app
- [ ] Calibration check (compare with reference sensor)

---

## Next Steps

1. **Open KiCad Project**
   ```bash
   # Open the project file
   kicad hardware/kicad/air-quality-sensor.kicad_pro
   ```

2. **Review Schematic**
   - Check connections match your breadboard
   - Verify I2C addresses
   - Confirm power ratings

3. **Complete PCB Layout**
   - Place components according to airflow guidelines
   - Route I2C traces carefully
   - Add ground pour
   - Run Design Rule Check (DRC)

4. **Generate Manufacturing Files**
   - File → Plot → Generate Gerbers
   - File → Fabrication Outputs → Drill Files
   - Export BOM (File → Fabrication Outputs → BOM)

5. **Order PCBs**
   - Upload Gerber ZIP to JLCPCB/PCBWay
   - Select options (2 layers, 1.6mm, etc.)
   - Order 5 pieces (~$5 + shipping)

6. **Order Components**
   - SGP41 from Adafruit: $21.95
   - OLED display: $5.95
   - Battery + TP4056: ~$8
   - Passive components kit: ~$2

7. **Assembly**
   - Follow assembly option above
   - Test each component as you solder
   - Flash firmware when complete

---

## Troubleshooting

### Common Issues

**I2C devices not detected**:
- Check solder joints on SDA/SCL
- Verify 4.7kΩ pullup resistors installed
- Measure voltage on I2C lines (should be 3.3V at idle)
- Try I2C scanner code

**Battery not charging**:
- Check TP4056 connections
- Verify USB-C provides 5V
- Test with known good LiPo battery
- Check polarity (red=+, black=-)

**Sensors giving incorrect readings**:
- Verify sensors have airflow
- Check for heat from ESP32-C3 affecting sensors
- Run sensor self-test functions
- Compare with reference measurements

**Short battery life**:
- Measure actual current draw with multimeter
- Check for shorts or parasitic drain
- Verify ESP32-C3 sleep mode is working
- Consider larger battery (750mAh or 1000mAh)

---

## Enclosure Design

### Mounting Options

**Option 1: Badge Clip**
- Add holes for standard badge clip (35mm spacing)
- Wear on shirt pocket or lanyard

**Option 2: Wrist Mount**
- Design wrist strap holder
- Velcro or buckle closure

**Option 3: Desk Stand**
- 3D print stand with angled display
- Weighted base for stability

### 3D Printable Case

Key features needed:
- Ventilation grilles aligned with sensors
- Cutout for OLED display window
- Access to USB-C port for charging
- Snap-fit or screw assembly
- Clip or strap mounting points

*Case design files will be added to `/enclosure` folder*

---

## Resources

### Datasheets
- [SCD41 Datasheet](https://sensirion.com/media/documents/E0F04247/631EF271/CD_DS_SCD40_SCD41_Datasheet_D1.pdf)
- [SGP41 Datasheet](https://sensirion.com/media/documents/5FE8673C/61E96F50/Sensirion_Gas_Sensors_Datasheet_SGP41.pdf)
- [ESP32-C3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf)
- [SSD1306 Datasheet](https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf)

### KiCad Tutorials
- [KiCad Quick Start Guide](https://docs.kicad.org/6.0/en/getting_started_in_kicad/getting_started_in_kicad.html)
- [PCB Design Tutorial](https://www.youtube.com/watch?v=vaCVh2SAZY4)

### PCB Manufacturing Guides
- [JLCPCB Capabilities](https://jlcpcb.com/capabilities/Capabilities)
- [How to Order PCBs Guide](https://jlcpcb.com/help/article/how-to-generate-gerber-and-drill-files-in-kicad-6)

---

## License

This PCB design is released under the MIT License. You are free to manufacture, modify, and distribute this design for personal or commercial use.

---

**Document Version**: 1.0
**Last Updated**: 2026-01-15
**Author**: Air Quality Sensor Project
