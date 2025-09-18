#!/usr/bin/env bash

info() {
  echo -e "\x1b[1;34m[INFO]\x1b[0m $*"
}

error() {
  echo -e "\x1b[1;31m[ERROR]\x1b[0m $*" >&2
}

success() {
  echo -e "\x1b[1;32m[SUCCESS]\x1b[0m $*"
}

BUILD_DIR="build"
BIN_DIR="bin"
EXE="xylia"
OUT_PATH="${BUILD_DIR}/${EXE}"
BIN_PATH="${BIN_DIR}/${EXE}"

BUILD_TYPE="Release"
if [ $# -ge 1 ]; then
    BUILD_TYPE="$1"
fi

info "Using build type: ${BUILD_TYPE}"

[ ! -d "$BUILD_DIR" ] && mkdir -p "$BUILD_DIR" && info "Created build directory: ${BUILD_DIR}"
[ ! -d "$BIN_DIR" ] && mkdir -p "$BIN_DIR" && info "Created bin directory: ${BIN_DIR}"

info "Configuring CMake for Ninja build..."
cmake -S . -B "${BUILD_DIR}" -G "Ninja" \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

if [ $? -ne 0 ]; then
  error "CMake configuration failed. Aborting."
  exit 1
fi

info "Building with Ninja"
ninja -C "${BUILD_DIR}"
if [ $? -ne 0 ]; then
  error "Ninja build failed. Aborting."
  exit 1
fi

if [ -f "${OUT_PATH}" ]; then
  if [ "$BUILD_TYPE" != "Debug" ]; then
    before_size=$(stat -c%s "${OUT_PATH}")
    info "Stripping binary ${OUT_PATH}..."
    strip --strip-all "${OUT_PATH}"
    after_size=$(stat -c%s "${OUT_PATH}")
    
    saved=$((before_size - after_size))
    saved_kb=$((saved / 1024))
    after_kb=$((after_size / 1024))
    
    info "Stripped $(printf "%.2f" "$saved_kb") KB, final size: $(printf "%.2f" "$after_kb") KB"
  fi

  info "Moving binary to ${BIN_PATH}..."
  cp "${OUT_PATH}" "${BIN_PATH}"
fi

info "Exporting compile_commands.json"
if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
  error "Exporting compile_commands.json failed. Aborting."
  exit 1
fi
mv "${BUILD_DIR}/compile_commands.json" .

success "Build complete!"
