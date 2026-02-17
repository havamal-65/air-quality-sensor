"""
Automate KiCad PCB Editor to export Specctra DSN file.

Opens KiCad PCB editor, navigates File → Export → Specctra DSN,
saves the file, and closes the dialog.

Requires: pyautogui, KiCad 9 installed
"""

import subprocess
import time
import sys
import os

# Try pywinauto first (more reliable for Windows GUI automation)
try:
    from pywinauto import Application, Desktop, keyboard
    USE_PYWINAUTO = True
except ImportError:
    USE_PYWINAUTO = False
    import pyautogui
    pyautogui.FAILSAFE = True

PCB_PATH = r"D:\GitHub\Portfolio\Projects\air-quality-sensor\hardware\kicad\air-quality-sensor.kicad_pcb"
DSN_PATH = r"D:\GitHub\Portfolio\Projects\air-quality-sensor\hardware\kicad\freerouting.dsn"
KICAD_CLI = r"C:\Program Files\KiCad\9.0\bin\pcbnew.exe"


def wait_for_window(title_fragment, timeout=30):
    """Wait for a window with the given title fragment to appear."""
    start = time.time()
    while time.time() - start < timeout:
        if USE_PYWINAUTO:
            try:
                windows = Desktop(backend="uia").windows()
                for w in windows:
                    if title_fragment.lower() in w.window_text().lower():
                        return w
            except Exception:
                pass
        time.sleep(1)
    return None


def export_dsn_pyautogui():
    """Use PyAutoGUI keyboard shortcuts to export DSN."""
    print("Launching KiCad PCB Editor...")
    proc = subprocess.Popen([KICAD_CLI, PCB_PATH])

    print("Waiting for PCB Editor to load...")
    time.sleep(8)  # Give KiCad time to fully load

    # Use keyboard shortcut: Alt+F to open File menu
    print("Opening File menu...")
    pyautogui.hotkey('alt', 'f')
    time.sleep(1)

    # Navigate to Export submenu
    print("Looking for Export...")
    # In KiCad 9, File menu: Export → Specctra DSN
    # Press 'x' for Export or navigate with arrow keys
    pyautogui.press('x')  # Export shortcut
    time.sleep(0.5)

    # Look for Specctra DSN option
    pyautogui.press('s')  # Specctra DSN
    time.sleep(1)

    # Save dialog should appear - type the path
    print("Entering file path...")
    pyautogui.hotkey('ctrl', 'a')  # Select all in filename field
    time.sleep(0.3)
    pyautogui.typewrite(DSN_PATH.replace('\\', '/'), interval=0.02)
    time.sleep(0.5)
    pyautogui.press('enter')  # Save
    time.sleep(2)

    # Check if file was created
    if os.path.exists(DSN_PATH):
        print(f"SUCCESS: DSN file exported to {DSN_PATH}")
        print(f"File size: {os.path.getsize(DSN_PATH)} bytes")
    else:
        print("WARNING: DSN file may not have been created.")
        print("Check KiCad manually.")

    # Close KiCad
    print("Closing PCB Editor...")
    pyautogui.hotkey('alt', 'F4')
    time.sleep(1)

    # Handle "Save changes?" dialog if it appears
    pyautogui.press('tab')  # Move to "Don't Save"
    pyautogui.press('enter')

    return os.path.exists(DSN_PATH)


def export_dsn_ipc():
    """Use KiCad IPC API to trigger DSN export."""
    try:
        from kipy import KiCad

        kicad = KiCad()
        kicad.ping()
        print("Connected to KiCad IPC API!")

        # Try run_action to trigger DSN export
        # The action ID for Specctra DSN export
        result = kicad.run_action("pcbnew.ExportSpecctraDSN", {"filename": DSN_PATH})
        print(f"Action result: {result}")
        return os.path.exists(DSN_PATH)
    except Exception as e:
        print(f"IPC API failed: {e}")
        return False


def main():
    # Remove old DSN file
    if os.path.exists(DSN_PATH):
        os.unlink(DSN_PATH)

    # Try IPC API first (if KiCad is already running)
    print("Attempting IPC API export...")
    if export_dsn_ipc():
        print("DSN exported via IPC API!")
        return True

    # Fall back to GUI automation
    print("\nFalling back to GUI automation...")
    print("DO NOT MOVE THE MOUSE during this process!")
    print("Starting in 3 seconds...")
    time.sleep(3)

    return export_dsn_pyautogui()


if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1)
