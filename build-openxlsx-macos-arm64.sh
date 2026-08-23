#!/usr/bin/env bash
# Builds OpenXLSX as a static library for macOS ARM64 and copies the result to
# libraries/OpenXLSX/bin/macarm64/libOpenXLSX.a
#
# Prerequisites (install via Homebrew):
#   brew install cmake git
#
# Usage:
#   chmod +x build-openxlsx-macos-arm64.sh
#   ./build-openxlsx-macos-arm64.sh

set -euo pipefail

# Ensure Homebrew tools are on PATH (handles cases where shell init is minimal)
for brew_prefix in /opt/homebrew /usr/local; do
    [ -d "$brew_prefix/bin" ] && export PATH="$brew_prefix/bin:$PATH"
done

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
OUTPUT_DIR="$SCRIPT_DIR/libraries/OpenXLSX/bin/macarm64"
WORK_DIR="$(mktemp -d)"
REPO_URL="https://codeberg.org/troldal/OpenXLSX.git"

# The revision the headers in libraries/OpenXLSX/include/ were taken from. All
# 32 vendored headers match this commit exactly. Keep it in sync with
# libraries/OpenXLSX/PROVENANCE.md - headers and library must always come from
# one revision, or XLDocument segfaults on the first open.
#
# This used to clone master and overlay the vendored headers onto it. That
# cannot work: master's XLDocument.cpp calls m_archive.addEntryAndCommit(),
# which the vendored headers do not declare, so the build fails outright.
# Checking out the matching revision is both simpler and correct.
OPENXLSX_REV="9d673a34e59e156cc3477a8fdb70d55a91bb8646"

echo "==> Working directory: $WORK_DIR"
echo "==> Output: $OUTPUT_DIR"

cleanup() { rm -rf "$WORK_DIR"; }
trap cleanup EXIT

echo "==> Cloning OpenXLSX at $OPENXLSX_REV..."
git clone --quiet "$REPO_URL" "$WORK_DIR/OpenXLSX"

cd "$WORK_DIR/OpenXLSX"
git checkout --quiet "$OPENXLSX_REV"

echo "==> Configuring CMake (arm64, static, Release)..."
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_ARCHITECTURES=arm64 \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0 \
    -DBUILD_SHARED_LIBS=OFF \
    -DOPENXLSX_BUILD_SHARED_LIBS=OFF \
    -DOPENXLSX_BUILD_SAMPLES=OFF \
    -DOPENXLSX_BUILD_TESTS=OFF \
    -DOPENXLSX_BUILD_BENCHMARKS=OFF \
    -DCMAKE_INSTALL_PREFIX="$WORK_DIR/install"

echo "==> Building..."
cmake --build build --config Release --parallel "$(sysctl -n hw.ncpu)"

echo "==> Installing..."
cmake --install build --config Release

echo "==> Copying static library to $OUTPUT_DIR..."
mkdir -p "$OUTPUT_DIR"

# OpenXLSX installs its static lib under lib/ or lib64/ depending on the version
LIB_PATH=$(find "$WORK_DIR/install" -name "libOpenXLSX.a" | head -1)
if [ -z "$LIB_PATH" ]; then
    echo "ERROR: libOpenXLSX.a not found after install. Contents of install dir:"
    find "$WORK_DIR/install" -type f
    exit 1
fi

cp "$LIB_PATH" "$OUTPUT_DIR/libOpenXLSX.a"
echo ""
echo "==> Done. Verifying:"
file "$OUTPUT_DIR/libOpenXLSX.a"

# LipidSpace.pro already links this path; nothing further to configure. Confirm
# the archive and the vendored headers agree before relying on it - a mismatch
# links cleanly and then segfaults on the first XLSX open.
echo ""
echo "==> Checking the archive against the vendored headers..."
clang++ -std=c++17 -O2 \
    -I "$SCRIPT_DIR/libraries/OpenXLSX/include" \
    "$SCRIPT_DIR/tests/xlsx_smoke.cpp" \
    "$OUTPUT_DIR/libOpenXLSX.a" \
    -o "$WORK_DIR/xlsx_smoke"
"$WORK_DIR/xlsx_smoke" "$SCRIPT_DIR/examples/Example-Dataset.xlsx"
