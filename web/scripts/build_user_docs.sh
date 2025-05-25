#!/bin/bash

mkdir -p .dist/pages/simulator

# check for wasm and js file in build_wasm dest-dir
BUILD_DIR="../build_wasm/source"
DST_DIR=".dist/pages/simulator"
FILES=("badge2025_c.js" "badge2025_c.wasm" "badge2025_c.data")

for file in "${FILES[@]}"; do
	if [ ! -f "$BUILD_DIR/$file" ]; then
		echo "Missing file: $BUILD_DIR/$file"
		echo "You might need to run: bash run_cmake_wasm.sh"
		exit 1
	fi
done

for file in "${FILES[@]}"; do
	cp "$BUILD_DIR/$file" "$DST_DIR/"
done

