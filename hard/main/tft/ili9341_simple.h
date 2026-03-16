#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "esp_lcd_panel_ops.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"

/**
 * @brief ILI9341 SPI 简易对象（保存句柄与参数）
 */
typedef struct {
    // 句柄
    esp_lcd_panel_io_handle_t io;
    esp_lcd_panel_handle_t panel;

    // 屏幕尺寸（用于填充/裁剪）
    int width;
    int height;

    // 是否已初始化
    int inited;
} ili9341_simple_t;

/**
 * @brief RGB888 -> RGB565
 */
static inline uint16_t ili9341_rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

/**
 * @brief 初始化 ILI9341（SPI）
 *
 * 说明：
 * - 本函数会初始化 SPI bus、创建 panel io、创建 ili9341 panel 并 reset/init。
 * - 背光如果 pin_bl>=0 会设置为输出并拉高。
 */
esp_err_t ili9341_simple_init(ili9341_simple_t *lcd,
                             spi_host_device_t host,
                             int width, int height,
                             int pin_sclk, int pin_mosi, int pin_miso,
                             int pin_cs, int pin_dc,
                             int pin_rst, int pin_bl,
                             int pclk_hz,
                             int rgb_ele_order);
/**
 * @brief 全屏填充单色（最稳：按行刷）
 */
esp_err_t ili9341_simple_fill_screen(ili9341_simple_t *lcd, uint16_t color);

/**
 * @brief 填充矩形区域（按行刷）
 */
esp_err_t ili9341_simple_fill_rect(ili9341_simple_t *lcd,
                                   int x, int y, int w, int h,
                                   uint16_t color);

/**
 * @brief 设置显示开关
 */
esp_err_t ili9341_simple_disp_on(ili9341_simple_t *lcd, int on);

/**
 * @brief 镜像（左右/上下）
 */
esp_err_t ili9341_simple_mirror(ili9341_simple_t *lcd, int mirror_x, int mirror_y);

/**
 * @brief 交换 XY（相当于旋转 90°）
 */
esp_err_t ili9341_simple_swap_xy(ili9341_simple_t *lcd, int swap);