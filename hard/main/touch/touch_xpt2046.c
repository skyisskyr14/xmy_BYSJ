#include "touch_xpt2046.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include <string.h>
#include <stdio.h>

#define LCD_W 240
#define LCD_H 320

static spi_device_handle_t tp_spi;
static int tp_irq;

void touch_init(spi_host_device_t host, int cs, int irq)
{
    tp_irq = irq;

    gpio_config_t io_conf = {
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = 1ULL << irq,
        .pull_up_en = 1};
    gpio_config(&io_conf);

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = cs,
        .queue_size = 1,
    };

    ESP_ERROR_CHECK(spi_bus_add_device(host, &devcfg, &tp_spi));
}

static uint16_t tp_read(uint8_t cmd)
{
    spi_transaction_t t;
    uint8_t tx[3] = {cmd, 0, 0};
    uint8_t rx[3] = {0};

    memset(&t, 0, sizeof(t));
    t.length = 24;
    t.tx_buffer = tx;
    t.rx_buffer = rx;

    ESP_ERROR_CHECK(spi_device_transmit(tp_spi, &t));

    return (((uint16_t)rx[1] << 8) | rx[2]) >> 3;
}

bool touch_read(uint16_t *x, uint16_t *y)
{
    if (gpio_get_level(tp_irq) == 1)
    {
        return false;
    }

    uint32_t sum_x = 0;
    uint32_t sum_y = 0;
    int valid = 0;

    for (int i = 0; i < 8; i++)
    {
        uint16_t raw_x = tp_read(0x90);
        uint16_t raw_y = tp_read(0xD0);

        if (raw_x == 0 || raw_y == 0)
            continue;
        if (raw_x >= 4095 || raw_y >= 4095)
            continue;

        sum_x += raw_x;
        sum_y += raw_y;
        valid++;
    }

    if (valid < 3)
    {
        return false;
    }

    uint16_t raw_x = sum_x / valid;
    uint16_t raw_y = sum_y / valid;

    printf("raw avg: %u %u\n", raw_x, raw_y);

#define TP_X_MIN 400
#define TP_X_MAX 3760
#define TP_Y_MIN 280
#define TP_Y_MAX 3660

    int px = (raw_y - TP_Y_MIN) * LCD_W / (TP_Y_MAX - TP_Y_MIN);
    int py = (TP_X_MAX - raw_x) * LCD_H / (TP_X_MAX - TP_X_MIN);

    if (px < 0)
        px = 0;
    if (py < 0)
        py = 0;
    if (px >= LCD_W)
        px = LCD_W - 1;
    if (py >= LCD_H)
        py = LCD_H - 1;

    *x = px;
    *y = py;

    return true;
}