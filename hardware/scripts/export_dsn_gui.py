"""
Automate KiCad PCB Editor DSN export via GUI automation.
Uses pyautogui to navigate: File → Export → Specctra DSN...
"""

import pyautogui
import subprocess
import time
import os
import sys

DSN_PATH = r"D:\GitHub\Portfolio\Projects\air-quality-sensor\hardware\kicad\freerouting.dsn"

# Safety settings
pyautogui.FAILSAFE = True
pyautogui.PAUSE = 0.3


def bring_pcbnew_to_front():
    """Bring pcbnew window to the foreground."""
    try:
        import ctypes
        import win32gui
        import win32con

        def callback(hwnd, results):
            if win32gui.IsWindowVisible(hwnd):
                title = win32gui.GetWindowText(hwnd)
                if 'pcb editor' in title.lower() or 'pcbnew' in title.lower() or 'air-quality-sensor' in title.lower():
                    results.append(hwnd)

        results = []
        win32gui.EnumWindows(callback, results)

        if results:
            hwnd = results[0]
            win32gui.ShowWindow(hwnd, win32con.SW_RESTORE)
            win32gui.SetForegroundWindow(hwnd)
            print(f"  Brought window to front: {win32gui.GetWindowText(hwnd)}")
            return True
    except ImportError:
        # Fall back to alt-tab
        pyautogui.hotkey('alt', 'tab')
        time.sleep(0.5)
    return False


def export_dsn():
    """Automate File → Export → Specctra DSN export."""
    # Remove old file
    if os.path.exists(DSN_PATH):
        os.unlink(DSN_PATH)

    print("Bringing PCB Editor to front...")
    bring_pcbnew_to_front()
    time.sleep(1)

    print("Opening File menu (Alt+F)...")
    pyautogui.hotkey('alt', 'f')
    time.sleep(0.8)

    print("Clicking Export...")
    # Navigate to Export submenu - use arrow keys for reliability
    # File menu items vary, so let's find "Export" by pressing down arrow keys
    # In KiCad 9: File → Export → Specctra DSN...
    # "Export" is typically the 4th-5th item in File menu
    for _ in range(10):
        pyautogui.press('down')
        time.sleep(0.1)

    # Actually, let's use the reliable method - type the menu accelerator
    pyautogui.press('escape')  # Close menu first
    time.sleep(0.3)

    # Use the menu bar directly - File → Export → Specctra DSN
    # In KiCad 9, the Specctra DSN export might be under File → Export
    # Let's try the menu navigation more carefully
    pyautogui.hotkey('alt', 'f')
    time.sleep(0.5)

    # Look for Export submenu - screenshot and find it
    # For reliability, let's just navigate with arrow keys
    # File menu order in KiCad 9 PCB editor:
    # New, Open, Save, Save As, Revert, ---
    # Board Setup, Import, Export, ---
    # Fabrication Outputs, Page Setup, Print, Plot
    # Close, Exit

    # Export is roughly item 8-9
    for i in range(8):
        pyautogui.press('down')
        time.sleep(0.15)

    # Enter Export submenu
    pyautogui.press('right')
    time.sleep(0.5)

    # In Export submenu, find Specctra DSN
    # Export submenu items: Specctra DSN, GenCad, IPC-2581, etc.
    # Specctra DSN should be first or near first
    pyautogui.press('down')
    time.sleep(0.15)

    # Click it
    pyautogui.press('enter')
    time.sleep(1.5)

    # Save dialog should appear
    # Clear the filename and type our path
    pyautogui.hotkey('ctrl', 'a')
    time.sleep(0.2)

    # Type the filename
    pyautogui.typewrite(DSN_PATH, interval=0.01)
    time.sleep(0.3)

    # Press Enter/Save
    pyautogui.press('enter')
    time.sleep(2)

    # Check if file was created
    if os.path.exists(DSN_PATH):
        size = os.path.getsize(DSN_PATH)
        print(f"\nSUCCESS! DSN exported: {DSN_PATH} ({size} bytes)")
        return True
    else:
        print("\nFAILED: DSN file was not created.")
        print("You may need to do this manually:")
        print("  File → Export → Specctra DSN...")
        print(f"  Save as: {DSN_PATH}")
        return False


if __name__ == "__main__":
    print("=" * 50)
    print("KiCad DSN Export - GUI Automation")
    print("=" * 50)
    print()
    print("DO NOT move the mouse for the next 10 seconds!")
    print("Starting in 3 seconds...")
    time.sleep(3)

    success = export_dsn()
    sys.exit(0 if success else 1)
