#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} color_t;

#define LED_RED_PIN         33
#define LED_GREEN_PIN       32
#define LED_BLUE_PIN        25

ledc_timer_config_t ledc_timer = {
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .timer_num  = LEDC_TIMER_0,
    .duty_resolution = LEDC_TIMER_13_BIT,
    .freq_hz = 1000,
    .clk_cfg = LEDC_AUTO_CLK
};

ledc_channel_config_t ledc_channel[3];

static void init(void)
{
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel[0].channel = LEDC_CHANNEL_0;
    ledc_channel[0].gpio_num = LED_RED_PIN;

    ledc_channel[1].channel = LEDC_CHANNEL_1;
    ledc_channel[1].gpio_num = LED_GREEN_PIN;
    
    ledc_channel[2].channel = LEDC_CHANNEL_2;
    ledc_channel[2].gpio_num = LED_BLUE_PIN;

    for (int i = 0; i < 3; i++)
    {   
        ledc_channel[i].speed_mode = LEDC_LOW_SPEED_MODE;
        ledc_channel[i].timer_sel = LEDC_TIMER_0;
        ledc_channel[i].intr_type = LEDC_INTR_DISABLE;
        ledc_channel[i].duty = 0;
        ledc_channel[i].hpoint = 0;
        
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel[i]));
    }
}

static void setColor(color_t color)
{
    uint32_t red_duty = 8191 * color.r / 255;
    uint32_t green_duty = 8191 * color.g / 255;
    uint32_t blue_duty = 8191 * color.b / 255; 

    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, red_duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));

    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, green_duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1));

    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, blue_duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2));
}

void app_main(void)
{
    const color_t red = { .r = 255, .g = 0, .b = 0 };
    const color_t green = { .r = 0, .g = 255, .b = 0 };
    const color_t blue = { .r = 0, .g = 0, .b = 255 };
    const color_t yellow = { .r = 255, .g = 255, .b = 0 };
    const color_t purple = { .r = 128, .g = 0, .b = 128 };
    const color_t silver = { .r = 192, .g = 192, .b = 192 };

    color_t sample_colors[6] = { red, green, blue, yellow, purple, silver };

    init();

    while (1) {
        for (int i = 0; i < 6; i++)
        {
            printf("Color %d\r\n", i);
            setColor(sample_colors[i]);
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }
}
