#ifndef UI_PAGES_H
#define UI_PAGES_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "esp_wifi.h"
#include "lvgl.h"
#include "wifi/wifi_manager.h"

typedef struct {
    void (*on_scan)(void);
    void (*on_connect)(void);
    void (*on_disconnect)(void);
    void (*on_saved_use)(int index);
    void (*on_saved_delete)(int index);
} ui_pages_callbacks_t;

void ui_pages_create(const ui_pages_callbacks_t *cbs);

void ui_pages_show_water(void);
void ui_pages_show_wifi(void);
void ui_pages_show_saved(void);

void ui_pages_update_turbidity(int turbidity, const char *level);

void ui_pages_set_wifi_status(const char *status);
void ui_pages_set_wifi_ip(const char *ip);
void ui_pages_set_wifi_tip(const char *tip);
void ui_pages_set_wifi_list(const wifi_ap_record_t *records, uint16_t count);

void ui_pages_refresh_saved_wifi(const wifi_book_t *book);

void ui_pages_get_selected_ssid(char *buf, size_t len);
void ui_pages_get_password(char *buf, size_t len);

void ui_pages_set_touch_point(bool pressed, uint16_t x, uint16_t y);

void ui_pages_set_nav_wifi(const char *status, const char *ssid, const char *ip);

void ui_pages_set_wifi_connected(bool connected);
void ui_pages_set_network_info(const char *status, const char *ssid, const char *pass, const char *ip);
void ui_pages_show_network_info(void);

void ui_pages_update_tds(float ppm, const char *level);
void ui_pages_update_temp(float temp_c, const char *level);
void ui_pages_update_ph(float ph, const char *level);
#endif