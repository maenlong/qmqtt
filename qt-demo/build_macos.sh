#!/usr/bin/env bash
set -euo pipefail

ARCH="$(uname -m)"

# 脚本内配置非空时严格使用指定目录；留空时才扫描并让用户选择。
if [[ "$ARCH" == "arm64" ]]; then
    CONFIG_QTDIR="/opt/homebrew/Cellar/qt@5/5.15.19"
else
    CONFIG_QTDIR="$HOME/Qt/5.15.2/clang_64"
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_FILE="$SCRIPT_DIR/mqtt-client.pro"
BUILD_DIR="${BUILD_DIR:-$SCRIPT_DIR/build/macos-$ARCH-release}"
JOBS="${JOBS:-$(sysctl -n hw.logicalcpu 2>/dev/null || echo 1)}"
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
    local brew_qt_dir=""
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
    for candidate in "$HOME"/Qt/*/clang_64/bin/qmake /opt/homebrew/opt/qt@5/bin/qmake /usr/local/opt/qt@5/bin/qmake; do
        add_qmake "$candidate"
    done
    if command -v brew >/dev/null 2>&1; then
        brew_qt_dir="$(brew --prefix qt@5 2>/dev/null || true)"
        if [[ -n "$brew_qt_dir" ]]; then
            add_qmake "$brew_qt_dir/bin/qmake"
        fi
    fi

    if [[ ${#QT_CANDIDATES[@]} -eq 0 ]]; then
        echo "[ERROR] No Qt installation found in QTDIR, QMAKE, PATH, Homebrew or common Qt directories."
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
echo "Build mqtt-client (qmqtt demo, macOS $ARCH)"
echo "Qt $QT_VERSION: $QMAKE"
echo "Build directory: $BUILD_DIR"
echo "========================================"
echo ""

echo "[1/4] Generating Makefile..."
"$QMAKE" "$PROJECT_FILE" CONFIG+=release CONFIG+=sdk_no_version_check
echo ""

echo "[2/4] Compiling..."
make -j"$JOBS"
echo ""

echo "[3/4] Deploying Qt frameworks..."
if command -v macdeployqt >/dev/null 2>&1; then
    macdeployqt "$BUILD_DIR/mqtt-client.app"
else
    echo "[WARN] macdeployqt not found; skipped framework deployment."
fi
echo ""

echo "[4/4] Re-signing app bundle..."
if command -v codesign >/dev/null 2>&1; then
    codesign --deep --force --sign - "$BUILD_DIR/mqtt-client.app"
else
    echo "[WARN] codesign not found; skipped ad-hoc signing."
fi
echo ""

echo "========================================"
echo "Done! Run: open $BUILD_DIR/mqtt-client.app"
echo "========================================"
