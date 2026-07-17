# CatKey

A free, open-source Vietnamese input method (bộ gõ tiếng Việt) with an
EVKey-style interface.

- **UI/UX:** Python + PySide6
- **Core (conversion + system keyboard hook):** C
- **Input methods:** Telex and VNI (word-level, correct tone placement,
  full uppercase support: Ê, Ô, Ơ, Ă, Đ, ...)
- **Languages:** English / Tiếng Việt (switchable in-app)

> Tiếng Việt: xem [`docs/README_vi.md`](docs/README_vi.md).

---

## Why verify before you trust a build

**Only run CatKey builds you can verify.** This project deliberately
forbids obfuscated/packed binaries and requires independent verification
before distribution (see [`LICENSE`](LICENSE)).

Here is why this matters. UniKey is the best-known Vietnamese IME. Its
**official** website is **unikey.org**. A separate site, **unikey.vn**,
is *not* the official project and has been reported to distribute UniKey
builds bundled with unwanted/possibly-malicious software, without a valid
digital signature. Because an input method sees every keystroke you type,
a tampered build is extremely dangerous.

CatKey's rule: a distributed build must come from published source, be
reproducible, ship checksums, and be reproduced by at least one
independent party before it is offered for download. If a CatKey download
is packed, unsigned, or you can't match its checksum to a reproducible
build from source — **do not run it.**

---

## Quick start (from source)

Requirements: Python 3.11+, a C compiler (MSVC or GCC/Clang).

```powershell
python -m venv .venv
.\.venv\Scripts\Activate.ps1
pip install PySide6-Essentials pynput
python run_ui.py
```

The C core (`catkey_core.dll` / `libcatkey_core.so`) is built
automatically on first run if a compiler is available.

## Features

- System-wide typing into any app (Notepad, browsers, chat, ...).
- Tray icon: left-click toggles Vietnamese/English; double-click opens
  settings.
- Global toggle hotkey (default `Ctrl+Shift`).
- Conflict detection for other Vietnamese IMEs (UniKey/EVKey/OpenKey/...).
- Single-instance guard (prevents doubled keystrokes).

## Building a distributable

```powershell
# PyInstaller (default) or Nuitka; add -OneFile for a single exe
.\build.ps1 -Tool pyinstaller
.\build.ps1 -Tool nuitka -OneFile
```

```bash
./build.sh --tool pyinstaller
./build.sh --tool nuitka --onefile
```

If you plan to share the build with others, follow the verification
requirements in [`LICENSE`](LICENSE) and the notes in [`docs/`](docs/).

## Documentation

- [`docs/README_vi.md`](docs/README_vi.md) — Vietnamese README.

## License

CatKey is licensed under **CC BY-NC-SA 4.0** with additional terms
(no commercial sale, no obfuscated/packed redistribution, verify before
distributing). See [`LICENSE`](LICENSE) and
[`CC-BY-NC-SA LICENSE`](CC-BY-NC-SA%20LICENSE).
