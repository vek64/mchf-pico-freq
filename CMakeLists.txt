cmake_minimum_required(VERSION 3.13...3.27)

# initialize the SDK based on PICO_SDK_PATH
# note: this must happen before project()
include(pico_sdk_import.cmake)

project(my_project C CXX ASM)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()





# rest of your project

add_executable(mchf-freq
        mchf-freq.cpp
        )
# Add the pi-pico-LCD library which aggregates LCD 1602 control
# copy "lcd_display" from gihub to the same level as mchf-freq
# https://github.com/martinkooij/pi-pico-LCD/tree/main/lcd_display
add_subdirectory(lcd_display build)

target_link_libraries(mchf-freq pico_stdlib hardware_adc lcd_display)


    # enable usb output, disable uart output
    pico_enable_stdio_usb(mchf-freq 1)
    pico_enable_stdio_uart(mchf-freq 0)


# create map/bin/hex file etc.
pico_add_extra_outputs(mchf-freq)

