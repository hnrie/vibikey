"""
CatKey - detect other Vietnamese input tools running in the background.

Pure stdlib (no psutil dependency). On Windows uses `tasklist`, on Linux
uses /proc. Returns friendly names of any conflicting IME processes so the
UI can warn the user (two Vietnamese IMEs active at once garbles typing).
"""

import os
import sys
import subprocess

# process-name (lowercase, without path) -> friendly product name
KNOWN_TOOLS = {
    # Windows
    "unikey.exe": "UniKey",
    "unikeynt.exe": "UniKey",
    "evkey.exe": "EVKey",
    "evkey64.exe": "EVKey",
    "evkey32.exe": "EVKey",
    "openkey.exe": "OpenKey",
    "gotiengviet.exe": "GoTiengViet",
    "vietkey.exe": "Vietkey",
    "winvnkey.exe": "WinVNKey",
    "evkau.exe": "EVKey (updater)",
    # Linux
    "ibus-engine-bogo": "BoGo (IBus)",
    "ibus-bamboo": "Bamboo (IBus)",
    "fcitx5-unikey": "UniKey (Fcitx5)",
    "fcitx-unikey": "UniKey (Fcitx)",
    "ibus-unikey": "UniKey (IBus)",
    "x-unikey": "x-unikey",
}

# Our own process names to ignore.
_SELF = {"python.exe", "pythonw.exe", "catkey.exe", "catkey_msvc.exe"}


def _running_process_names_windows():
    try:
        out = subprocess.run(
            ["tasklist", "/fo", "csv", "/nh"],
            capture_output=True, text=True, timeout=5,
            creationflags=0x08000000,  # CREATE_NO_WINDOW
        ).stdout
    except Exception:
        return []
    names = []
    for line in out.splitlines():
        # "name.exe","pid",...
        if line.startswith('"'):
            names.append(line.split('","', 1)[0].strip('"').lower())
    return names


def _running_process_names_linux():
    names = []
    try:
        for pid in os.listdir("/proc"):
            if not pid.isdigit():
                continue
            try:
                with open(f"/proc/{pid}/comm", "r") as f:
                    names.append(f.read().strip().lower())
            except OSError:
                continue
    except OSError:
        pass
    return names


def detect_conflicts():
    """Return a sorted list of friendly names of conflicting IMEs found."""
    if sys.platform == "win32":
        running = _running_process_names_windows()
    else:
        running = _running_process_names_linux()

    found = set()
    for name in running:
        if name in _SELF:
            continue
        friendly = KNOWN_TOOLS.get(name)
        if friendly:
            found.add(friendly)
    return sorted(found)
