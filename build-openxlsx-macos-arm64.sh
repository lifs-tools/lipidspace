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
REPO_URL="https://github.com/troldal/OpenXLSX.git"

# The project carries post-v0.3.2 headers (XLSheet.hpp, XLColor.hpp, XLDateTime.hpp
# differ from v0.3.2). We clone master and overlay the project's own headers so
# the compiled library is ABI-compatible with what LipidSpace includes.
OPENXLSX_TAG="master"

echo "==> Working directory: $WORK_DIR"
echo "==> Output: $OUTPUT_DIR"

cleanup() { rm -rf "$WORK_DIR"; }
trap cleanup EXIT

echo "==> Cloning OpenXLSX $OPENXLSX_TAG..."
git clone --quiet --depth 1 --branch "$OPENXLSX_TAG" "$REPO_URL" "$WORK_DIR/OpenXLSX"

cd "$WORK_DIR/OpenXLSX"

# Overlay the project's own headers so the compiled library matches exactly what
# LipidSpace includes. This ensures ABI compatibility on the macOS build.
echo "==> Overlaying project headers onto cloned source..."
cp "$SCRIPT_DIR/libraries/OpenXLSX/include/headers/"*.hpp \
   "$WORK_DIR/OpenXLSX/OpenXLSX/headers/"
cp "$SCRIPT_DIR/libraries/OpenXLSX/include/OpenXLSX.hpp" \
   "$WORK_DIR/OpenXLSX/OpenXLSX/"
cp "$SCRIPT_DIR/libraries/OpenXLSX/include/OpenXLSX-Exports.hpp" \
   "$WORK_DIR/OpenXLSX/OpenXLSX/"

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
echo ""
echo "Add to your PATH for qmake:"
echo "  LIBS += -L\$\$PWD/libraries/OpenXLSX/bin/macarm64 -lOpenXLSX"
