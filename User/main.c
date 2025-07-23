#include "debug.h"
#include "ws2812b_driver.h" // Our WS2812B library

/*=============================================================================
 * Defines
 *===========================================================================*/
#define BRIGHTNESS 40   // Max brightness for the LEDs (0-255)
#define ADC_MAP_MIN            350     // The minimum ADC value for mapping
#define ADC_MAP_MAX            700     // The maximum ADC value for mapping

/*=============================================================================
 * Main Function
 *===========================================================================*/
int main(void)
{
    uint8_t hue = 0;
    // System initializations
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    // Initialize peripherals
    ws2812b_init(); // WS2812B LEDs (uses TIM1 on PD2)

    // Main loop
    while(1)
    {
        for(uint16_t i = 0; i < WS2812B_NUM_LEDS; i++)
            {
                CRGB color = hsv_to_rgb(hue + (i * 255 / WS2812B_NUM_LEDS), 255, BRIGHTNESS);
                ws2812b_set_pixel(i, color);
            }
        ws2812b_show(); // Send the data to the LEDs
        hue++;
        Delay_Ms(20);
    }
}