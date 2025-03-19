# Download the ST7735sDriver compile it
#  The source for driver-ST7735S is added to ./build/_deps/st7735sdriver-src/
#  We can then compile it and not have to deal with git submodules

include(FetchContent)

FetchContent_Declare(
  ST7735SDriver
  GIT_REPOSITORY https://github.com/HackRVA/driver-ST7735S
  GIT_TAG main
)

FetchContent_MakeAvailable(
  ST7735SDriver
)

include_directories(${st7735sdriver_SOURCE_DIR}/source)

function(add_st7735sdriver_sources target)
    file(GLOB ST7735SDriver_SOURCES ${st7735sdriver_SOURCE_DIR}/source/st7735s.c)
    target_sources(${target} PRIVATE ${ST7735SDriver_SOURCES})
endfunction()

