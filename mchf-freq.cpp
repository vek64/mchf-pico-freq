/**
 * Copyright (c) 2025 BlueCatSystems.
 *
 */

#include <stdio.h>
#include <string.h>

// Pico SDK libraries
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

// Local LCD library
// github.com/martinkooij/pi-pico-LCD.git
#include "lcd_display.hpp"

// LCD pin definitions
const uint pin_RS = 14;
const uint pin_EN = 15;
const uint pin_d4 = 2;
const uint pin_d5 = 3;
const uint pin_d6 = 4;
const uint pin_d7 = 5;
const uint pin_BL = 10;  // Backlight

// ADC configuration
const uint BUTTON_ADC_PIN = 26;  // GPIO26 = ADC0
const uint ADC_CHANNEL = 0;

// ADC conversion factor: 3.3V / 4096 (12-bit ADC) -- NNED deviced to get 5V from LCD module to 3.3V
const float ADC_CONVERSION_FACTOR = 3.3f / (1 << 12);

// Button voltage thresholds (adjusted for actual voltage values)
const int ADC_RIGHT = 50;
const int ADC_UP = 250;
const int ADC_DOWN = 450;
const int ADC_LEFT = 600;
const int ADC_SELECT = 920;

// Create LCD object
LCDdisplay myLCD(pin_d4, pin_d5, pin_d6, pin_d7, pin_RS, pin_EN, 16, 2);

/**
 * Read ADC value and return as 10-bit equivalent (0-1023)
 */
/* uint16_t read_buttons(void) {
    uint16_t adc_value = adc_read(); // Read 12-bit ADC value 
    return adc_value >> 2;  // Convert 12-bit to 10-bit range
} */

/**
 * Determine button pressed based on voltage
 */
const char* get_button_name(uint16_t adc_) {

    adc_ >> 2;  // Convert 12-bit to 10-bit range (10-bit equivalent (0-1023))

    if (adc_ < ADC_RIGHT) {
        return "Right ";
    }
    else if (adc_ < ADC_UP) {
        return "Up    ";
    }
    else if (adc_ < ADC_DOWN) {
        return "Down  ";
    }
    else if (adc_ < ADC_LEFT) {
        return "Left  ";
    }
    else if (adc_ < ADC_SELECT) {
        return "Select";
    }
    else {
        return "None  ";
    }
}

/**
 * Convert float voltage to string with 2 decimal places
 */
void convert_float_str(float voltage, char* str_buffer, size_t buffer_size) {
    snprintf(str_buffer, buffer_size, "%.2f", voltage);
}

/**
 * Initialize hardware components
 */
void init_hardware(void) {
    // Initialize stdio for USB serial output
    stdio_init_all();
    
    // Small delay for system stability
    sleep_ms(100);
    
    // Initialize LCD
    myLCD.init();
    myLCD.clear();
    myLCD.cursor_off();  // Turn off cursor for cleaner display
    
    // Display startup message
    myLCD.print("BlueCatSystems");
    myLCD.goto_pos(0, 1);
    myLCD.print("ADC Monitor");
    
    sleep_ms(2000);  // Show startup message for 2 seconds
    
    // Initialize ADC
    adc_init();
    adc_gpio_init(BUTTON_ADC_PIN);
    adc_select_input(ADC_CHANNEL);
    
    printf("System initialized. Starting ADC monitoring...\n");
}

/**
 * Update LCD display with current readings
 */
void update_display(float voltage, const char* button_name) {
    char voltage_str[8];  // Increased buffer size for safety
    
    convert_float_str(voltage, voltage_str, sizeof(voltage_str));
    
    // Clear and update first line
    myLCD.goto_pos(0, 0);
    myLCD.print("V:");
    myLCD.print(voltage_str);
    myLCD.print("V    ");  // Clear any remaining characters
    
    // Update second line with button status
    myLCD.goto_pos(0, 1);
    myLCD.print("Btn:");
    myLCD.print(button_name);
}

int main() {
    // Initialize all hardware
    init_hardware();
    
    uint16_t adc_raw;
    float adc_voltage;
    const char* current_button;
    
    // Clear LCD and show ready status
    myLCD.clear();
    
    while (true) {
        // Read ADC value
        adc_raw = adc_read();
        adc_voltage = adc_raw * ADC_CONVERSION_FACTOR;
        current_button = get_button_name(adc_raw);
        
        // Update display
        update_display(adc_voltage, current_button);
        
        // Debug output to console
        printf("Raw: 0x%03x (%d), Voltage: %.3fV, Button: %s\n", 
               adc_raw, adc_raw, adc_voltage, current_button);
        
        // Delay to prevent excessive updates
        sleep_ms(250);
    }
    
    return 0;
}