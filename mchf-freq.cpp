/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h> // Include for printf
#include <stdlib.h> // Include for dtostrf
#include <string.h> // Include for strlen

// the following libs are from Pico SDK ????
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

// github.com/martinkooij/pi-pico-LCD.git
// it is copied localy.
#include "lcd_display.hpp"


// LCD pin definitions (converted from Arduino pins)
const uint pin_RS = 14;
const uint pin_EN = 15;
const uint pin_d4 = 2;
const uint pin_d5 = 3;
const uint pin_d6 = 4;
const uint pin_d7 = 5;
const uint pin_BL = 10;  // Backlight

// ADC channel for button reading (GPIO26 = ADC0, GPIO27 = ADC1, GPIO28 = ADC2)
const uint BUTTON_ADC_PIN = 26;  // Use ADC0
const uint ADC_CHANNEL = 0;



LCDdisplay myLCD(pin_d4,pin_d5,pin_d6,pin_d7,pin_RS,pin_EN,16,2); // DB4, DB5, DB6, DB7, RS, E, character_width, no_of_lines

/* 
// LCD Commands
#define LCD_CLEAR_DISPLAY 0x01
#define LCD_RETURN_HOME 0x02
#define LCD_ENTRY_MODE_SET 0x04
#define LCD_DISPLAY_CONTROL 0x08
#define LCD_CURSOR_SHIFT 0x10
#define LCD_FUNCTION_SET 0x20
#define LCD_SET_CGRAM_ADDR 0x40
#define LCD_SET_DDRAM_ADDR 0x80

// LCD flags
#define LCD_ENTRY_RIGHT 0x00
#define LCD_ENTRY_LEFT 0x02
#define LCD_ENTRY_SHIFT_INCREMENT 0x01
#define LCD_ENTRY_SHIFT_DECREMENT 0x00

#define LCD_DISPLAY_ON 0x04
#define LCD_DISPLAY_OFF 0x00
#define LCD_CURSOR_ON 0x02
#define LCD_CURSOR_OFF 0x00
#define LCD_BLINK_ON 0x01
#define LCD_BLINK_OFF 0x00

#define LCD_DISPLAY_MOVE 0x08
#define LCD_CURSOR_MOVE 0x00
#define LCD_MOVE_RIGHT 0x04
#define LCD_MOVE_LEFT 0x00

#define LCD_8BIT_MODE 0x10
#define LCD_4BIT_MODE 0x00
#define LCD_2_LINE 0x08
#define LCD_1_LINE 0x00
#define LCD_5x10_DOTS 0x04
#define LCD_5x8_DOTS 0x00

// Function prototypes
void lcd_init(void);
void lcd_write_4bits(uint8_t value);
void lcd_write_cmd(uint8_t cmd);
void lcd_write_data(uint8_t data);
void lcd_clear(void);
void lcd_home(void);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_print(const char* str);
void lcd_print_int(int value);
void init_pins(void);
uint16_t read_buttons(void);
const char* get_button_name(uint16_t adc_value);

void init_pins(void) {
    // Initialize LCD pins
    gpio_init(pin_RS);
    gpio_init(pin_EN);
    gpio_init(pin_d4);
    gpio_init(pin_d5);
    gpio_init(pin_d6);
    gpio_init(pin_d7);
    gpio_init(pin_BL);
    
    gpio_set_dir(pin_RS, GPIO_OUT);
    gpio_set_dir(pin_EN, GPIO_OUT);
    gpio_set_dir(pin_d4, GPIO_OUT);
    gpio_set_dir(pin_d5, GPIO_OUT);
    gpio_set_dir(pin_d6, GPIO_OUT);
    gpio_set_dir(pin_d7, GPIO_OUT);
    gpio_set_dir(pin_BL, GPIO_OUT);
    
    // Turn on backlight
    gpio_put(pin_BL, 1);
    
    // Initialize all pins to low
    gpio_put(pin_RS, 0);
    gpio_put(pin_EN, 0);
    gpio_put(pin_d4, 0);
    gpio_put(pin_d5, 0);
    gpio_put(pin_d6, 0);
    gpio_put(pin_d7, 0);
    
    // Initialize ADC for button reading
    adc_init();
    adc_gpio_init(BUTTON_ADC_PIN);
    adc_select_input(ADC_CHANNEL);
}

void lcd_write_4bits(uint8_t value) {
    gpio_put(pin_d4, (value >> 0) & 0x01);
    gpio_put(pin_d5, (value >> 1) & 0x01);
    gpio_put(pin_d6, (value >> 2) & 0x01);
    gpio_put(pin_d7, (value >> 3) & 0x01);
    
    gpio_put(pin_EN, 1);
    sleep_us(1);    // Enable pulse width
    gpio_put(pin_EN, 0);
    sleep_us(100);  // Commands need > 37us to settle
}

void lcd_write_cmd(uint8_t cmd) {
    gpio_put(pin_RS, 0);  // Command mode
    
    // Send upper 4 bits
    lcd_write_4bits(cmd >> 4);
    // Send lower 4 bits
    lcd_write_4bits(cmd & 0x0F);
    
    if (cmd < 4) {
        sleep_ms(2);  // Commands 1 and 2 need up to 1.64ms
    }
}

void lcd_write_data(uint8_t data) {
    gpio_put(pin_RS, 1);  // Data mode
    
    // Send upper 4 bits
    lcd_write_4bits(data >> 4);
    // Send lower 4 bits
    lcd_write_4bits(data & 0x0F);
}

void lcd_init(void) {
    sleep_ms(50);  // Wait for LCD to power up
    
    // LCD starts in 8-bit mode, try to set 4-bit mode
    gpio_put(pin_RS, 0);
    gpio_put(pin_EN, 0);
    
    // We start in 8bit mode, try to set 4 bit mode
    lcd_write_4bits(0x03);
    sleep_ms(5);
    
    // Second try
    lcd_write_4bits(0x03);
    sleep_ms(5);
    
    // Third go!
    lcd_write_4bits(0x03);
    sleep_us(150);
    
    // Finally, set to 4-bit interface
    lcd_write_4bits(0x02);
    
    // Set # lines, font size, etc.
    lcd_write_cmd(LCD_FUNCTION_SET | LCD_4BIT_MODE | LCD_2_LINE | LCD_5x8_DOTS);
    
    // Turn the display on with no cursor or blinking default
    lcd_write_cmd(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);
    
    // Clear display
    lcd_clear();
    
    // Initialize to default text direction (left to right)
    lcd_write_cmd(LCD_ENTRY_MODE_SET | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_DECREMENT);
}

void lcd_clear(void) {
    lcd_write_cmd(LCD_CLEAR_DISPLAY);
    sleep_ms(2);  // Clear command takes a long time
}

void lcd_home(void) {
    lcd_write_cmd(LCD_RETURN_HOME);
    sleep_ms(2);  // This command takes a long time
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    const uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
    if (row >= 4) row = 0;  // We only have 4 rows max
    if (col >= 20) col = 0; // We only have 20 columns max
    
    lcd_write_cmd(LCD_SET_DDRAM_ADDR | (col + row_offsets[row]));
}

void lcd_print(const char* str) {
    while (*str) {
        lcd_write_data(*str++);
    }
}

void lcd_print_int(int value) {
    char buffer[16];
    sprintf(buffer, "%d", value);
    lcd_print(buffer);
}
 */


uint16_t read_buttons(void) {
    // Read ADC value (12-bit, 0-4095)
    uint16_t adc_value = adc_read();
    
    // Convert to similar range as Arduino (0-1023)
    return adc_value >> 2;  // Divide by 4 to get 0-1023 range
}

const char* get_button_name(uint16_t adc_value) {
    if (adc_value < 60) {
        return "Right ";
    }
    else if (adc_value < 0.5) {
        return "Up    ";
    }
    else if (adc_value < 1.8) {
        return "Down  ";
    }
    else if (adc_value < 2.5) {
        return "Left  ";
    }
    else if (adc_value < 3.0) {
        return "Select";
    }
    else {
        return "None  ";
    }
}



int main() {
    stdio_init_all();

      // Initialize hardware
    //init_pins();
    sleep_ms(100);
    
    // Initialize LCD
	myLCD.init();
    //lcd_init();
    
/*     myLCD.clear();
		myLCD.cursor_on();
		myLCD.print("HELLO   ");
		sleep_ms(2500);
		myLCD.clear() ;
		myLCD.cursor_off();
		myLCD.print_wrapped("0123456789012345BYE");
 */

    // Display initial messages
    myLCD.clear();
	myLCD.cursor_on();
    myLCD.print("BlueCatSystems.com   ");

/*     
    lcd_set_cursor(0, 0);
    lcd_print("BlueCatSystems.com");
    lcd_set_cursor(0, 1);
    lcd_print("Press Key:");
     */

    printf("Raspberry Pi Pico LCD Keypad Demo Started\n");
    
    printf("ADC BUTTON, measuring GPIO26\n");

    adc_init();

    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(BUTTON_ADC_PIN);
    // Select ADC input 0 (GPIO26)
    adc_select_input(0);

    while (1) {
        // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
        const float conversion_factor = 3.3f / (1 << 12);
        uint16_t adc_value = adc_read();
        float adc_voltage = adc_value * conversion_factor;
        printf("Raw value: 0x%03x, voltage: %f V\n", adc_value, adc_voltage);
        sleep_ms(500);

        char str_voltage[5];
        //str_voltage = "3.33"
        // Use dtostrf to convert the float to a string
        //dtostrf(adc_voltage, 5, 3, str_voltage);

        
        // Display ADC value
        //lcd_set_cursor(10, 0);
        myLCD.cursor_on();
        
        // Clear the area first by printing spaces
        myLCD.print("      ");
        //lcd_set_cursor(10, 0);
        
        
        
        myLCD.print(str_voltage);
        
        // Display button name
        myLCD.goto_pos(10, 1);
        myLCD.print(get_button_name(adc_voltage));
        
        // Print to console for debugging
        printf("ADC: %d, Button: %s\n", adc_voltage, get_button_name(adc_voltage));
        
        sleep_ms(100);  // Small delay to prevent excessive updates
    }
}
