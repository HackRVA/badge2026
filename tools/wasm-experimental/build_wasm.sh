#!/bin/bash
# cargo install wasm-opt --locked
# npm install -g assemblyscript 

asc demo.ts --textFile module.wat --outFile demo.wasm --bindings raw -O3 --runtime stub
wasm-opt -Oz -o demo.wasm demo.wasm --enable-bulk-memory --enable-nontrapping-float-to-int  
xxd -iC demo.wasm > demo.wasm.h

