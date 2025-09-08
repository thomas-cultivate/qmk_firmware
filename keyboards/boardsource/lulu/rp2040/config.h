#pragma once
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 200U
// Use the same pins as you had for split, I2C, etc. ...
// (unchanged from your snippet)
#define RGB_DI_PIN GP29          // The WS2812 data pin
#define DRIVER_LED_TOTAL 70      // Or however many LEDs you actually have
#define RGB_MATRIX_MAXIMUM_BRIGHTNESS 200  // Cap the brightness
// If you want typical animations, define them:
#define ENABLE_RGB_MATRIX_SOLID_COLOR
#define ENABLE_RGB_MATRIX_RAINBOW_MOVING_CHEVRON
#define ENABLE_RGB_MATRIX_RAINBOW_BEACON
// etc. (see QMK docs for the full list of effects)
