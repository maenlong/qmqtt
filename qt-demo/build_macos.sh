#!/bin/bash
set -euo pipefail

# ======================== CONFIG ========================
# Override QTDIR here if needed, otherwise auto-detected
# QTDIR=~/Qt/5.15.2/clang_64

ARCH=$(uname -m)
case "$ARCH" in
    x86_64)
        DEFAULT_QTDIR=~/Qt/5.15.2/clang_64
        ;;
    arm64)
        DEFAULT_QTDIR=/opt/homebrew/Cellar/qt@5/5.15.19
        ;;
    *)
        echo "[ERROR] Unsupported architecture: $ARCH"
        exit 1
        ;;
esac
QTDIR="${QTDIR:-$DEFAULT_QTDIR}"
# ========================================================

export PATH="$QTDIR/bin:/usr/bin:$PATH"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

echo "========================================"
echo "Build mqtt-client (qmqtt demo, macOS $ARCH)"
echo "Qt: $QTDIR"
echo "========================================"
echo ""

# Clean
if [ -d mqtt-client.app ]; then
    echo "Removing existing mqtt-client.app..."
    rm -rf mqtt-client.app
fi
rm -f Makefile

echo "[1/3] qmake:"
qmake mqtt-client.pro CONFIG+=release CONFIG+=sdk_no_version_check CONFIG+=QMQTT_WEBSOCKETS
echo ""

echo "[2/3] Compiling..."
make -j"$(sysctl -n hw.logicalcpu)"
echo ""

echo "[3/4] Deploying Qt frameworks..."
macdeployqt mqtt-client.app
echo ""

echo "[4/4] Re-signing app bundle..."
codesign --deep -f -s - mqtt-client.app
echo ""

echo "========================================"
echo "Done! Run: open mqtt-client.app"
echo "========================================"
