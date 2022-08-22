add_library(10baset
    10baset.c
)

target_include_directories(10baset PUBLIC
    .
)

pico_generate_pio_header(10baset ${CMAKE_CURRENT_LIST_DIR}/ser_10base_t.pio)

target_link_libraries(
    10baset
    hardware_pio
    hardware_irq
    pico_multicore
    pico_stdlib)