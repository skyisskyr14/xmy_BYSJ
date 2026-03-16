#include "ili9341_simple.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_lcd_ili9341.h"

static const char *TAG = "ili9341_simple";

/**
 * @brief 裁剪到屏幕范围内（内部使用）
 */
static void clip_rect(const ili9341_simple_t *lcd, int *x, int *y, int *w, int *h)
{
    if (*w <= 0 || *h <= 0) { *w = 0; *h = 0; return; }

    if (*x < 0) { *w += *x; *x = 0; }
    if (*y < 0) { *h += *y; *y = 0; }

    if (*x + *w > lcd->width)  *w = lcd->width  - *x;
    if (*y + *h > lcd->height) *h = lcd->height - *y;

    if (*w < 0) *w = 0;
    if (*h < 0) *h = 0;
}

esp_err_t ili9341_simple_init(ili9341_simple_t *lcd,
                             spi_host_device_t host,
                             int width, int height,
                             int pin_sclk, int pin_mosi, int pin_miso,
                             int pin_cs, int pin_dc,
                             int pin_rst, int pin_bl,
                             int pclk_hz,
                             int rgb_ele_order)
{
    if (!lcd) return ESP_ERR_INVALID_ARG;
    memset(lcd, 0, sizeof(*lcd));

    lcd->width  = width;
    lcd->height = height;

    // ===== 背光（如果有）=====
    if (pin_bl >= 0) {
        gpio_config_t bk = {
            .pin_bit_mask = 1ULL << pin_bl,
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = 0,
            .pull_down_en = 0,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&bk);
        gpio_set_level(pin_bl, 1);
    }

    // ===== SPI bus 初始化 =====
    spi_bus_config_t buscfg = {
        .sclk_io_num = pin_sclk,
        .mosi_io_num = pin_mosi,
        .miso_io_num = pin_miso,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        // 这里给一个足够的最大传输尺寸（按你的原代码）
        .max_transfer_sz = width * 40 * sizeof(uint16_t),
    };
    ESP_ERROR_CHECK(spi_bus_initialize(host, &buscfg, SPI_DMA_CH_AUTO));

    // ===== LCD IO (SPI) =====
    esp_lcd_panel_io_spi_config_t io_cfg = {
        .dc_gpio_num = pin_dc,
        .cs_gpio_num = pin_cs,
        .pclk_hz = pclk_hz,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 1, // 最稳（避免队列导致 buffer 覆盖问题）
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)host, &io_cfg, &lcd->io));

    // ===== ILI9341 panel =====
    esp_lcd_panel_dev_config_t panel_cfg = {
        .reset_gpio_num = pin_rst,
        .rgb_ele_order = rgb_ele_order,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(lcd->io, &panel_cfg, &lcd->panel));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(lcd->panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(lcd->panel));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd->panel, true));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(lcd->panel, true, false));  // ★加这一行

    lcd->inited = 1;
    ESP_LOGI(TAG, "ILI9341 初始化完成: %dx%d, pclk=%d", width, height, pclk_hz);
    return ESP_OK;
}

esp_err_t ili9341_simple_fill_rect(ili9341_simple_t *lcd,
                                   int x, int y, int w, int h,
                                   uint16_t color)
{
    if (!lcd || !lcd->inited) return ESP_ERR_INVALID_STATE;

    clip_rect(lcd, &x, &y, &w, &h);
    if (w == 0 || h == 0) return ESP_OK;

    // 说明：
    // - 为了最稳，这里按“每行刷”。
    // - line_buf 用静态数组，避免频繁 malloc。
    // - 注意：w 不能超过 lcd->width，否则会越界
    static uint16_t line_buf[320]; // ILI9341 常见最大宽 320，够用

    for (int i = 0; i < w; i++) line_buf[i] = color;

    for (int row = 0; row < h; row++) {
        esp_lcd_panel_draw_bitmap(lcd->panel, x, y + row, x + w, y + row + 1, line_buf);
    }
    return ESP_OK;
}

esp_err_t ili9341_simple_fill_screen(ili9341_simple_t *lcd, uint16_t color)
{
    return ili9341_simple_fill_rect(lcd, 0, 0, lcd->width, lcd->height, color);
}

esp_err_t ili9341_simple_disp_on(ili9341_simple_t *lcd, int on)
{
    if (!lcd || !lcd->inited) return ESP_ERR_INVALID_STATE;
    return esp_lcd_panel_disp_on_off(lcd->panel, on ? true : false);
}

esp_err_t ili9341_simple_mirror(ili9341_simple_t *lcd, int mirror_x, int mirror_y)
{
    if (!lcd || !lcd->inited) return ESP_ERR_INVALID_STATE;
    return esp_lcd_panel_mirror(lcd->panel, mirror_x ? true : false, mirror_y ? true : false);
}

esp_err_t ili9341_simple_swap_xy(ili9341_simple_t *lcd, int swap)
{
    if (!lcd || !lcd->inited) return ESP_ERR_INVALID_STATE;
    return esp_lcd_panel_swap_xy(lcd->panel, swap ? true : false);
}