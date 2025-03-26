#!/bin/bash

GOARCH=wasm tinygo build -o go_demo.wasm -target=wasm -no-debug -panic=trap -scheduler=none -gc=leaking demo.go
wasm-opt -Oz -o go_demo.wasm go_demo.wasm --enable-bulk-memory --enable-nontrapping-float-to-int
xxd -iC go_demo.wasm > go_demo.wasm.h
