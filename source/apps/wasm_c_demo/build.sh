#!/bin/bash

set -e

# cargo install wasm-opt --locked
OUT_DIR="out"
OUT_BASENAME="cdemo"
WASM_FILE="$OUT_DIR/$OUT_BASENAME.wasm"
HEADER_FILE="$OUT_DIR/$OUT_BASENAME.wasm.h"
SRC_FILE="demo.c"

mkdir -p "$OUT_DIR"

echo "Building $WASM_FILE from $SRC_FILE..."
clang --target=wasm32 -nostdlib -Wl,--no-entry -Wl,--export-all \
  -Wl,--allow-undefined -O2 -o "$WASM_FILE" "$SRC_FILE"

echo "Optimizing $WASM_FILE..."
wasm-opt -Oz -o "$WASM_FILE" "$WASM_FILE" --enable-bulk-memory --enable-nontrapping-float-to-int

echo "Generating header $HEADER_FILE..."
xxd -iC "$WASM_FILE" > "$HEADER_FILE"

echo "Done. Output written to $OUT_DIR"
