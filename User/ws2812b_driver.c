#include "ws2812b_driver.h"

// --- Timing and Buffer Configuration ---
#define T0H             19  // Pulse width for a '0' (approx. 0.4us)
#define T1H             38  // Pulse width for a '1' (approx. 0.8us)
#define BITS_PER_LED    24
#define RESET_PULSE_LEN 50

// This buffer holds the PWM data for all LEDs at once.
static uint16_t dma_buffer[WS2812B_NUM_LEDS * BITS_PER_LED + RESET_PULSE_LEN];

CRGB leds[WS2812B_NUM_LEDS];

void ws2812b_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure={0};
    TIM_OCInitTypeDef TIM_OCInitStructure={0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure={0};
    DMA_InitTypeDef DMA_InitStructure={0};

    // Enable clocks
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_TIM1 | RCC_APB2Periph_AFIO, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    // Configure GPIO Pin
    GPIO_InitStructure.GPIO_Pin = WS2812B_DATA_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(WS2812B_DATA_PORT, &GPIO_InitStructure);

    // Configure Timer
    TIM_TimeBaseInitStructure.TIM_Period = 60 - 1; // 48MHz / 60 = 800kHz
    TIM_TimeBaseInitStructure.TIM_Prescaler = 0;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

    // Configure PWM Output
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);

    // Configure DMA (DMA1 Channel 5 is triggered by TIM1_UP)
    DMA_DeInit(DMA1_Channel5);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&TIM1->CH1CVR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)dma_buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = sizeof(dma_buffer) / sizeof(dma_buffer[0]);
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; // Normal mode, not circular
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);

    TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE);
}

void ws2812b_show(void)
{
    uint32_t buffer_pos = 0;
    uint32_t i, j;

    for (i = 0; i < WS2812B_NUM_LEDS; i++)
    {
        uint32_t color_packed = (leds[i].g << 16) | (leds[i].r << 8) | leds[i].b;
        for (j = 0; j < BITS_PER_LED; j++)
        {
            if ((color_packed << j) & 0x800000)
            {
                dma_buffer[buffer_pos++] = T1H;
            }
            else
            {
                dma_buffer[buffer_pos++] = T0H;
            }
        }
    }

    for (i = 0; i < RESET_PULSE_LEN; i++)
    {
        dma_buffer[buffer_pos++] = 0;
    }

    // Start DMA transfer
    DMA_Cmd(DMA1_Channel5, DISABLE);
    DMA_SetCurrDataCounter(DMA1_Channel5, sizeof(dma_buffer) / sizeof(dma_buffer[0]));
    DMA_Cmd(DMA1_Channel5, ENABLE);
    TIM_Cmd(TIM1, ENABLE);

    // Wait until the DMA transfer is complete
    while(!DMA_GetFlagStatus(DMA1_FLAG_TC5));
    DMA_ClearFlag(DMA1_FLAG_TC5);

    // Stop timer
    TIM_Cmd(TIM1, DISABLE);
}

// --- Standard API functions
void ws2812b_set_pixel_rgb(uint16_t pixel_num, uint8_t red, uint8_t green, uint8_t blue)
{
    if (pixel_num < WS2812B_NUM_LEDS)
    {
        leds[pixel_num].r = red;
        leds[pixel_num].g = green;
        leds[pixel_num].b = blue;
    }
}

void ws2812b_set_pixel(uint16_t pixel_num, CRGB color)
{
    if (pixel_num < WS2812B_NUM_LEDS)
    {
        leds[pixel_num] = color;
    }
}

void ws2812b_fill_solid(CRGB color)
{
    for (uint16_t i = 0; i < WS2812B_NUM_LEDS; i++)
    {
        leds[i] = color;
    }
}

CRGB hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v)
{
    CRGB rgb;
    uint8_t region, remainder, p, q, t;

    if (s == 0)
    {
        rgb.r = v;
        rgb.g = v;
        rgb.b = v;
        return rgb;
    }

    region = h / 43;
    remainder = (h - (region * 43)) * 6;

    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0: rgb.r = v; rgb.g = t; rgb.b = p; break;
        case 1: rgb.r = q; rgb.g = v; rgb.b = p; break;
        case 2: rgb.r = p; rgb.g = v; rgb.b = t; break;
        case 3: rgb.r = p; rgb.g = q; rgb.b = v; break;
        case 4: rgb.r = t; rgb.g = p; rgb.b = v; break;
        default: rgb.r = v; rgb.g = p; rgb.b = q; break;
    }
    return rgb;
}
