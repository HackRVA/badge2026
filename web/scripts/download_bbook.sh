#!/bin/bash

set -e

mkdir -p .bin

EXPECTED_HASH="c5327470733b2c9cc9d124fcb9a1edfaf3126e2df899a70737517008d4581210"

ARCHIVE=".bin/bbook.tar.gz"
BIN=".bin/bbook"

if [ ! -f "$BIN" ]; then
    echo "Downloading bbook..."

    curl -sSL -o "$ARCHIVE" https://github.com/dfirebaugh/bbook/releases/download/v0.0.0/bbook-x86_64_unknown-linux.tar.gz

    echo "Verifying checksum..."
    ACTUAL_HASH=$(sha256sum "$ARCHIVE" | awk '{print $1}')

    if [ "$ACTUAL_HASH" != "$EXPECTED_HASH" ]; then
        echo "Checksum mismatch!"
        echo "Expected: $EXPECTED_HASH"
        echo "Actual:   $ACTUAL_HASH"
        exit 1
    fi

    echo "checksum OK, extracting..."
    tar -xz -C .bin -f "$ARCHIVE"
    rm "$ARCHIVE"
else
    echo "$BIN already exists"
fi
