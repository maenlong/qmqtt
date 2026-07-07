#!/bin/bash
set -euo pipefail

# ======================== CONFIG ========================
# Adjust to your Qt installation path
QTDIR=/opt/Qt/5.15.2/gcc_64
# ========================================================

export PATH="$QTDIR/bin:/usr/bin:$PATH"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

echo "========================================"
echo "Build mqtt-client (qmqtt demo, Linux)"
echo "Qt: $QTDIR"
echo "========================================"
echo ""

# Clean
if [ -d mqtt-client ]; then
    echo "Removing existing mqtt-client..."
    rm -rf mqtt-client
fi
rm -f Makefile

echo "[1/3] qmake:"
qmake mqtt-client.pro CONFIG+=release CONFIG+=QMQTT_WEBSOCKETS
echo ""

echo "[2/3] Compiling..."
make -j"$(nproc)"
echo ""

echo "[3/3] Deploying Qt libraries..."
linuxdeployqt mqtt-client -appimage
echo ""

echo "========================================"
echo "Done! Run: ./mqtt-client"
echo "========================================"
