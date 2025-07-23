# CH32V003 WS2812 LED Driver

This project provides a driver for WS2812B addressable LEDs for the WCH CH32V003 MCU. It uses TIM1 and DMA to generate the precise timing required for the WS2812B protocol, allowing for non-blocking control of the LED strip.


## How it Works

The driver utilizes **Timer 1 (TIM1)** of the CH32V003 MCU to generate a PWM signal with a frequency of 800kHz, which is the standard frequency for WS2812B LEDs. The different pulse widths for "0" and "1" bits are achieved by modifying the timer's CCR (Capture/Compare Register) value.

To offload the CPU, **Direct Memory Access (DMA)** is used to transfer the PWM data from a buffer in memory directly to the TIM1 CCR register. This allows the CPU to perform other tasks while the LED data is being sent.

The data pin for the WS2812B strip is connected to a GPIO pin that is configured as an alternate function for the TIM1_CH1 output.



## Configuration

### Number of LEDs and Data Pin

To configure the driver, you need to modify the following macros in the `ws2812b_driver.h` file:

* `WS2812B_NUM_LEDS`: Set this to the number of LEDs in your strip.
* `WS2812B_DATA_PIN`: This defines the GPIO pin connected to the data line of the LED strip. The default is `GPIO_Pin_2`.
* `WS2812B_DATA_PORT`: This defines the GPIO port for the data pin. The default is `GPIOD`.

**Important:** The data pin is tied to the **TIM1\_CH1 output**. On the CH32V003, this can be mapped to several different GPIO pins. Refer to the "Pin Configuration" section below for more details on which pins can be used.

### Timer and DMA Configuration

The timer and DMA settings are pre-configured in `ws2812b_driver.c` for a 48MHz system clock. The key parameters are:

* **TIM1 Period**: Set to 59 (for a 60-count cycle) to achieve an 800kHz PWM frequency (48MHz / 60 = 800kHz).
* **T0H and T1H**: These values define the pulse widths for a '0' and a '1' bit, respectively. They are set to 19 and 38, which correspond to approximately 0.4us and 0.8us pulse widths at an 800kHz frequency.

These values should not need to be changed unless you are using a different system clock frequency.


## How to Use

1.  **Include the driver header:**
    ```c
    #include "ws2812b_driver.h"
    ```

2.  **Initialize the driver:**
    ```c
    ws2812b_init();
    ```
    This function configures the necessary GPIO, TIM1, and DMA peripherals.

3.  **Set the color of individual LEDs:**
    The `leds` array is a global array of `CRGB` structs that holds the color data for each LED. You can modify this array directly or use the provided helper functions.

    ```c
    // Set the color of the first LED to red
    leds[0].r = 255;
    leds[0].g = 0;
    leds[0].b = 0;

    // Or use the helper function
    ws2812b_set_pixel_rgb(0, 255, 0, 0);
    ```

4.  **Send the data to the LEDs:**
    ```c
    ws2812b_show();
    ```
    This function will start the DMA transfer to update the LED strip with the colors stored in the `leds` array. This is a blocking function that will wait for the DMA transfer to complete.

### Example Usage

The `main.c` file provides an example of how to use the driver to create a rainbow effect on the LED strip:

```c
#include "ws2812b_driver.h"
#include "debug.h"

#define BRIGHTNESS 40

int main(void)
{
    uint8_t hue = 0;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    ws2812b_init();

    while(1)
    {
        for(uint16_t i = 0; i < WS2812B_NUM_LEDS; i++)
        {
            CRGB color = hsv_to_rgb(hue + (i * 255 / WS2812B_NUM_LEDS), 255, BRIGHTNESS);
            ws2812b_set_pixel(i, color);
        }
        ws2812b_show();
        hue++;
        Delay_Ms(20);
    }
}
```
## Pin Configuration

The WS2812B data pin must be connected to a GPIO pin that can be used as the TIM1_CH1 output. The CH32V003 MCU offers several pin remapping options for TIM1.

According to the CH32V003 datasheet, the following pins can be configured as **TIM1_CH1**:

| Pin | Default Function | Partial Remap 1 | Partial Remap 2 | Full Remap |
| :--- | :--- | :--- | :--- | :--- |
| **PD2** | TIM1\_CH1 | | | |
| **PC0**| | | | TIM1\_CH1 |

To use a different pin than the default **PD2**, you will need to:

1.  Change `WS2812B_DATA_PIN` and `WS2812B_DATA_PORT` in `ws2812b_driver.h`.
2.  Enable the appropriate pin remapping in the `ws2812b_init` function in `ws2812b_driver.c` using the `GPIO_PinRemapConfig()` function.

For example, to use **PC0** as the data pin, you would make the following changes:

**In `ws2812b_driver.h`:**
```c
#define WS2812B_DATA_PIN    GPIO_Pin_0
#define WS2812B_DATA_PORT   GPIOC
```

**In `ws2812b_driver.c`, inside `ws2812b_init()`:**
```c
GPIO_PinRemapConfig(GPIO_FullRemap_TIM1, ENABLE);
```