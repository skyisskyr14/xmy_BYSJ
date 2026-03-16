#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "esp_wifi.h"

#define WIFI_SCAN_MAX_AP   20
#define WIFI_SSID_MAX_LEN  32
#define WIFI_PASS_MAX_LEN  64
#define WIFI_SLOT_COUNT    5

typedef struct {
    bool scan_done;
    bool got_ip;
    bool connect_fail;
    bool disconnected;

    char status[96];
    char ip[32];
    char disconnect_reason[64];
} wifi_manager_events_t;

typedef struct {
    char ssid[WIFI_SSID_MAX_LEN + 1];
    char pass[WIFI_PASS_MAX_LEN + 1];
    char last_ip[32];      // 上一次成功连接拿到的 IP
    uint8_t valid;         // 0=空槽位 1=有效
    uint8_t last_ok;       // 0=上次失败 1=上次成功
} wifi_slot_t;

typedef struct {
    wifi_slot_t slots[WIFI_SLOT_COUNT];
    int32_t last_used_index;
} wifi_book_t;

void wifi_manager_init(void);

esp_err_t wifi_manager_start_scan(void);
esp_err_t wifi_manager_connect(const char *ssid, const char *pass);

/* 已保存 WiFi 配置簿 */
void wifi_manager_book_load(void);
esp_err_t wifi_manager_book_save(void);
const wifi_book_t *wifi_manager_book_get(void);
esp_err_t wifi_manager_book_add_or_update(const char *ssid, const char *pass);
esp_err_t wifi_manager_book_delete(int index);
esp_err_t wifi_manager_connect_saved(int index);
esp_err_t wifi_manager_disconnect(void);

/* 事件与扫描结果 */
void wifi_manager_poll_events(wifi_manager_events_t *out);
const wifi_ap_record_t *wifi_manager_get_ap_records(uint16_t *count);

#endif