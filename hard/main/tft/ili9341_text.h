#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "ili9341_simple.h"   // 你之前的封装（里面有 ili9341_simple_t）

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 绘制一个 8x16 ASCII 字符（目前字库只内置 a/b/d）
 * @param lcd  你的 ili9341_simple_t 对象
 * @param x,y  左上角坐标
 * @param c    字符：'a'/'b'/'d'
 * @param fg   前景色(RGB565)
 * @param bg   背景色(RGB565)
 */
esp_err_t ili9341_draw_char8x16(ili9341_simple_t *lcd,
                                int x, int y,
                                char c,
                                uint16_t fg, uint16_t bg);

/**
 * @brief 绘制 8x16 ASCII 字符串（目前只保证 a/b/d）
 * @param spacing 字符间距（像素）
 */
esp_err_t ili9341_draw_text8x16(ili9341_simple_t *lcd,
                                int x, int y,
                                const char *s,
                                uint16_t fg, uint16_t bg,
                                int spacing);

/**
 * @brief 绘制一个 16x16 汉字（目前内置 "中"、"文" 两个样例）
 * @param hz  UTF-8 字符串指针（指向一个汉字的起始位置）
 *
 * 注意：UTF-8 汉字通常占 3 字节，本函数只识别 "中"(E4 B8 AD) 和 "文"(E6 96 87)
 */
esp_err_t ili9341_draw_hz16x16(ili9341_simple_t *lcd,
                               int x, int y,
                               const char *hz,
                               uint16_t fg, uint16_t bg);

/**
 * @brief 绘制 16x16 汉字字符串（目前只识别 "中"、"文"）
 * @param spacing 字间距
 */
esp_err_t ili9341_draw_hz_text16x16(ili9341_simple_t *lcd,
                                    int x, int y,
                                    const char *utf8,
                                    uint16_t fg, uint16_t bg,
                                    int spacing);

#ifdef __cplusplus
}
#endif