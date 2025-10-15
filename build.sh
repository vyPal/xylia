#!/usr/bin/env bash

info() {
  echo -e "\x1b[1;34m[INFO]\x1b[0m $*"
}

warn() {
  echo -e "\x1b[1;33m[WARN]\x1b[0m $*" >&2
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

if command -v nproc &>/dev/null; then
  CORES=$(nproc)
elif command -v sysctl &>/dev/null; then
  CORES=$(sysctl -n hw.ncpu 2>/dev/null || echo 2)
elif [ -f /proc/cpuinfo ]; then
  CORES=$(grep -c '^processor' /proc/cpuinfo)
else
  CORES=2
fi

[[ "$CORES" =~ ^[0-9]+$ ]] && [ "$CORES" -gt 0 ] || CORES=2ORES="8"

BUILD_TYPE="Release"
EXTRA_ARGS=()

if [ $# -ge 1 ]; then
  BUILD_TYPE="$1"
  shift
  EXTRA_ARGS=("$@")
fi

info "Using build type: ${BUILD_TYPE}"

[ ! -d "$BUILD_DIR" ] && mkdir -p "$BUILD_DIR" && info "Created build directory: ${BUILD_DIR}"

info "Configuring CMake for Ninja build..."
cmake -S . -B "${BUILD_DIR}" -G "Ninja" \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  "${EXTRA_ARGS[@]}"

if [ $? -ne 0 ]; then
  error "CMake configuration failed. Aborting."
  exit 1
fi

info "Building with Ninja using $CORES cores"
ninja -C "${BUILD_DIR}" "-j$CORES"
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
fi

info "Exporting compile_commands.json"
if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
  error "Exporting compile_commands.json failed. Aborting."
  exit 1
fi
mv "${BUILD_DIR}/compile_commands.json" .

if [ -f "${OUT_PATH}" ]; then
  [ ! -d "${BIN_DIR}" ] && mkdir -p "${BIN_DIR}" && info "Created bin directory: ${BIN_DIR}"

  mv -f "${OUT_PATH}" "${BIN_DIR}/"
  success "Moved binary to ${BIN_DIR}/$(basename "${OUT_PATH}")"
else
  error "Expected binary '${OUT_PATH}' not found!"
fi

if [ -z "${XYL_HOME}" ]; then
  warn "Environment variable 'XYL_HOME' is not set."
  echo -e "       You should set it to your project root (parent of build dir) for Xylia to work properly:"
  echo -e "       \x1b[1;37mexport XYL_HOME=\"$(realpath "${BUILD_DIR}/..")\"\x1b[0m"
else
  info "XYL_HOME is set to: ${XYL_HOME}"
fi

if [[ ":$PATH:" != *":$(realpath "${BIN_DIR}"):"* ]]; then
  warn "'${BIN_DIR}' is not in your PATH."
  echo -e "       You may want to add it so you can run '${EXE}' from anywhere:"
  echo -e "       \x1b[1;37mexport PATH=\"$(realpath "${BIN_DIR}"):\$PATH\"\x1b[0m"
else
  info "'${BIN_DIR}' is already in PATH"
fi

success "Build complete!"
