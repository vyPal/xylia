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

if [ ! -d "$BUILD_DIR" ]; then
  info "Creating build directory: ${BUILD_DIR}"
  mkdir -p "$BUILD_DIR"
fi

if [ ! -d "$BIN_DIR" ]; then
  info "Creating bin directory: ${BIN_DIR}"
  mkdir -p "$BIN_DIR"
fi

info "Configuring CMake for Ninja build..."
cmake -S . -B "${BUILD_DIR}" -G "Ninja" \
  -DCMAKE_BUILD_TYPE=Release \
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

if [ -f "${BUILD_DIR}/${EXE}" ]; then
  before_size=$(stat -c%s "${BUILD_DIR}/${EXE}")
  info "Stripping binary ${BUILD_DIR}/${EXE}..."
  strip --strip-all "${BUILD_DIR}/${EXE}"
  after_size=$(stat -c%s "${BUILD_DIR}/${EXE}")

  saved=$((before_size - after_size))
  saved_kb=$((saved / 1024))
  after_kb=$((after_size / 1024))

  info "Stripped $(printf "%.2f" "$saved_kb") KB, final size: $(printf "%.2f" "$after_kb") KB"

  info "Moving binary to ${BIN_DIR}/${EXE}..."
  mv "${BUILD_DIR}/${EXE}" "${BIN_DIR}/${EXE}"
fi

info "Exporting compile_commands.json"

if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
  error "Exporting compile_commands.json failed. Aborting."
  exit 1
fi

cp "${BUILD_DIR}/compile_commands.json" .

success "Build complete!"
