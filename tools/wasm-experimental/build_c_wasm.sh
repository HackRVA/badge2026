
clang --target=wasm32 -nostdlib -Wl,--no-entry -Wl,--export-all \
  -Wl,--allow-undefined -O2 -o c_demo.wasm demo.c

# cargo install wasm-opt --locked
wasm-opt -Oz -o c_demo.wasm c_demo.wasm --enable-bulk-memory --enable-nontrapping-float-to-int
xxd -iC c_demo.wasm > c_demo.wasm.h
