#ifndef WS2812B_DRIVER_H
#define WS2812B_DRIVER_H

#include "ch32v00x.h"
#include <stdint.h>
#include <stdbool.h>

// ===================================================================================
// CONFIGURATION
// ===================================================================================
#define WS2812B_NUM_LEDS    24      // <<< CHANGE THE NUMBER OF YOUR LEDS HERE
#define WS2812B_DATA_PIN    GPIO_Pin_2 // <<< The pin where the LEDs are connected (TIM1_CH1)
#define WS2812B_DATA_PORT   GPIOD      // <<< The port of the pin

// ===================================================================================
// DATA TYPES
// ===================================================================================

// Struct for storing a single RGB color
typedef struct {
    uint8_t g; // Green
    uint8_t r; // Red
    uint8_t b; // Blue
} CRGB;

// ===================================================================================
// GLOBAL LED ARRAY
// ===================================================================================
extern CRGB leds[WS2812B_NUM_LEDS];

// ===================================================================================
// FUNCTION PROTOTYPES
// ===================================================================================

/**
 * @brief Initializes the Timer, DMA, and GPIO for WS2812B control.
 */
void ws2812b_init(void);

/**
 * @brief Sends the color data stored in the 'leds' array to the LED strip.
 * @note This is a blocking function that uses a large RAM buffer.
 */
void ws2812b_show(void);

/**
 * @brief Sets the color of a single LED in the 'leds' array.
 * @param pixel_num The index of the LED (starting from 0).
 * @param red The red value (0-255).
 * @param green The green value (0-255).
 * @param blue The blue value (0-255).
 */
void ws2812b_set_pixel_rgb(uint16_t pixel_num, uint8_t red, uint8_t green, uint8_t blue);

/**
 * @brief Sets the color of a single LED in the 'leds' array using a CRGB struct.
 * @param pixel_num The index of the LED (starting from 0).
 * @param color The CRGB color.
 */
void ws2812b_set_pixel(uint16_t pixel_num, CRGB color);

/**
 * @brief Fills the entire LED strip with a single color.
 * @param color The CRGB color to fill the strip with.
 */
void ws2812b_fill_solid(CRGB color);

/**
 * @brief Converts a color from HSV (Hue, Saturation, Value) color space to RGB.
 * @param h Hue (0-255).
 * @param s Saturation (0-255).
 * @param v Value/Brightness (0-255).
 * @return A CRGB struct with the corresponding RGB values.
 */
CRGB hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v);


#endif // WS2812B_DRIVER_H
