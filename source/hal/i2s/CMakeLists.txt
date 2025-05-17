# build as a library

add_library(rp2040_i2s_example rp2040_i2s_example/i2s.c)

pico_generate_pio_header(rp2040_i2s_example ${CMAKE_CURRENT_LIST_DIR}/rp2040_i2s_example/i2s.pio)
pico_generate_pio_header(rp2040_i2s_example ${CMAKE_CURRENT_LIST_DIR}/i2s_master_full.pio)

target_link_libraries(rp2040_i2s_example PRIVATE
    pico_stdlib
    hardware_clocks
    hardware_dma
    hardware_irq
    hardware_pio
    )

target_include_directories(rp2040_i2s_example PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/rp2040_i2s_example
    )

