#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "esp_err.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_ili9341.h"

#include "tft/ili9341_simple.h"
#include "touch_xpt2046.h"
#include "lvgl.h"

#include "wifi/wifi_manager.h"
#include "sensor/sensor_service.h"
#include "ui/ui_pages.h"
#include "turbidity.h"
#include "tds_sensor.h"
#include "temp_sensor.h"
#include "ph_sensor.h"

static const char *TAG = "main";

/* =========================
 * 启动日志页对象
 * boot_screen : 黑底启动页容器
 * boot_label  : 启动日志文本
 * boot_text   : 累积日志缓冲
 * ========================= */
static lv_obj_t *boot_screen = NULL;
static lv_obj_t *boot_label = NULL;
static char boot_text[2048] = {0};

/* =========================
 * LCD 引脚定义
 * ========================= */
#define PIN_SCLK 12
#define PIN_MOSI 11
#define PIN_MISO 13
#define PIN_CS 10
#define PIN_DC 9
#define PIN_RST 8
#define PIN_BL 7

/* =========================
 * 触摸芯片 XPT2046 引脚定义
 * ========================= */
#define PIN_T_CS 6
#define PIN_T_IRQ 5

/* =========================
 * SPI 主机与屏幕分辨率
 * ========================= */
#define LCD_HOST SPI2_HOST
#define LCD_W 240
#define LCD_H 320

static ili9341_simple_t lcd;

/* =========================
 * LVGL 显示缓冲区
 * ========================= */
static lv_disp_draw_buf_t draw_buf;
static lv_color_t lv_buf[LCD_W * 40];

/* =========================
 * 当前网络信息缓存
 * 给：
 * - 顶部导航栏
 * - Network Info 页面
 * 使用
 * ========================= */
static char g_nav_ssid[WIFI_SSID_MAX_LEN + 1] = "-";
static char g_nav_ip[32] = "-";
static char g_nav_status[96] = "Idle";
static char g_nav_pass[WIFI_PASS_MAX_LEN + 1] = "-";

/* =========================
 * 前置声明
 * ========================= */
static void sync_nav_from_last_saved_wifi(void);
static void boot_screen_create(void);
static void boot_log(const char *line);
static void boot_delay_ms(int ms);
static void boot_screen_destroy(void);

/* =========================
 * LVGL 刷屏回调
 * ========================= */
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p)
{
    int w = (area->x2 - area->x1 + 1);

    for (int y = area->y1; y <= area->y2; y++)
    {
        esp_lcd_panel_draw_bitmap(
            lcd.panel,
            area->x1,
            y,
            area->x2 + 1,
            y + 1,
            color_p);
        color_p += w;
    }

    lv_disp_flush_ready(drv);
}

/* =========================
 * LVGL 时基函数
 * 每 1ms 调一次
 * ========================= */
static void lv_tick_task(void *arg)
{
    (void)arg;
    lv_tick_inc(1);
}

/* =========================
 * 触摸读取回调
 * LVGL 会周期调用
 * ========================= */
static void touch_read_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    (void)drv;

    uint16_t x, y;

    if (touch_read(&x, &y))
    {
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = x;
        data->point.y = y;
        ui_pages_set_touch_point(true, x, y);
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
        ui_pages_set_touch_point(false, 0, 0);
    }
}

/* =========================
 * 点击 Scan 按钮
 * ========================= */
static void on_scan_clicked(void)
{
    esp_err_t err = wifi_manager_start_scan();

    if (err != ESP_OK)
    {
        ui_pages_set_wifi_status("Scan start failed");
        ESP_LOGE(TAG, "wifi_manager_start_scan failed: %s", esp_err_to_name(err));
    }
    else
    {
        ui_pages_set_wifi_tip("Scanning nearby WiFi...");
    }
}

/* =========================
 * 点击 Connect 按钮
 * 从 UI 读取：
 * - 当前选中的 SSID
 * - 当前输入的密码
 * 然后发起连接
 * ========================= */
static void on_connect_clicked(void)
{
    char ssid[WIFI_SSID_MAX_LEN + 1] = {0};
    char pass[WIFI_PASS_MAX_LEN + 1] = {0};

    ui_pages_get_selected_ssid(ssid, sizeof(ssid));
    ui_pages_get_password(pass, sizeof(pass));

    if (ssid[0] == '\0')
    {
        ui_pages_set_wifi_status("Please select SSID");
        return;
    }

    esp_err_t err = wifi_manager_connect(ssid, pass);
    if (err != ESP_OK)
    {
        ui_pages_set_wifi_status("Connect start failed");
        ESP_LOGE(TAG, "wifi_manager_connect failed: %s", esp_err_to_name(err));
    }
    else
    {
        strncpy(g_nav_ssid, ssid, sizeof(g_nav_ssid) - 1);
        g_nav_ssid[sizeof(g_nav_ssid) - 1] = '\0';

        strncpy(g_nav_pass, pass, sizeof(g_nav_pass) - 1);
        g_nav_pass[sizeof(g_nav_pass) - 1] = '\0';

        strncpy(g_nav_status, "Connecting...", sizeof(g_nav_status) - 1);
        g_nav_status[sizeof(g_nav_status) - 1] = '\0';

        ui_pages_set_wifi_connected(false);
        ui_pages_set_wifi_tip("Connecting...");
        ui_pages_set_nav_wifi(g_nav_status, g_nav_ssid, g_nav_ip);
        ui_pages_set_network_info(g_nav_status, g_nav_ssid, g_nav_pass, g_nav_ip);
    }
}

/* =========================
 * 点击 Disconnect 按钮
 * ========================= */
static void on_disconnect_clicked(void)
{
    esp_err_t err = wifi_manager_disconnect();
    if (err != ESP_OK)
    {
        ui_pages_set_wifi_status("Disconnect failed");
        ESP_LOGE(TAG, "wifi_manager_disconnect failed: %s", esp_err_to_name(err));
        return;
    }

    strncpy(g_nav_status, "Disconnected", sizeof(g_nav_status) - 1);
    g_nav_status[sizeof(g_nav_status) - 1] = '\0';

    strncpy(g_nav_ip, "-", sizeof(g_nav_ip) - 1);
    g_nav_ip[sizeof(g_nav_ip) - 1] = '\0';

    ui_pages_set_wifi_connected(false);
    ui_pages_set_wifi_tip("Disconnected");
    ui_pages_set_wifi_status("Disconnected");
    ui_pages_set_nav_wifi(g_nav_status, g_nav_ssid, g_nav_ip);
    ui_pages_set_network_info(g_nav_status, g_nav_ssid, g_nav_pass, g_nav_ip);
}

/* =========================
 * Saved 页点击 Use
 * 读取保存簿里的 SSID/PASS
 * 然后连接该保存项
 * ========================= */
static void on_saved_use_clicked(int index)
{
    const wifi_book_t *book = wifi_manager_book_get();
    if (book && index >= 0 && index < WIFI_SLOT_COUNT && book->slots[index].valid)
    {
        strncpy(g_nav_ssid, book->slots[index].ssid, sizeof(g_nav_ssid) - 1);
        g_nav_ssid[sizeof(g_nav_ssid) - 1] = '\0';

        strncpy(g_nav_pass, book->slots[index].pass, sizeof(g_nav_pass) - 1);
        g_nav_pass[sizeof(g_nav_pass) - 1] = '\0';
    }

    esp_err_t err = wifi_manager_connect_saved(index);
    if (err != ESP_OK)
    {
        ui_pages_set_wifi_status("Saved connect failed");
        ui_pages_set_wifi_tip("Use saved WiFi failed");
        ESP_LOGE(TAG, "wifi_manager_connect_saved failed: %s", esp_err_to_name(err));
    }
    else
    {
        strncpy(g_nav_status, "Connecting...", sizeof(g_nav_status) - 1);
        g_nav_status[sizeof(g_nav_status) - 1] = '\0';

        ui_pages_set_wifi_connected(false);
        ui_pages_set_wifi_tip("Connecting saved WiFi...");
        ui_pages_set_nav_wifi(g_nav_status, g_nav_ssid, g_nav_ip);
        ui_pages_set_network_info(g_nav_status, g_nav_ssid, g_nav_pass, g_nav_ip);
        ui_pages_show_wifi();
    }
}

/* =========================
 * Saved 页点击 Delete
 * ========================= */
static void on_saved_delete_clicked(int index)
{
    esp_err_t err = wifi_manager_book_delete(index);
    if (err != ESP_OK)
    {
        ui_pages_set_wifi_tip("Delete failed");
        ESP_LOGE(TAG, "wifi_manager_book_delete failed: %s", esp_err_to_name(err));
        return;
    }

    ui_pages_refresh_saved_wifi(wifi_manager_book_get());
    ui_pages_set_wifi_tip("Deleted");
}

/* =========================
 * 处理 WiFi 模块异步事件
 * 统一同步到：
 * - WiFi 页面
 * - 顶部导航栏
 * - 网络信息页
 * ========================= */
static void process_wifi_events(void)
{
    wifi_manager_events_t events;
    wifi_manager_poll_events(&events);

    ui_pages_set_wifi_status(events.status);

    strncpy(g_nav_status, events.status, sizeof(g_nav_status) - 1);
    g_nav_status[sizeof(g_nav_status) - 1] = '\0';

    if (events.scan_done)
    {
        uint16_t count = 0;
        const wifi_ap_record_t *records = wifi_manager_get_ap_records(&count);
        ui_pages_set_wifi_list(records, count);
    }

    if (events.got_ip)
    {
        strncpy(g_nav_ip, events.ip, sizeof(g_nav_ip) - 1);
        g_nav_ip[sizeof(g_nav_ip) - 1] = '\0';

        ui_pages_set_wifi_connected(true);
        ui_pages_set_wifi_tip("Connected");
        ui_pages_refresh_saved_wifi(wifi_manager_book_get());
    }

    if (events.connect_fail || events.disconnected)
    {
        strncpy(g_nav_ip, "-", sizeof(g_nav_ip) - 1);
        g_nav_ip[sizeof(g_nav_ip) - 1] = '\0';

        ui_pages_set_wifi_connected(false);
        ui_pages_set_wifi_tip("Connect failed, retry or rescan");
        ui_pages_refresh_saved_wifi(wifi_manager_book_get());
    }

    ui_pages_set_nav_wifi(g_nav_status, g_nav_ssid, g_nav_ip);
    ui_pages_set_network_info(g_nav_status, g_nav_ssid, g_nav_pass, g_nav_ip);
}

/* =========================
 * 重启后同步上次使用的保存 WiFi
 * 让自动重连时：
 * - SSID
 * - PASS
 * 能正确显示到 Network Info 页
 * ========================= */
static void sync_nav_from_last_saved_wifi(void)
{
    const wifi_book_t *book = wifi_manager_book_get();
    if (!book)
    {
        return;
    }

    int idx = book->last_used_index;
    if (idx < 0 || idx >= WIFI_SLOT_COUNT)
    {
        return;
    }

    if (!book->slots[idx].valid)
    {
        return;
    }

    strncpy(g_nav_ssid, book->slots[idx].ssid, sizeof(g_nav_ssid) - 1);
    g_nav_ssid[sizeof(g_nav_ssid) - 1] = '\0';

    strncpy(g_nav_pass, book->slots[idx].pass, sizeof(g_nav_pass) - 1);
    g_nav_pass[sizeof(g_nav_pass) - 1] = '\0';
}

/* =========================
 * 创建启动日志黑底界面
 * ========================= */
static void boot_screen_create(void)
{
    boot_screen = lv_obj_create(lv_scr_act());
    lv_obj_set_size(boot_screen, LCD_W, LCD_H);
    lv_obj_center(boot_screen);
    lv_obj_clear_flag(boot_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(boot_screen, lv_color_black(), 0);
    lv_obj_set_style_border_width(boot_screen, 0, 0);
    lv_obj_set_style_radius(boot_screen, 0, 0);
    lv_obj_set_style_pad_all(boot_screen, 6, 0);

    boot_label = lv_label_create(boot_screen);
    lv_obj_set_width(boot_label, LCD_W - 12);
    lv_obj_align(boot_label, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_color(boot_label, lv_color_white(), 0);
    lv_label_set_long_mode(boot_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(boot_label, "");
}

/* =========================
 * 启动日志追加一行
 * ========================= */
static void boot_log(const char *line)
{
    if (!boot_label || !line)
    {
        return;
    }

    if (strlen(boot_text) + strlen(line) + 2 >= sizeof(boot_text))
    {
        boot_text[0] = '\0';
    }

    strcat(boot_text, line);
    strcat(boot_text, "\n");

    lv_label_set_text(boot_label, boot_text);
    lv_timer_handler();
}

/* =========================
 * 启动阶段专用延时
 * 延时期间继续让 LVGL 刷新
 * ========================= */
static void boot_delay_ms(int ms)
{
    int step = 20;
    int count = ms / step;

    for (int i = 0; i < count; i++)
    {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(step));
    }

    int remain = ms % step;
    if (remain > 0)
    {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(remain));
    }
}

/* =========================
 * 销毁启动日志界面
 * ========================= */
static void boot_screen_destroy(void)
{
    if (boot_screen)
    {
        lv_obj_del(boot_screen);
        boot_screen = NULL;
        boot_label = NULL;
        boot_text[0] = '\0';
    }
}

/* =========================
 * app_main
 * 启动流程：
 * 1. 冷启动等待
 * 2. LCD 初始化
 * 3. LVGL 初始化
 * 4. 黑底启动日志页
 * 5. Touch 初始化
 * 6. Sensor 初始化
 * 7. WiFi 初始化
 * 8. 创建正式 UI
 * 9. 销毁启动页，进入主循环
 * ========================= */
void app_main(void)
{
    /* 冷启动先稳定一会 */
    vTaskDelay(pdMS_TO_TICKS(100));

    /* 1. 初始化 LCD */
    ESP_ERROR_CHECK(ili9341_simple_init(
        &lcd,
        LCD_HOST,
        LCD_W,
        LCD_H,
        PIN_SCLK,
        PIN_MOSI,
        PIN_MISO,
        PIN_CS,
        PIN_DC,
        PIN_RST,
        PIN_BL,
        20 * 1000 * 1000,
        LCD_RGB_ELEMENT_ORDER_RGB));

    /* 给 LCD 稳定时间 */
    vTaskDelay(pdMS_TO_TICKS(80));

    /* 2. 初始化 LVGL */
    lv_init();

    lv_disp_draw_buf_init(&draw_buf, lv_buf, NULL, LCD_W * 40);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_W;
    disp_drv.ver_res = LCD_H;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    /* 3. 创建 LVGL tick */
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &lv_tick_task,
        .name = "lv_tick"};

    esp_timer_handle_t lvgl_tick_timer;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, 1000));

    /* 4. 显示启动日志页 */
    boot_screen_create();
    boot_log("[ OK ] LVGL ready");
    boot_delay_ms(500);

    /* 5. 初始化触摸 */
    touch_init(LCD_HOST, PIN_T_CS, PIN_T_IRQ);

    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touch_read_cb;
    lv_indev_drv_register(&indev_drv);

    boot_log("[ OK ] Touch ready");
    boot_delay_ms(500);

    /* 6. 初始化传感器 */
    sensor_service_init();
    boot_log("[ OK ] Sensor ready");
    boot_delay_ms(500);

    /* 7. 初始化 WiFi */
    wifi_manager_init();
    sync_nav_from_last_saved_wifi();
    boot_log("[ OK ] WiFi manager ready");
    boot_delay_ms(500);

    /* 8. 创建正式 UI */
    ui_pages_callbacks_t cbs = {
        .on_scan = on_scan_clicked,
        .on_connect = on_connect_clicked,
        .on_disconnect = on_disconnect_clicked,
        .on_saved_use = on_saved_use_clicked,
        .on_saved_delete = on_saved_delete_clicked,
    };
    ui_pages_create(&cbs);

    ui_pages_refresh_saved_wifi(wifi_manager_book_get());
    ui_pages_set_wifi_connected(false);
    ui_pages_set_nav_wifi(g_nav_status, g_nav_ssid, g_nav_ip);
    ui_pages_set_network_info(g_nav_status, g_nav_ssid, g_nav_pass, g_nav_ip);

    boot_log("[ OK ] UI ready");
    boot_log("[ OK ] System boot complete");
    boot_delay_ms(500);

    /* 9. 销毁启动页，进入正式主界面 */
    boot_screen_destroy();

    /* 主循环 */
    while (1)
    {
        int turb = sensor_service_get_turbidity();
        const char *turb_level = turbidity_level(turb);

        float tds = sensor_service_get_tds();
        const char *tds_level = tds_sensor_level(tds);

        float temp = sensor_service_get_temp();
        const char *temp_level = temp_sensor_level(temp);

        float ph = sensor_service_get_ph();
        const char *ph_level = ph_sensor_level(temp);

        ui_pages_update_turbidity(turb, turb_level);
        ui_pages_update_tds(tds, tds_level);
        ui_pages_update_temp(temp, temp_level);
        ui_pages_update_ph(ph,ph_level);

        process_wifi_events();

        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}