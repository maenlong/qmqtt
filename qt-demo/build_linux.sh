#!/usr/bin/env bash
set -euo pipefail

# 脚本内配置非空时严格使用指定目录；留空时才扫描并让用户选择。
CONFIG_QTDIR="/opt/Qt/5.15.2/gcc_64"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_FILE="$SCRIPT_DIR/mqtt-client.pro"
BUILD_DIR="${BUILD_DIR:-$SCRIPT_DIR/build/linux-release}"
JOBS="${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)}"
ENV_QMAKE="${QMAKE:-}"

QT_CANDIDATES=()
QT_VERSIONS=()

add_qmake()
{
    local candidate="$1"
    local existing=""
    local version="unknown"

    if [[ ! -x "$candidate" ]]; then
        return
    fi
    candidate="$(cd "$(dirname "$candidate")" && pwd)/$(basename "$candidate")"

    for existing in "${QT_CANDIDATES[@]:-}"; do
        if [[ "$existing" == "$candidate" ]]; then
            return
        fi
    done

    version="$("$candidate" -query QT_VERSION 2>/dev/null || echo unknown)"
    QT_CANDIDATES+=("$candidate")
    QT_VERSIONS+=("$version")
}

resolve_qmake()
{
    local candidate=""
    local index=0

    if [[ -n "$CONFIG_QTDIR" ]]; then
        QMAKE="$CONFIG_QTDIR/bin/qmake"
        if [[ ! -x "$QMAKE" ]]; then
            echo "[ERROR] Configured qmake not found: $QMAKE"
            exit 1
        fi
        QT_VERSION="$("$QMAKE" -query QT_VERSION)"
        echo "[Qt] Using configured Qt $QT_VERSION: $QMAKE"
        return
    fi

    if [[ -n "${QTDIR:-}" ]]; then
        add_qmake "$QTDIR/bin/qmake"
    fi
    if [[ -n "$ENV_QMAKE" ]] && command -v "$ENV_QMAKE" >/dev/null 2>&1; then
        add_qmake "$(command -v "$ENV_QMAKE")"
    fi
    while IFS= read -r candidate; do
        add_qmake "$candidate"
    done < <(type -a -p qmake qmake-qt5 2>/dev/null || true)
    for candidate in "$HOME"/Qt/*/*/bin/qmake /opt/Qt/*/*/bin/qmake /usr/lib/qt*/bin/qmake; do
        add_qmake "$candidate"
    done

    if [[ ${#QT_CANDIDATES[@]} -eq 0 ]]; then
        echo "[ERROR] No Qt installation found in QTDIR, QMAKE, PATH or common Qt directories."
        exit 1
    fi

    echo "[Qt] Found installations:"
    for ((index = 0; index < ${#QT_CANDIDATES[@]}; ++index)); do
        printf '  [%d] Qt %s - %s\n' "$((index + 1))" "${QT_VERSIONS[index]}" "${QT_CANDIDATES[index]}"
    done

    if [[ -z "${QT_CHOICE:-}" ]]; then
        read -r -p "Select Qt [1]: " QT_CHOICE || true
    fi
    QT_CHOICE="${QT_CHOICE:-1}"
    if [[ ! "$QT_CHOICE" =~ ^[0-9]+$ ]] || ((QT_CHOICE < 1 || QT_CHOICE > ${#QT_CANDIDATES[@]})); then
        echo "[ERROR] Invalid Qt selection: $QT_CHOICE"
        exit 1
    fi

    index=$((QT_CHOICE - 1))
    QMAKE="${QT_CANDIDATES[index]}"
    QT_VERSION="${QT_VERSIONS[index]}"
    echo "[Qt] Selected Qt $QT_VERSION: $QMAKE"
}

resolve_qmake
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo "========================================"
echo "Build mqtt-client (qmqtt demo, Linux)"
echo "Qt $QT_VERSION: $QMAKE"
echo "Build directory: $BUILD_DIR"
echo "========================================"
echo ""

echo "[1/3] Generating Makefile..."
"$QMAKE" "$PROJECT_FILE" CONFIG+=release
echo ""

echo "[2/3] Compiling..."
make -j"$JOBS"
echo ""

echo "[3/3] Packaging..."
if command -v linuxdeployqt >/dev/null 2>&1; then
    linuxdeployqt "$BUILD_DIR/mqtt-client" -appimage
else
    echo "[WARN] linuxdeployqt not found; skipped AppImage packaging."
fi
echo ""

echo "========================================"
echo "Done! Run: $BUILD_DIR/mqtt-client"
echo "========================================"
