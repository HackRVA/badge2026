#!/bin/bash

set -e

OUT_DIR="out"
OUT_BASENAME="gol"
WASM_FILE="$OUT_DIR/$OUT_BASENAME.wasm"
HEADER_FILE="$OUT_DIR/$OUT_BASENAME.wasm.h"

mkdir -p "$OUT_DIR"

echo "Building $WASM_FILE from current package..."
GOARCH=wasm tinygo build -o "$WASM_FILE" -target=wasm -no-debug -panic=trap -scheduler=none -gc=leaking .

echo "Optimizing $WASM_FILE..."
wasm-opt -Oz -o "$WASM_FILE" "$WASM_FILE" --enable-bulk-memory --enable-nontrapping-float-to-int

echo "Generating header $HEADER_FILE..."
xxd -iC "$WASM_FILE" > "$HEADER_FILE"

echo "Done. Output written to $OUT_DIR"

