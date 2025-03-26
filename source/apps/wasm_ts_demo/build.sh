#!/bin/bash
# cargo install wasm-opt --locked
# npm install -g assemblyscript 

set -e

OUT_DIR="out"
OUT_BASENAME="as_demo"
WASM_FILE="$OUT_DIR/$OUT_BASENAME.wasm"
HEADER_FILE="$OUT_DIR/$OUT_BASENAME.wasm.h"
TEXT_FILE="$OUT_DIR/$OUT_BASENAME.wat"
SRC_FILE="demo.ts"

mkdir -p "$OUT_DIR"

echo "Building $WASM_FILE from $SRC_FILE..."
asc "$SRC_FILE" --textFile "$TEXT_FILE" --outFile "$WASM_FILE" --bindings raw -O3 --runtime stub

echo "Optimizing $WASM_FILE..."
wasm-opt -Oz -o "$WASM_FILE" "$WASM_FILE" --enable-bulk-memory --enable-nontrapping-float-to-int

echo "Generating header $HEADER_FILE..."
xxd -iC "$WASM_FILE" > "$HEADER_FILE"

echo "Done. Output written to $OUT_DIR"
