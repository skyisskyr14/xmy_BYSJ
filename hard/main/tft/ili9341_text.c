#include "ili9341_text.h"
#include <string.h>
#include "font8x16.h"
#include "hz16x16.h"



/* =========================
 * 核心：按行绘制（最稳）
 * ========================= */


esp_err_t ili9341_draw_char8x16(ili9341_simple_t *lcd, int x, int y, char c, uint16_t fg, uint16_t bg)
{
    if (!lcd || !lcd->inited) return ESP_ERR_INVALID_STATE;

    const uint8_t *m = font8x16_get(c);
    if (!m) return ESP_ERR_NOT_FOUND;

    uint16_t line[8];
    for (int row = 0; row < 16; row++) {
        uint8_t bits = m[row];
        for (int col = 0; col < 8; col++) {
            int on = (bits >> (7 - col)) & 1;
            line[col] = on ? fg : bg;
        }
        esp_lcd_panel_draw_bitmap((esp_lcd_panel_handle_t)lcd->panel, x, y + row, x + 8, y + row + 1, line);
    }
    return ESP_OK;
}

esp_err_t ili9341_draw_text8x16(ili9341_simple_t *lcd,
                                int x, int y,
                                const char *s,
                                uint16_t fg, uint16_t bg,
                                int spacing)
{
    if (!s)
        return ESP_ERR_INVALID_ARG;
    int cx = x;

    for (size_t i = 0; i < strlen(s); i++)
    {
        esp_err_t err = ili9341_draw_char8x16(lcd, cx, y, s[i], fg, bg);
        if (err == ESP_OK)
        {
            cx += 8 + spacing;
        }
        else
        {
            // 不支持的字符：跳过一个宽度（你也可以改成画空格）
            cx += 8 + spacing;
        }
    }
    return ESP_OK;
}

esp_err_t ili9341_draw_hz16x16(ili9341_simple_t *lcd, int x, int y, const char *hz, uint16_t fg, uint16_t bg)
{
    if (!lcd || !lcd->inited) return ESP_ERR_INVALID_STATE;
    if (!hz) return ESP_ERR_INVALID_ARG;

    const uint8_t *m = hz16x16_get_utf8(hz);
    if (!m) return ESP_ERR_NOT_FOUND;

    uint16_t line[16];
    for (int row = 0; row < 16; row++) {
        uint16_t bits = ((uint16_t)m[row * 2] << 8) | m[row * 2 + 1];
        for (int col = 0; col < 16; col++) {
            int on = (bits >> (15 - col)) & 1;
            line[col] = on ? fg : bg;
        }
        esp_lcd_panel_draw_bitmap((esp_lcd_panel_handle_t)lcd->panel, x, y + row, x + 16, y + row + 1, line);
    }
    return ESP_OK;
}

esp_err_t ili9341_draw_hz_text16x16(ili9341_simple_t *lcd,
                                    int x, int y,
                                    const char *utf8,
                                    uint16_t fg, uint16_t bg,
                                    int spacing)
{
    if (!utf8)
        return ESP_ERR_INVALID_ARG;

    int cx = x;
    const unsigned char *p = (const unsigned char *)utf8;

    while (*p)
    {
        // ASCII（单字节）
        if (*p < 0x80)
        {
            // 这里你如果想混排 ASCII，可以调用 8x16
            // 暂时简单处理：跳过
            cx += 8 + spacing;
            p += 1;
            continue;
        }

        // UTF-8 汉字一般 3 字节（本项目先按 3 字节处理）
        if (p[0] && p[1] && p[2])
        {
            esp_err_t err = ili9341_draw_hz16x16(lcd, cx, y, (const char *)p, fg, bg);
            (void)err;
            cx += 16 + spacing;
            p += 3;
        }
        else
        {
            break;
        }
    }

    return ESP_OK;
}