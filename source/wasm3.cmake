include(FetchContent)

FetchContent_Declare(
  wasm3
  GIT_REPOSITORY https://github.com/wasm3/wasm3
  GIT_TAG main
)

FetchContent_Populate(wasm3)

file(GLOB WASM3_SRC "${wasm3_SOURCE_DIR}/source/*.c")
target_sources(${PRODUCT} PUBLIC ${WASM3_SRC})
target_include_directories(${PRODUCT} PRIVATE "${wasm3_SOURCE_DIR}/source")
