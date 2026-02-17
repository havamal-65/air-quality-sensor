"""
Automated PCB routing pipeline:
1. Remove keepout zones that block routing
2. Remove existing bad tracks
3. Export DSN for FreeRouting
4. Run FreeRouting auto-router
5. Import routed SES back into KiCad
"""

import pcbnew
import subprocess
import sys
import os
import re
from pathlib import Path

# Paths
PCB_PATH = Path(r"D:\GitHub\Portfolio\Projects\air-quality-sensor\hardware\kicad\air-quality-sensor.kicad_pcb")
DSN_PATH = PCB_PATH.parent / "freerouting.dsn"
TEMP_DSN_PATH = PCB_PATH.parent / "temp-freerouting.dsn"
SES_PATH = PCB_PATH.parent / "freerouting.ses"
FREEROUTING_JAR = Path(r"D:\GitHub\freerouting\integrations\KiCad\kicad-freerouting\plugins\jar\freerouting-1.9.0.jar")
JAVA_PATH = r"C:\Program Files\Eclipse Adoptium\jre-17.0.8.101-hotspot\bin\java.exe"

# Step flags - set to False to skip a step
DO_FIX_BOARD = True
DO_EXPORT_DSN = True
DO_ROUTE = True
DO_IMPORT_SES = True


def fix_board():
    """Remove keepout zones and bad tracks, save the board."""
    print("=" * 60)
    print("STEP 1: Fixing board (removing keepouts and bad tracks)")
    print("=" * 60)

    board = pcbnew.LoadBoard(str(PCB_PATH))

    # Remove all keepout (rule area) zones
    zones_to_remove = []
    for zone in board.Zones():
        if zone.GetIsRuleArea():
            zones_to_remove.append(zone)
            print(f"  Removing keepout zone on layer {zone.GetLayerName()}")

    for zone in zones_to_remove:
        board.Remove(zone)
    print(f"  Removed {len(zones_to_remove)} keepout zones")

    # Remove tracks with no net or that are dangling/shorting
    tracks_to_remove = []
    for track in board.GetTracks():
        net = track.GetNet()
        net_name = net.GetNetname() if net else ""
        # Remove tracks with no net assignment
        if not net_name:
            tracks_to_remove.append(track)

    for track in tracks_to_remove:
        board.Remove(track)
    print(f"  Removed {len(tracks_to_remove)} unassigned tracks")

    # Save
    pcbnew.SaveBoard(str(PCB_PATH), board)
    print(f"  Board saved to {PCB_PATH}")
    print()


def export_dsn():
    """Export the PCB to Specctra DSN format."""
    print("=" * 60)
    print("STEP 2: Exporting DSN file")
    print("=" * 60)

    # Clean up old files
    for f in [DSN_PATH, TEMP_DSN_PATH, SES_PATH]:
        if f.exists():
            f.unlink()

    board = pcbnew.LoadBoard(str(PCB_PATH))
    ok = pcbnew.ExportSpecctraDSN(board, str(TEMP_DSN_PATH))

    if not ok or not TEMP_DSN_PATH.exists():
        print("  ERROR: Failed to export DSN file!")
        sys.exit(1)

    # Clean up DSN file (remove Unicode characters that FreeRouting can't handle)
    with open(TEMP_DSN_PATH, "r", encoding="utf-8") as fr:
        first_line = True
        with open(DSN_PATH, "w", encoding="utf-8") as fw:
            for line in fr:
                if first_line:
                    fw.write(f"(pcb {DSN_PATH.name}\n")
                    first_line = False
                else:
                    # Remove characters that cause issues
                    cleaned = re.sub('[ΩµΦ]', '', line)
                    fw.write(cleaned)

    TEMP_DSN_PATH.unlink(missing_ok=True)
    print(f"  DSN exported to {DSN_PATH}")
    print()


def run_freerouting():
    """Run FreeRouting auto-router on the DSN file."""
    print("=" * 60)
    print("STEP 3: Running FreeRouting auto-router")
    print("=" * 60)

    if not FREEROUTING_JAR.exists():
        print(f"  ERROR: FreeRouting JAR not found at {FREEROUTING_JAR}")
        sys.exit(1)

    if not DSN_PATH.exists():
        print(f"  ERROR: DSN file not found at {DSN_PATH}")
        sys.exit(1)

    cmd = [
        JAVA_PATH,
        "-jar", str(FREEROUTING_JAR),
        "-de", str(DSN_PATH),
        "-do", str(SES_PATH),
        "-mp", "100",  # max passes
        "-host", "KiCad",
    ]

    print(f"  Running: {' '.join(cmd)}")
    print("  This may take a minute...")
    print()

    result = subprocess.run(cmd, capture_output=True, text=True, timeout=300)

    if result.stdout:
        print("  STDOUT:", result.stdout[:2000])
    if result.stderr:
        print("  STDERR:", result.stderr[:2000])

    if result.returncode != 0:
        print(f"  WARNING: FreeRouting exited with code {result.returncode}")
    else:
        print("  FreeRouting completed successfully")

    if SES_PATH.exists():
        print(f"  Session file created: {SES_PATH}")
    else:
        print("  WARNING: No SES file was generated!")
    print()


def import_ses():
    """Import the routed SES file back into the PCB."""
    print("=" * 60)
    print("STEP 4: Importing routed design")
    print("=" * 60)

    if not SES_PATH.exists():
        print(f"  ERROR: SES file not found at {SES_PATH}")
        sys.exit(1)

    board = pcbnew.LoadBoard(str(PCB_PATH))
    ok = pcbnew.ImportSpecctraSES(board, str(SES_PATH))

    if ok:
        pcbnew.SaveBoard(str(PCB_PATH), board)
        print("  Routed design imported and saved successfully!")
    else:
        print("  ERROR: Failed to import SES file!")
        sys.exit(1)

    # Clean up temp files
    DSN_PATH.unlink(missing_ok=True)
    SES_PATH.unlink(missing_ok=True)
    print()


def main():
    print("\n" + "=" * 60)
    print("  AIR QUALITY SENSOR - PCB AUTO-ROUTING PIPELINE")
    print("=" * 60 + "\n")

    if DO_FIX_BOARD:
        fix_board()

    if DO_EXPORT_DSN:
        export_dsn()

    if DO_ROUTE:
        run_freerouting()

    if DO_IMPORT_SES:
        import_ses()

    print("=" * 60)
    print("  DONE! Open the PCB in KiCad to verify the routing.")
    print("=" * 60)


if __name__ == "__main__":
    main()
