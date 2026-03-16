#include "wifi_manager.h"

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "nvs.h"

static const char *TAG = "wifi_manager";

#define WIFI_NVS_NAMESPACE "wifi_book"
#define WIFI_NVS_KEY_BOOK  "book"

static bool s_wifi_inited = false;
static bool s_wifi_scanning = false;
static bool s_wifi_connecting = false;

static char s_status[96] = "Idle";
static char s_ip[32] = "0.0.0.0";
static char s_disconnect_reason[64] = {0};

static wifi_ap_record_t s_ap_records[WIFI_SCAN_MAX_AP];
static uint16_t s_ap_count = 0;

static volatile bool s_scan_done_flag = false;
static volatile bool s_ip_got_flag = false;
static volatile bool s_connect_fail_flag = false;
static volatile bool s_disconnect_flag = false;

/* 已保存 WiFi 配置簿 */
static wifi_book_t s_book = {0};

/* 当前正在尝试连接的热点 */
static char s_pending_ssid[WIFI_SSID_MAX_LEN + 1] = {0};
static char s_pending_pass[WIFI_PASS_MAX_LEN + 1] = {0};

static void wifi_set_status(const char *text)
{
    if (text) {
        strncpy(s_status, text, sizeof(s_status) - 1);
        s_status[sizeof(s_status) - 1] = '\0';
    }
}

void wifi_manager_book_load(void)
{
    nvs_handle_t nvs;
    esp_err_t err = nvs_open(WIFI_NVS_NAMESPACE, NVS_READONLY, &nvs);
    if (err != ESP_OK) {
        memset(&s_book, 0, sizeof(s_book));
        s_book.last_used_index = -1;
        return;
    }

    size_t required = sizeof(s_book);
    err = nvs_get_blob(nvs, WIFI_NVS_KEY_BOOK, &s_book, &required);
    nvs_close(nvs);

    if (err != ESP_OK || required != sizeof(s_book)) {
        memset(&s_book, 0, sizeof(s_book));
        s_book.last_used_index = -1;
        ESP_LOGI(TAG, "no saved wifi book");
        return;
    }

    ESP_LOGI(TAG, "wifi book loaded");
}

esp_err_t wifi_manager_book_save(void)
{
    nvs_handle_t nvs;
    esp_err_t err = nvs_open(WIFI_NVS_NAMESPACE, NVS_READWRITE, &nvs);
    if (err != ESP_OK) {
        return err;
    }

    err = nvs_set_blob(nvs, WIFI_NVS_KEY_BOOK, &s_book, sizeof(s_book));
    if (err == ESP_OK) {
        err = nvs_commit(nvs);
    }

    nvs_close(nvs);
    return err;
}

const wifi_book_t *wifi_manager_book_get(void)
{
    return &s_book;
}

static int wifi_book_find_ssid(const char *ssid)
{
    if (!ssid || ssid[0] == '\0') {
        return -1;
    }

    for (int i = 0; i < WIFI_SLOT_COUNT; i++) {
        if (s_book.slots[i].valid &&
            strcmp(s_book.slots[i].ssid, ssid) == 0) {
            return i;
        }
    }
    return -1;
}

static int wifi_book_find_empty_slot(void)
{
    for (int i = 0; i < WIFI_SLOT_COUNT; i++) {
        if (!s_book.slots[i].valid) {
            return i;
        }
    }
    return -1;
}

esp_err_t wifi_manager_book_add_or_update(const char *ssid, const char *pass)
{
    if (!ssid || ssid[0] == '\0') {
        return ESP_ERR_INVALID_ARG;
    }

    int idx = wifi_book_find_ssid(ssid);
    if (idx < 0) {
        idx = wifi_book_find_empty_slot();
    }
    if (idx < 0) {
        /* 没空位时简单覆盖最后一个 */
        idx = WIFI_SLOT_COUNT - 1;
    }

    /* 先保留旧的 last_ip / last_ok，避免更新密码时被清掉 */
    char old_ip[32] = {0};
    uint8_t old_last_ok = 0;

    if (s_book.slots[idx].valid) {
        strncpy(old_ip, s_book.slots[idx].last_ip, sizeof(old_ip) - 1);
        old_last_ok = s_book.slots[idx].last_ok;
    }

    memset(&s_book.slots[idx], 0, sizeof(s_book.slots[idx]));

    strncpy(s_book.slots[idx].ssid, ssid, WIFI_SSID_MAX_LEN);
    s_book.slots[idx].ssid[WIFI_SSID_MAX_LEN] = '\0';

    if (pass) {
        strncpy(s_book.slots[idx].pass, pass, WIFI_PASS_MAX_LEN);
        s_book.slots[idx].pass[WIFI_PASS_MAX_LEN] = '\0';
    }

    strncpy(s_book.slots[idx].last_ip, old_ip, sizeof(s_book.slots[idx].last_ip) - 1);
    s_book.slots[idx].last_ip[sizeof(s_book.slots[idx].last_ip) - 1] = '\0';
    s_book.slots[idx].last_ok = old_last_ok;
    s_book.slots[idx].valid = 1;

    s_book.last_used_index = idx;

    ESP_LOGI(TAG, "save/update slot=%d ssid=%s", idx, ssid);
    return wifi_manager_book_save();
}

esp_err_t wifi_manager_book_delete(int index)
{
    if (index < 0 || index >= WIFI_SLOT_COUNT) {
        return ESP_ERR_INVALID_ARG;
    }

    memset(&s_book.slots[index], 0, sizeof(s_book.slots[index]));

    if (s_book.last_used_index == index) {
        s_book.last_used_index = -1;
    }

    ESP_LOGI(TAG, "delete slot=%d", index);
    return wifi_manager_book_save();
}

esp_err_t wifi_manager_connect(const char *ssid, const char *pass)
{
    if (!s_wifi_inited) {
        return ESP_ERR_INVALID_STATE;
    }
    if (!ssid || ssid[0] == '\0') {
        return ESP_ERR_INVALID_ARG;
    }

    wifi_config_t cfg = {0};

    strncpy((char *)cfg.sta.ssid, ssid, sizeof(cfg.sta.ssid) - 1);
    if (pass) {
        strncpy((char *)cfg.sta.password, pass, sizeof(cfg.sta.password) - 1);
    }

    cfg.sta.scan_method = WIFI_FAST_SCAN;
    cfg.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    cfg.sta.threshold.rssi = -127;
    cfg.sta.threshold.authmode = WIFI_AUTH_OPEN;

    /* 记录本次准备连接的信息，后续成功/失败时写回保存簿 */
    strncpy(s_pending_ssid, ssid, sizeof(s_pending_ssid) - 1);
    s_pending_ssid[sizeof(s_pending_ssid) - 1] = '\0';

    if (pass) {
        strncpy(s_pending_pass, pass, sizeof(s_pending_pass) - 1);
        s_pending_pass[sizeof(s_pending_pass) - 1] = '\0';
    } else {
        s_pending_pass[0] = '\0';
    }

    esp_wifi_disconnect();
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &cfg));

    s_wifi_connecting = true;
    wifi_set_status("Connecting...");
    strncpy(s_ip, "0.0.0.0", sizeof(s_ip) - 1);
    s_ip[sizeof(s_ip) - 1] = '\0';

    return esp_wifi_connect();
}

esp_err_t wifi_manager_connect_saved(int index)
{
    if (index < 0 || index >= WIFI_SLOT_COUNT) {
        return ESP_ERR_INVALID_ARG;
    }

    if (!s_book.slots[index].valid) {
        return ESP_ERR_INVALID_STATE;
    }

    s_book.last_used_index = index;
    ESP_ERROR_CHECK(wifi_manager_book_save());

    return wifi_manager_connect(
        s_book.slots[index].ssid,
        s_book.slots[index].pass
    );
}

static void wifi_event_handler(void *arg,
                               esp_event_base_t event_base,
                               int32_t event_id,
                               void *event_data)
{
    (void)arg;

    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_SCAN_DONE: {
                uint16_t ap_num = WIFI_SCAN_MAX_AP;
                memset(s_ap_records, 0, sizeof(s_ap_records));

                if (esp_wifi_scan_get_ap_records(&ap_num, s_ap_records) == ESP_OK) {
                    s_ap_count = ap_num;
                } else {
                    s_ap_count = 0;
                }

                s_wifi_scanning = false;
                wifi_set_status("Scan done");
                s_scan_done_flag = true;
                break;
            }

            case WIFI_EVENT_STA_CONNECTED:
                wifi_set_status("Connected, waiting IP");
                break;

            case WIFI_EVENT_STA_DISCONNECTED: {
                wifi_event_sta_disconnected_t *disc = (wifi_event_sta_disconnected_t *)event_data;

                s_wifi_connecting = false;
                snprintf(s_disconnect_reason, sizeof(s_disconnect_reason),
                         "Disconnected(%d)", disc ? disc->reason : -1);
                wifi_set_status(s_disconnect_reason);

                strncpy(s_ip, "0.0.0.0", sizeof(s_ip) - 1);
                s_ip[sizeof(s_ip) - 1] = '\0';

                s_disconnect_flag = true;
                s_connect_fail_flag = true;

                /* 如果这个 SSID 在保存簿里，标记上次失败 */
                if (s_pending_ssid[0] != '\0') {
                    int idx = wifi_book_find_ssid(s_pending_ssid);
                    if (idx >= 0) {
                        s_book.slots[idx].last_ok = 0;
                        wifi_manager_book_save();
                    }
                }
                break;
            }

            default:
                break;
        }
    } else if (event_base == IP_EVENT) {
        if (event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;

            snprintf(s_ip, sizeof(s_ip), IPSTR, IP2STR(&event->ip_info.ip));
            s_wifi_connecting = false;
            wifi_set_status("Got IP");
            s_ip_got_flag = true;

            /* 真正连上后再写进配置簿 */
            if (s_pending_ssid[0] != '\0') {
                wifi_manager_book_add_or_update(s_pending_ssid, s_pending_pass);

                int idx = wifi_book_find_ssid(s_pending_ssid);
                if (idx >= 0) {
                    s_book.slots[idx].last_ok = 1;
                    s_book.last_used_index = idx;

                    strncpy(s_book.slots[idx].last_ip, s_ip, sizeof(s_book.slots[idx].last_ip) - 1);
                    s_book.slots[idx].last_ip[sizeof(s_book.slots[idx].last_ip) - 1] = '\0';

                    wifi_manager_book_save();
                }
            }
        }
    }
}

void wifi_manager_init(void)
{
    if (s_wifi_inited) {
        return;
    }

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_manager_book_load();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    s_wifi_inited = true;
    wifi_set_status("WiFi ready");

    /* 开机优先自动连接上次使用的 */
    if (s_book.last_used_index >= 0 &&
        s_book.last_used_index < WIFI_SLOT_COUNT &&
        s_book.slots[s_book.last_used_index].valid) {
        ESP_LOGI(TAG, "auto connect saved slot=%ld ssid=%s",
                 (long)s_book.last_used_index,
                 s_book.slots[s_book.last_used_index].ssid);
        wifi_manager_connect_saved(s_book.last_used_index);
    }
}

esp_err_t wifi_manager_start_scan(void)
{
    if (!s_wifi_inited) {
        return ESP_ERR_INVALID_STATE;
    }
    if (s_wifi_scanning) {
        return ESP_ERR_INVALID_STATE;
    }

    wifi_scan_config_t scan_cfg = {
        .ssid = NULL,
        .bssid = NULL,
        .channel = 0,
        .show_hidden = true,
        .scan_type = WIFI_SCAN_TYPE_ACTIVE,
    };

    s_ap_count = 0;
    memset(s_ap_records, 0, sizeof(s_ap_records));

    s_wifi_scanning = true;
    wifi_set_status("Scanning...");
    strncpy(s_ip, "0.0.0.0", sizeof(s_ip) - 1);
    s_ip[sizeof(s_ip) - 1] = '\0';

    return esp_wifi_scan_start(&scan_cfg, false);
}

void wifi_manager_poll_events(wifi_manager_events_t *out)
{
    if (!out) {
        return;
    }

    memset(out, 0, sizeof(*out));

    out->scan_done = s_scan_done_flag;
    out->got_ip = s_ip_got_flag;
    out->connect_fail = s_connect_fail_flag;
    out->disconnected = s_disconnect_flag;

    strncpy(out->status, s_status, sizeof(out->status) - 1);
    out->status[sizeof(out->status) - 1] = '\0';

    strncpy(out->ip, s_ip, sizeof(out->ip) - 1);
    out->ip[sizeof(out->ip) - 1] = '\0';

    strncpy(out->disconnect_reason, s_disconnect_reason, sizeof(out->disconnect_reason) - 1);
    out->disconnect_reason[sizeof(out->disconnect_reason) - 1] = '\0';

    s_scan_done_flag = false;
    s_ip_got_flag = false;
    s_connect_fail_flag = false;
    s_disconnect_flag = false;
}

const wifi_ap_record_t *wifi_manager_get_ap_records(uint16_t *count)
{
    if (count) {
        *count = s_ap_count;
    }
    return s_ap_records;
}

esp_err_t wifi_manager_disconnect(void)
{
    if (!s_wifi_inited) {
        return ESP_ERR_INVALID_STATE;
    }

    s_wifi_connecting = false;
    wifi_set_status("Disconnected");
    strncpy(s_ip, "0.0.0.0", sizeof(s_ip) - 1);
    s_ip[sizeof(s_ip) - 1] = '\0';

    return esp_wifi_disconnect();
}