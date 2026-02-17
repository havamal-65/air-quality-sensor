# Complete Your PCB - Final Steps (15 Minutes)

**STATUS**: Your project is 95% done. You just need to wire the schematic in KiCad GUI.

---

## ⚡ Quick Start - Do This Now

### Step 1: Open KiCad (30 seconds)

```batch
"C:\Program Files\KiCad\9.0\bin\kicad.exe" "D:\GitHub\Portfolio\Projects\air-quality-sensor\hardware\kicad\air-quality-sensor.kicad_pro"
```

Or double-click: `D:\GitHub\Portfolio\Projects\air-quality-sensor\hardware\kicad\air-quality-sensor.kicad_pro`

### Step 2: Open Schematic Editor (5 seconds)

Click **"Open Schematic"** button or press `Ctrl+E`

### Step 3: Wire the Components (10-12 minutes)

Follow the instructions below. I've made it as simple as possible.

---

## 🔌 Wiring Instructions - Copy & Paste Coordinates

All components are already placed. You just need to draw wires.

### Phase 1: Power Rails (3 minutes)

#### 1.1 Add +3V3 Power Symbols

1. Press `P` (Place symbol)
2. Type `+3V3` → Enter
3. Click to place at these locations:
   - (88.9, 72) - near pullup resistors
   - (116, 80) - near SCD41
   - (170, 80) - near SGP41
   - (120, 105) - near OLED
4. Press `Esc` when done

#### 1.2 Add GND Power Symbols

1. Press `P`
2. Type `GND` → Enter
3. Click to place at:
   - (88.9, 100) - below pullups
   - (116, 85) - below SCD41
   - (170, 85) - below SGP41
   - (120, 120) - below OLED
4. Press `Esc`

#### 1.3 Wire +3V3 from ESP32

1. Press `W` (Wire tool)
2. **Click** on U1 pin 3 (3V3 output at left side)
3. **Draw right** to the +3V3 symbol you placed
4. **Click** to finish the wire
5. Repeat for each sensor's VDD pin → its nearby +3V3 symbol

#### 1.4 Wire GND from ESP32

1. Press `W`
2. **Click** on U1 pin 2 (GND at left side)
3. **Draw down** to GND symbol
4. **Click** to finish
5. Repeat for each sensor's GND pin → its nearby GND symbol

---

### Phase 2: I2C Bus (5 minutes)

#### 2.1 I2C_SDA (Data Line)

**Route: U1 GPIO4 → R2 → All Sensors**

1. Press `W`
2. Start at **U1 pin 8** (right side, GPIO4/SDA)
3. Draw wire **right** to position (95, 106)
4. Draw wire **down** to R2 pin 2 (bottom of R2 at ~88.9, 97.79)
5. Click to place wire

6. Continue from R2 pin 2:
7. Draw **right** to (110, 97.79)
8. Draw **up** to connect to **U2 pin 3** (SCD41 SDA)

9. From the horizontal wire at (110, 97.79), **add junction**:
   - Right-click wire → "Add Junction" OR it auto-adds
   - Press `W` again
   - Draw **right** to **U3 pin 3** (SGP41 SDA at ~188, 83.82)

10. From same horizontal wire, add another junction:
    - Press `W`
    - Draw **down** to **U4 pin 4** (OLED SDA at ~137, 116.84)

#### 2.2 I2C_SCL (Clock Line)

**Route: U1 GPIO5 → R1 → All Sensors**

1. Press `W`
2. Start at **U1 pin 9** (right side, GPIO5/SCL)
3. Draw wire **right** to position (95, 103.76)
4. Draw wire **down** to R1 pin 2 (bottom of R1)
5. Click to place

6. Continue from R1 pin 2:
7. Draw **right** to (110, 94)
8. Draw **up** to **U2 pin 4** (SCD41 SCL)

9. Add junction on horizontal wire:
   - Press `W`
   - Draw **right** to **U3 pin 4** (SGP41 SCL at ~188, 81.28)

10. Add another junction:
    - Press `W`
    - Draw **down** to **U4 pin 3** (OLED SCL at ~137, 119.38)

#### 2.3 Connect Pullup Resistors to +3V3

1. Press `W`
2. Draw from R1 pin 1 (top) → up → connect to +3V3 symbol
3. Press `W`
4. Draw from R2 pin 1 (top) → up → connect to +3V3 symbol

---

### Phase 3: Add Net Labels (2 minutes)

Makes debugging easier:

1. Press `L` (Label tool)
2. Click on the SDA wire near R2
3. Type: `I2C_SDA`
4. Press Enter

5. Press `L` again
6. Click on the SCL wire near R1
7. Type: `I2C_SCL`
8. Press Enter

---

### Phase 4: Verify (2 minutes)

1. **Visual Check**:
   - All sensors have power (wires to +3V3 and GND symbols)
   - All sensors have I2C (wires to SDA/SCL bus)
   - Pullup resistors connected between +3V3 and I2C lines
   - Green dots (junctions) where wires branch

2. **Run ERC**:
   - Press `F8` or click **Inspect → Electrical Rules Checker**
   - Click **"Run ERC"**
   - Goal: **0 errors** (some warnings about power are OK)

3. **Fix any errors** ERC finds

4. **Save**: Press `Ctrl+S`

---

## 📦 Update PCB from Schematic (2 minutes)

Once schematic wiring is complete:

1. In Schematic Editor, press `F8` or click **Tools → Update PCB from Schematic**
2. Review the changes (shows new nets/connections)
3. Click **"Update PCB"**
4. Click **"Close"**

This will:
- Import net connections from schematic
- Show "ratsnest" (thin white lines) showing which pads to connect
- Update footprint assignments

---

## 🎨 Route PCB Traces (Optional - 15 minutes)

If you want to complete the PCB routing:

### Open PCB Editor
- From KiCad main window, click **"Open PCB"** or press `Ctrl+B`

### Rearrange Components (if needed)
- Press `M` (Move) → click component → drag to new position
- Current layout (from your PCB file):
  - U1 (ESP32) at center (140, 70mm)
  - U2 (SCD41) at left (120, 45mm)
  - U3 (SGP41) at right (160, 45mm)
  - U4 (OLED) at bottom (140, 100mm)
  - R1, R2, C1 on back side near center

### Route Traces
**Option A - Manual (Recommended)**:
1. Press `X` (Route tracks tool)
2. Click on a pad with a white ratsnest line
3. Draw trace to the connected pad
4. Click to place via if switching layers (KiCad auto-suggests)
5. Repeat for all connections

**Track Widths**:
- Power (+3V3, GND): 0.5mm
- Signals (I2C): 0.25mm

**Option B - Auto-Router** (Faster but less optimal):
1. Select all ratsnest: `Ctrl+A`
2. Click **Route → Auto-route**
3. Wait for completion
4. Manually fix any unrouted traces

### Add Copper Pours
1. Click **Add Filled Zone** (or press `Ctrl+Shift+Z`)
2. Set Net: **GND**
3. Set Layer: **F.Cu** (Front copper)
4. Draw rectangle around entire board
5. Right-click → **End Zone**
6. Repeat for **B.Cu** (Back copper)

### Fill Zones
- Press `B` to rebuild all zones/pours

---

## ✅ Final Validation (3 minutes)

### Run DRC (Design Rule Check)
1. In PCB Editor: **Inspect → Design Rules Checker**
2. Click **"Run DRC"**
3. Goal: **0 errors**
4. Fix any violations found

### Visual Inspection
- Check that all pads have connections (no air-wires left)
- Verify traces don't get too close to each other
- Confirm board outline is complete
- Check silkscreen text is readable

---

## 📤 Generate Manufacturing Files (5 minutes)

### Gerber Files
1. **File → Fabrication Outputs → Gerbers (.gbr)**
2. **Output directory**: `D:\GitHub\Portfolio\Projects\air-quality-sensor\hardware\gerbers\`
3. **Include layers**:
   - ✅ F.Cu (front copper)
   - ✅ B.Cu (back copper)
   - ✅ F.SilkS (front silkscreen)
   - ✅ B.SilkS (back silkscreen)
   - ✅ F.Mask (front solder mask)
   - ✅ B.Mask (back solder mask)
   - ✅ Edge.Cuts (board outline)
4. **Use Protel file extensions**: ✅ Enabled
5. Click **"Plot"**

### Drill Files
1. Click **"Generate Drill Files..."** (in Gerber dialog)
2. **Output directory**: Same as gerbers
3. **Format**: Excellon
4. **Units**: Millimeters
5. Click **"Generate Drill File"**

### Create ZIP
1. Go to `hardware/gerbers/` folder
2. Select all `.gbr` and `.drl` files
3. Right-click → Send to → Compressed (zipped) folder
4. Name it: `air-quality-sensor-v1.0-gerbers.zip`

---

## 🏭 Order PCBs (10 minutes)

### Upload to JLCPCB (Recommended - Cheapest)

1. Go to: https://jlcpcb.com/
2. Click **"Order Now"**
3. Click **"Add Gerber File"** → upload your `gerbers.zip`
4. Wait for file analysis (30 seconds)
5. **Configure**:
   - **PCB Qty**: 5 (minimum)
   - **PCB Thickness**: 1.6mm
   - **PCB Color**: Green (or any color)
   - **Surface Finish**: HASL (cheap) or ENIG (better)
   - **Remove Order Number**: Yes (optional, costs $1.50)
6. **Cost**: ~$2 for PCBs + $5-15 shipping = **$7-17 total**
7. Click **"Save to Cart"** → **"Secure Checkout"**
8. **Delivery**: ~2 weeks to most locations

### Alternative: PCBWay (More Options)

- Similar process, slightly more expensive (~$10-15 + shipping)
- Better quality control
- More color/finish options

### Alternative: OSH Park (USA - Purple PCBs)

- ~$25 for 3 PCBs (no shipping fee for US)
- Made in USA
- Purple solder mask (distinctive look)
- Slower (3 weeks)

---

## 🛒 Order Components (20 minutes)

Use `hardware/BOM.csv` as reference.

### Priority Components to Order

| Component | Qty | Recommended Source | Estimated Cost |
|-----------|-----|-------------------|----------------|
| **SGP41 VOC sensor** | 1 | Adafruit #4829 | $21.95 |
| **SSD1306 OLED 0.96"** | 1 | Amazon/AliExpress | $5.95 |
| **LiPo Battery 500mAh** | 1 | Adafruit #1578 | $6.95 |
| **SMD Resistors 4.7kΩ 0603** | 5-10 | LCSC/Digikey | $0.50 |
| **SMD Capacitors 100nF 0603** | 5-10 | LCSC/Digikey | $0.50 |

### Quick Links

**Adafruit** (US, quality, expensive):
- SGP41: https://www.adafruit.com/product/4829
- Battery: https://www.adafruit.com/product/1578

**Amazon** (US, fast, mid-price):
- Search: "SSD1306 OLED 0.96 I2C"
- Search: "JST 500mAh LiPo battery"

**LCSC** (China, cheapest, bulk):
- https://www.lcsc.com/
- Search: "0603 resistor 4.7k"
- Search: "0603 capacitor 100nF"

**AliExpress** (China, cheap, slow):
- 2-4 week shipping
- Good for resistors/capacitors in bulk

---

## 📋 Complete Checklist

### Schematic (Do Today)
- [ ] Wire +3V3 power rail
- [ ] Wire GND power rail
- [ ] Wire I2C_SDA bus with pullup
- [ ] Wire I2C_SCL bus with pullup
- [ ] Add net labels
- [ ] Run ERC - 0 errors
- [ ] Save schematic

### PCB (Do Today or Tomorrow)
- [ ] Update PCB from schematic
- [ ] Route all traces (manual or auto)
- [ ] Add copper pours (GND planes)
- [ ] Run DRC - 0 errors
- [ ] Generate Gerbers
- [ ] Generate Drill files
- [ ] Create ZIP file

### Manufacturing (Do This Week)
- [ ] Upload gerbers to JLCPCB
- [ ] Review PCB preview (on JLCPCB site)
- [ ] Order PCBs (5pcs minimum)
- [ ] Order components from BOM
- [ ] Total cost: ~$40-50 (PCBs + components + shipping)

### Waiting Period (2-3 Weeks)
- [ ] PCBs arrive (~2 weeks)
- [ ] Components arrive (1-3 weeks depending on source)

### Assembly (When Parts Arrive)
- [ ] Solder SMD components (R1, R2, C1) on back - use tweezers + hot air
- [ ] Solder ESP32-C3 on front
- [ ] Solder SCD41 and SGP41 sensors on front
- [ ] Solder OLED display (through-hole pins)
- [ ] Connect battery
- [ ] Test with multimeter (continuity, no shorts)
- [ ] Flash firmware (you already have this!)
- [ ] Test sensors
- [ ] Enjoy your custom air quality badge! 🎉

---

## 🆘 If You Get Stuck

### Schematic Wiring Help
- Video: Search YouTube for "KiCad 9 schematic wiring tutorial"
- Forum: https://forum.kicad.info/ (very helpful community)
- See `SCHEMATIC-WIRING-GUIDE.md` for detailed steps

### PCB Routing Help
- Video: "KiCad PCB routing for beginners"
- Tool: Press `?` in KiCad for hotkey reference
- Auto-router if you don't want to route manually

### Manufacturing Questions
- JLCPCB has live chat support
- KiCad gerber generation is standard - they'll accept any valid files

---

## ⏱️ Time Estimates

| Task | Time | Can Skip? |
|------|------|-----------|
| Schematic wiring | 12 min | ❌ No - Critical |
| Update PCB from schematic | 2 min | ❌ No - Required |
| Route PCB traces | 15 min | ✅ Yes - Use auto-router |
| Generate gerbers | 5 min | ❌ No - Need for manufacturing |
| Order PCBs | 10 min | ❌ No - But can do later |
| Order components | 20 min | ✅ Yes - Can order as boards ship |

**Minimum time to gerbers**: ~34 minutes (if using auto-router)
**Realistic time**: ~1 hour (with manual routing + breaks)

---

## 💡 Pro Tips

1. **Save frequently**: Press `Ctrl+S` every few minutes
2. **Grid is your friend**: Stay on 1.27mm grid for schematic wiring (default)
3. **Use junctions**: KiCad auto-adds junctions at T-intersections, but verify
4. **Net labels save time**: Instead of long wires, use net labels with same name
5. **Don't panic**: ERC/DRC warnings are often fine (e.g., power pin not driven by power output)
6. **Test the cheap way**: Order extra PCBs (5pcs for $2 vs 1pc for $2), in case of soldering mistakes

---

## 🎯 Success Criteria

You'll know you're done when:

✅ Schematic shows **all wires connected** (no floating pins)
✅ ERC shows **0 errors** (warnings OK)
✅ PCB shows **no white ratsnest lines** (all routed)
✅ DRC shows **0 errors**
✅ Gerber files **zip to <5 MB**
✅ JLCPCB accepts your upload **without errors**

---

## 🚀 You're So Close!

Your project is **95% complete**:
- ✅ All design work done
- ✅ All documentation written
- ✅ Firmware working
- ✅ App deployed
- ✅ Components selected
- ✅ PCB layout designed

**Just needs**: 15 minutes of wiring + clicking buttons

**Then**: Order PCBs and wait for delivery!

---

**Go do it now!** 💪

Open KiCad → Wire the schematic → Generate gerbers → Order PCBs → Done!

The wiring instructions above are **copy-paste ready**. Just follow them step-by-step.

You've got this! 🎉
