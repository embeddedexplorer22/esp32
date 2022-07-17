#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"

#define CLK_PIN     33
#define MOSI_PIN    32
#define CS_PIN      25

#define DECODE_MODE_REG     0x09
#define INTENSITY_REG       0x0A
#define SCAN_LIMIT_REG      0x0B
#define SHUTDOWN_REG        0x0C
#define DISPLAY_TEST_REG    0x0F

spi_device_handle_t spi2;

static void spi_init() {
    esp_err_t ret;

    spi_bus_config_t buscfg={
        .miso_io_num = -1,
        .mosi_io_num = MOSI_PIN,
        .sclk_io_num = CLK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };

    ret = spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret);

    spi_device_interface_config_t devcfg={
        .clock_speed_hz = 1000000,  // 1 MHz
        .mode = 0,                  //SPI mode 0
        .spics_io_num = CS_PIN,     
        .queue_size = 1,
        .flags = SPI_DEVICE_HALFDUPLEX,
        .pre_cb = NULL,
        .post_cb = NULL,
    };

    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi2));
};

static void write_reg(uint8_t reg, uint8_t value) {
    uint8_t tx_data[2] = { reg, value };

    spi_transaction_t t = {
        .tx_buffer = tx_data,
        .length = 2 * 8
    };

    ESP_ERROR_CHECK(spi_device_polling_transmit(spi2, &t));
}

static void set_row(uint8_t row_index) {
  write_reg(row_index + 1, 0xFF);
}

static void set_col(uint8_t col_index) {
  for (int i = 0; i < 8; i++) {
    write_reg(i + 1, 0x01 << col_index);
  }
}

static void clear(void) {
  for (int i = 0; i < 8; i++) {
    write_reg(i + 1, 0x00);
  }
}

static void max7219_init() {
    write_reg(DISPLAY_TEST_REG, 0);
    write_reg(SCAN_LIMIT_REG, 7);
    write_reg(DECODE_MODE_REG, 0);
    write_reg(SHUTDOWN_REG, 1);
    clear();
}

void app_main(void)
{
    spi_init();
    max7219_init();

    while (1) {
        for (int i = 0; i < 8; i++) {
            clear();
            set_row(i);
            vTaskDelay(1000/portTICK_PERIOD_MS);
        }

        for (int i = 0; i < 8; i++) {
            clear();
            set_col(i);
            vTaskDelay(1000/portTICK_PERIOD_MS);
        }
    }
}
