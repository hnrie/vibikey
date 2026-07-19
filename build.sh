#!/usr/bin/env bash
# Build VibiKey into a single-file executable using PyInstaller or Nuitka.
#
# Usage:
#   ./build.sh                     # PyInstaller onefile (default)
#   ./build.sh --tool nuitka       # Nuitka onefile
#   ./build.sh -t nuitka           # same, short form (-t == --tool)
#   ./build.sh --no-onefile        # folder build (onedir/standalone) for debugging
#   ./build.sh --arch x86 --compiler clang
#   ./build.sh --clean
#   ./build.sh --help
#
# Requires: Python 3.11+ and a C compiler (gcc or clang).
# Nuitka onefile: pip install "Nuitka[app]"
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
NAME="VibiKey"
ENTRY="$ROOT/run_ui.py"
CORE_DIR="$ROOT/vibikey_core"
LOCALES="$ROOT/locales"

show_help() {
    cat <<EOF
Usage: $0 [options]

Build VibiKey into a single-file executable using PyInstaller or Nuitka.

Options:
  -t, --tool pyinstaller|nuitka  Packaging backend (default: pyinstaller)
  --onefile                   Build a single self-extracting executable (default)
  --no-onefile                Folder build (onedir / standalone) for debugging
  -a, --arch x86|x64           Target architecture (default: x64)
  -c, --compiler gcc|clang    Native-core compiler (default: gcc)
  -p, --python PATH           Python interpreter to use
                              (default: ../.venv/bin/python or python3)
  --clean                     Remove build artifacts and exit
  -h, --help                  Show this help and exit

Output:
  dist/VibiKey-<arch>-<compiler>-<tool>   single binary (onefile, default)

Requirements:
  Python 3.11+; gcc or clang; PySide6-Essentials, pynput
  pyinstaller or Nuitka[app]. See requirements.txt.
EOF
}

TOOL="pyinstaller"
ONEFILE=1
CLEAN=0
PYTHON=""
ARCH="x64"
COMPILER="gcc"

while [ $# -gt 0 ]; do
    case "$1" in
        --help|-h) show_help; exit 0 ;;
        -t|--tool) TOOL="$2"; shift 2 ;;
        --onefile) ONEFILE=1; shift ;;
        --no-onefile) ONEFILE=0; shift ;;
        --clean) CLEAN=1; shift ;;
        -p|--python) PYTHON="$2"; shift 2 ;;
        -a|--arch) ARCH="$2"; shift 2 ;;
        -c|--compiler) COMPILER="$2"; shift 2 ;;
        *) echo "Unknown option: $1" >&2; exit 1 ;;
    esac
done

if [ -z "$PYTHON" ]; then
    if [ -x "$ROOT/../.venv/bin/python" ]; then PYTHON="$ROOT/../.venv/bin/python"
    else PYTHON="python3"; fi
fi

clean_artifacts() {
    rm -rf "$ROOT/build" "$ROOT/dist" "$ROOT/__pycache__" \
           "$ROOT/$NAME.build" "$ROOT/$NAME.dist" "$ROOT/$NAME.onefile-build" \
           "$ROOT/run_ui.build" "$ROOT/run_ui.dist" "$ROOT/run_ui.onefile-build" \
           "$ROOT"/*.spec 2>/dev/null || true
}

if [ "$CLEAN" -eq 1 ]; then clean_artifacts; echo "Cleaned."; exit 0; fi

if [ "$(uname)" = "Darwin" ]; then
    CORE_LIB="$CORE_DIR/libvibikey_core.dylib"
elif [ "$(uname)" = "Linux" ]; then
    CORE_LIB="$CORE_DIR/libvibikey_core.so"
else
    CORE_LIB="$CORE_DIR/vibikey_core.dll"
fi

if [ ! -f "$CORE_LIB" ]; then
    echo "Native core not found - building it..."
    MFLAG=""
    if [ "$ARCH" = "x86" ]; then MFLAG="-m32"; fi
    if [ "$(uname)" = "Linux" ]; then
        CC="$COMPILER"
        "$CC" -shared -fPIC -O2 $MFLAG -o "$CORE_LIB" \
            "$CORE_DIR/vietnamese_tep.c" 2>&1 || {
            echo "Failed to build the native core (need $CC)." >&2; exit 1; }
    elif [ "$(uname)" = "Darwin" ]; then
        cc -shared -fPIC -O2 -o "$CORE_LIB" \
            "$CORE_DIR/vietnamese_tep.c" 2>&1 || {
            echo "Failed to build the native core (need clang)." >&2; exit 1; }
    fi
    if [ ! -f "$CORE_LIB" ]; then
        echo "Failed to build the native core. Build it manually first." >&2; exit 1
    fi
    echo "core built: $CORE_LIB"
fi

SUFFIX="$ARCH-$COMPILER"
TAG="$NAME-$SUFFIX-$TOOL"
FINAL="$ROOT/dist/$TAG"

clean_artifacts
mkdir -p "$ROOT/dist"

STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT
find "$CORE_DIR" -maxdepth 1 -type f \( -name '*.so' -o -name '*.dylib' -o -name '*.dll' \) \
    -exec cp {} "$STAGE/" \;

if [ "$ONEFILE" -eq 1 ]; then
    echo "Mode: ONEFILE (single binary) / tool=$TOOL"
else
    echo "Mode: folder / tool=$TOOL"
fi

if [ "$TOOL" = "pyinstaller" ]; then
    if ! "$PYTHON" -c "import PyInstaller" 2>/dev/null; then
        "$PYTHON" -m pip install --quiet --upgrade --break-system-packages pyinstaller 2>/dev/null || "$PYTHON" -m pip install --quiet --upgrade pyinstaller || true
    fi
    ARGS=(-m PyInstaller --noconfirm --clean --name "$TAG" --windowed
          --add-binary "$STAGE:vibikey_core"
          --add-data "$LOCALES:locales")
    if [ "$ONEFILE" -eq 1 ]; then ARGS+=(--onefile); else ARGS+=(--onedir); fi
    ARGS+=("$ENTRY")
    "$PYTHON" "${ARGS[@]}"
    if [ "$ONEFILE" -eq 1 ]; then
        BUILT="$ROOT/dist/$TAG"
    else
        BUILT="$ROOT/dist/$TAG/$TAG"
    fi
    if [ ! -f "$BUILT" ]; then
        echo "PyInstaller output missing: $BUILT" >&2; exit 1
    fi
    if [ "$BUILT" != "$FINAL" ]; then
        cp -f "$BUILT" "$FINAL"
    fi
    OUT="$FINAL"
elif [ "$TOOL" = "nuitka" ]; then
    if ! "$PYTHON" -c "import nuitka" 2>/dev/null; then
        "$PYTHON" -m pip install --quiet --upgrade --break-system-packages 'Nuitka[app]' ordered-set zstandard 2>/dev/null \
            || "$PYTHON" -m pip install --quiet --upgrade 'Nuitka[app]' ordered-set zstandard || true
    else
        "$PYTHON" -m pip install --quiet --upgrade --break-system-packages ordered-set zstandard 2>/dev/null \
            || "$PYTHON" -m pip install --quiet --upgrade ordered-set zstandard || true
    fi
    if [ "$ONEFILE" -eq 1 ]; then MODE="--onefile"; else MODE="--standalone"; fi
    NARCH=""
    [ "$ARCH" != "x64" ] && NARCH="--target-arch=$ARCH"
    OUTDIR="$ROOT/dist/$TAG-build"
    NUITKA_LIBS=()
    for f in "$STAGE"/*; do
        [ -e "$f" ] && NUITKA_LIBS+=("--include-data-files=$f=vibikey_core/$(basename "$f")")
    done
    "$PYTHON" -m nuitka "$MODE" $NARCH \
        --enable-plugin=pyside6 \
        --static-libpython=no \
        --assume-yes-for-downloads \
        --remove-output \
        --output-filename="$TAG" \
        --output-dir="$OUTDIR" \
        --include-data-dir="$LOCALES=locales" \
        "${NUITKA_LIBS[@]}" \
        "$ENTRY"
    if [ "$ONEFILE" -eq 1 ]; then
        BUILT="$OUTDIR/$TAG"
    else
        BUILT="$OUTDIR/run_ui.dist/$TAG"
    fi
    if [ ! -f "$BUILT" ]; then
        BUILT="$(find "$OUTDIR" -type f -perm -111 \( -name "$TAG" -o -name 'run_ui*' -o -name "$NAME*" \) 2>/dev/null | head -n1 || true)"
    fi
    if [ -z "${BUILT:-}" ] || [ ! -f "$BUILT" ]; then
        echo "Nuitka output missing under $OUTDIR" >&2; exit 1
    fi
    cp -f "$BUILT" "$FINAL"
    chmod +x "$FINAL"
    OUT="$FINAL"
else
    echo "Unknown tool: $TOOL (use pyinstaller or nuitka)" >&2; exit 1
fi

if [ -f "$OUT" ]; then
    SIZE=$(du -h "$OUT" | cut -f1)
    echo "Build OK -> $OUT ($SIZE)"
else
    echo "Build finished but expected output not found: $OUT"
    echo "Check the dist/ folder."
    exit 1
fi
