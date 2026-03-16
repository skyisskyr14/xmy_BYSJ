#include "ui_pages.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* =========================
 * 屏幕与布局基础参数
 * SCREEN_W / SCREEN_H   : 整个屏幕分辨率
 * STATUS_BAR_H         : 顶部全局导航栏高度
 * PAGE_H               : 除去导航栏后，各页面可用高度
 * ========================= */
#define SCREEN_W 240
#define SCREEN_H 320
#define STATUS_BAR_H 26
#define PAGE_H (SCREEN_H - STATUS_BAR_H)

/* =========================
 * 页面回调集合
 * ========================= */
static ui_pages_callbacks_t s_cbs = {0};

/* =========================
 * 触摸点对象
 * ========================= */
static lv_obj_t *touch_dot;

/* =========================
 * 顶部全局状态栏对象
 * ========================= */
static lv_obj_t *status_bar;
static lv_obj_t *wifi_bar_1;
static lv_obj_t *wifi_bar_2;
static lv_obj_t *wifi_bar_3;
static lv_obj_t *wifi_bar_4;
static lv_obj_t *wifi_bar_x;
static lv_obj_t *label_nav_title;

/* =========================
 * 四个页面对象
 * ========================= */
static lv_obj_t *page_water;
static lv_obj_t *page_wifi;
static lv_obj_t *page_saved;
static lv_obj_t *page_info;

/* =========================
 * 水质页文本对象
 * ========================= */
static lv_obj_t *label_turb;
static lv_obj_t *label_level;

static lv_obj_t *label_tds;
static lv_obj_t *label_tds_level;

static lv_obj_t *label_temp;
static lv_obj_t *label_temp_level;

static lv_obj_t *label_ph;
static lv_obj_t *label_ph_level;

/* =========================
 * WiFi 连接页控件对象
 * ========================= */
static lv_obj_t *dd_ssid;
static lv_obj_t *ta_pass;
static lv_obj_t *kb;
static lv_obj_t *label_wifi_status;
static lv_obj_t *label_wifi_tip;

/* =========================
 * WiFi 页按钮对象
 * ========================= */
static lv_obj_t *btn_connect;
static lv_obj_t *label_connect_text;
static lv_obj_t *btn_info;

/* =========================
 * 网络信息页文本对象
 * ========================= */
static lv_obj_t *label_info_status;
static lv_obj_t *label_info_ssid;
static lv_obj_t *label_info_ip;
static lv_obj_t *label_info_pass;

/* =========================
 * 已保存 WiFi 页列表对象
 * ========================= */
static lv_obj_t *saved_list;

/* =========================
 * 当前选中的 SSID
 * ========================= */
static char s_selected_ssid[WIFI_SSID_MAX_LEN + 1] = {0};

/* =========================
 * 当前是否已连接 WiFi
 * ========================= */
static bool s_wifi_connected = false;

/* =========================
 * 前置声明
 * ========================= */
static void set_nav_title(const char *title);
static void hide_keyboard(void);
static void show_only_page(lv_obj_t *page);
static void wifi_bar_set_color(lv_obj_t *bar, lv_color_t color);
static void wifi_bar_show_level(int level, bool show_x);

static void style_screen_black(lv_obj_t *obj);
static void style_panel_bw(lv_obj_t *obj);
static void style_btn_bw(lv_obj_t *obj);
static void style_label_white(lv_obj_t *obj);
static void style_input_bw(lv_obj_t *obj);
static void style_dropdown_bw(lv_obj_t *obj);
static void style_keyboard_bw(lv_obj_t *obj);

/* =========================
 * 黑白主题：整页背景
 * ========================= */
static void style_screen_black(lv_obj_t *obj)
{
    lv_obj_set_style_bg_color(obj, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(obj, lv_color_white(), 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_text_color(obj, lv_color_white(), 0);
}

/* =========================
 * 黑白主题：面板/容器
 * ========================= */
static void style_panel_bw(lv_obj_t *obj)
{
    lv_obj_set_style_bg_color(obj, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(obj, lv_color_white(), 0);
    lv_obj_set_style_border_width(obj, 1, 0);
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_set_style_text_color(obj, lv_color_white(), 0);
}

/* =========================
 * 黑白主题：按钮
 * 默认黑底白框白字
 * 按下时白底黑字
 * ========================= */
static void style_btn_bw(lv_obj_t *obj)
{
    lv_obj_set_style_bg_color(obj, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(obj, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN);
    lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN);

    lv_obj_set_style_bg_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_text_color(obj, lv_color_black(), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_set_style_border_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_PRESSED);
}

/* =========================
 * 黑白主题：文字
 * ========================= */
static void style_label_white(lv_obj_t *obj)
{
    lv_obj_set_style_text_color(obj, lv_color_white(), 0);
}

/* =========================
 * 黑白主题：输入框
 * ========================= */
static void style_input_bw(lv_obj_t *obj)
{
    lv_obj_set_style_bg_color(obj, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(obj, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN);
    lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN);

    /* 占位提示文字颜色 */
    lv_obj_set_style_text_color(obj, lv_color_hex(0xBBBBBB), LV_PART_TEXTAREA_PLACEHOLDER);
}

/* =========================
 * 黑白主题：下拉框
 * 收起状态 + 展开列表 + 选中项
 * ========================= */
static void style_dropdown_bw(lv_obj_t *obj)
{
    /* 主体：黑底白框白字 */
    lv_obj_set_style_bg_color(obj, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(obj, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN);
    lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_MAIN);

    /* 右侧箭头 */
    lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_INDICATOR);

    /* 展开列表项 */
    lv_obj_set_style_bg_color(obj, lv_color_black(), LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_ITEMS);
    lv_obj_set_style_border_color(obj, lv_color_white(), LV_PART_ITEMS);
    lv_obj_set_style_border_width(obj, 1, LV_PART_ITEMS);
    lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_ITEMS);
    lv_obj_set_style_radius(obj, 0, LV_PART_ITEMS);

    /* 选中项 */
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x222222), LV_PART_SELECTED);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_SELECTED);
    lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_SELECTED);

    /* 尝试给真实弹出列表也套样式 */
    lv_obj_t *list = lv_dropdown_get_list(obj);
    if (list)
    {
        lv_obj_set_style_bg_color(list, lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(list, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_color(list, lv_color_white(), LV_PART_MAIN);
        lv_obj_set_style_border_width(list, 1, LV_PART_MAIN);
        lv_obj_set_style_radius(list, 0, LV_PART_MAIN);
        lv_obj_set_style_text_color(list, lv_color_white(), LV_PART_MAIN);

        lv_obj_set_style_bg_color(list, lv_color_hex(0x222222), LV_PART_SELECTED);
        lv_obj_set_style_bg_opa(list, LV_OPA_COVER, LV_PART_SELECTED);
        lv_obj_set_style_text_color(list, lv_color_white(), LV_PART_SELECTED);
    }
}

/* =========================
 * 黑白主题：键盘
 * ========================= */
static void style_keyboard_bw(lv_obj_t *obj)
{
    /* 键盘整体 */
    lv_obj_set_style_bg_color(obj, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(obj, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(obj, 4, LV_PART_MAIN);

    /* 按键 */
    lv_obj_set_style_bg_color(obj, lv_color_black(), LV_PART_ITEMS);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_ITEMS);
    lv_obj_set_style_border_color(obj, lv_color_white(), LV_PART_ITEMS);
    lv_obj_set_style_border_width(obj, 1, LV_PART_ITEMS);
    lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_ITEMS);
    lv_obj_set_style_radius(obj, 0, LV_PART_ITEMS);

    /* 按下 */
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x222222), LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_border_color(obj, lv_color_white(), LV_PART_ITEMS | LV_STATE_PRESSED);
    lv_obj_set_style_text_color(obj, lv_color_white(), LV_PART_ITEMS | LV_STATE_PRESSED);
}

/* =========================
 * 隐藏屏幕键盘
 * ========================= */
static void hide_keyboard(void)
{
    if (kb)
    {
        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_keyboard_set_textarea(kb, NULL);
    }
}

/* =========================
 * 页面切换基础函数
 * ========================= */
static void show_only_page(lv_obj_t *page)
{
    lv_obj_add_flag(page_water, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(page_wifi, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(page_saved, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(page_info, LV_OBJ_FLAG_HIDDEN);

    lv_obj_clear_flag(page, LV_OBJ_FLAG_HIDDEN);
}

/* =========================
 * 设置单根 WiFi 柱子的颜色
 * ========================= */
static void wifi_bar_set_color(lv_obj_t *bar, lv_color_t color)
{
    lv_obj_set_style_bg_color(bar, color, 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
}

/* =========================
 * 更新顶部 WiFi 信号柱状图
 * ========================= */
static void wifi_bar_show_level(int level, bool show_x)
{
    lv_color_t on = lv_color_white();
    lv_color_t off = lv_color_hex(0x444444);

    wifi_bar_set_color(wifi_bar_1, level >= 1 ? on : off);
    wifi_bar_set_color(wifi_bar_2, level >= 2 ? on : off);
    wifi_bar_set_color(wifi_bar_3, level >= 3 ? on : off);
    wifi_bar_set_color(wifi_bar_4, level >= 4 ? on : off);

    if (show_x)
    {
        lv_obj_clear_flag(wifi_bar_x, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(wifi_bar_x, LV_OBJ_FLAG_HIDDEN);
    }
}

/* =========================
 * 设置顶部导航栏左侧标题
 * ========================= */
static void set_nav_title(const char *title)
{
    if (label_nav_title)
    {
        lv_label_set_text(label_nav_title, title ? title : "");
    }
}

/* =========================
 * 水质页 WiFi 按钮
 * ========================= */
static void wifi_btn_event(lv_event_t *e)
{
    (void)e;
    ui_pages_show_wifi();
}

/* =========================
 * Saved 按钮
 * ========================= */
static void saved_btn_event(lv_event_t *e)
{
    (void)e;
    ui_pages_show_saved();
}

/* =========================
 * Network Info 按钮
 * ========================= */
static void info_btn_event(lv_event_t *e)
{
    (void)e;
    ui_pages_show_network_info();
}

/* =========================
 * 返回主水质页
 * ========================= */
static void back_to_water_event(lv_event_t *e)
{
    (void)e;
    hide_keyboard();
    ui_pages_show_water();
}

/* =========================
 * 返回 WiFi 页
 * ========================= */
static void back_to_wifi_event(lv_event_t *e)
{
    (void)e;
    hide_keyboard();
    ui_pages_show_wifi();
}

/* =========================
 * 密码输入框获得焦点
 * 自动弹出键盘
 * ========================= */
static void pass_ta_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);

    if (code == LV_EVENT_FOCUSED)
    {
        lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
        lv_keyboard_set_textarea(kb, ta);
    }
}

/* =========================
 * 键盘事件回调
 * READY / CANCEL 时收起
 * ========================= */
static void kb_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL)
    {
        hide_keyboard();
    }
}

/* =========================
 * Scan 按钮回调
 * ========================= */
static void scan_btn_event(lv_event_t *e)
{
    (void)e;
    hide_keyboard();
    if (s_cbs.on_scan)
    {
        s_cbs.on_scan();
    }
}

/* =========================
 * Connect / Disconnect 按钮回调
 * ========================= */
static void connect_btn_event(lv_event_t *e)
{
    (void)e;
    hide_keyboard();

    if (s_wifi_connected)
    {
        if (s_cbs.on_disconnect)
        {
            s_cbs.on_disconnect();
        }
    }
    else
    {
        if (s_cbs.on_connect)
        {
            s_cbs.on_connect();
        }
    }
}

/* =========================
 * WiFi 下拉框选择变化
 * ========================= */
static void ssid_dropdown_event_cb(lv_event_t *e)
{
    (void)e;

    char ssid[WIFI_SSID_MAX_LEN + 1] = {0};
    lv_dropdown_get_selected_str(dd_ssid, ssid, sizeof(ssid));

    if (strncmp(ssid, "No WiFi", 7) == 0 || strncmp(ssid, "No WiFi Found", 13) == 0)
    {
        s_selected_ssid[0] = '\0';
        lv_label_set_text(label_wifi_tip, "No valid SSID");
        return;
    }

    if (strncmp(ssid, "<Hidden", 7) == 0)
    {
        s_selected_ssid[0] = '\0';
        lv_label_set_text(label_wifi_tip, "Hidden AP not supported now");
        return;
    }

    strncpy(s_selected_ssid, ssid, sizeof(s_selected_ssid) - 1);
    s_selected_ssid[sizeof(s_selected_ssid) - 1] = '\0';

    lv_label_set_text_fmt(label_wifi_tip, "Selected: %s", s_selected_ssid);
    lv_label_set_text(label_wifi_status, "Status: SSID selected");

    /* 再套一遍，确保弹出列表也是黑白风 */
    style_dropdown_bw(dd_ssid);
}

/* =========================
 * Saved 页点击 Use
 * ========================= */
static void saved_use_btn_event(lv_event_t *e)
{
    int index = (int)(intptr_t)lv_event_get_user_data(e);
    if (s_cbs.on_saved_use)
    {
        s_cbs.on_saved_use(index);
    }
}

/* =========================
 * Saved 页点击 Del
 * ========================= */
static void saved_del_btn_event(lv_event_t *e)
{
    int index = (int)(intptr_t)lv_event_get_user_data(e);
    if (s_cbs.on_saved_delete)
    {
        s_cbs.on_saved_delete(index);
    }
}

/* =========================
 * 创建整个 UI
 * ========================= */
void ui_pages_create(const ui_pages_callbacks_t *cbs)
{
    if (cbs)
    {
        s_cbs = *cbs;
    }

    lv_obj_t *scr = lv_scr_act();
    style_screen_black(scr);
    lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);

    /* =========================
     * 顶部状态栏
     * ========================= */
    status_bar = lv_obj_create(scr);
    lv_obj_set_size(status_bar, SCREEN_W, STATUS_BAR_H);
    lv_obj_align(status_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(status_bar, LV_OBJ_FLAG_FLOATING);
    lv_obj_add_flag(status_bar, LV_OBJ_FLAG_IGNORE_LAYOUT);
    style_panel_bw(status_bar);

    label_nav_title = lv_label_create(status_bar);
    lv_label_set_text(label_nav_title, "Water");
    lv_obj_align(label_nav_title, LV_ALIGN_LEFT_MID, 6, 0);
    style_label_white(label_nav_title);

    wifi_bar_1 = lv_obj_create(status_bar);
    lv_obj_set_size(wifi_bar_1, 3, 5);
    lv_obj_align(wifi_bar_1, LV_ALIGN_RIGHT_MID, -26, 4);
    lv_obj_set_style_radius(wifi_bar_1, 0, 0);
    lv_obj_set_style_border_width(wifi_bar_1, 0, 0);

    wifi_bar_2 = lv_obj_create(status_bar);
    lv_obj_set_size(wifi_bar_2, 3, 8);
    lv_obj_align(wifi_bar_2, LV_ALIGN_RIGHT_MID, -21, 2);
    lv_obj_set_style_radius(wifi_bar_2, 0, 0);
    lv_obj_set_style_border_width(wifi_bar_2, 0, 0);

    wifi_bar_3 = lv_obj_create(status_bar);
    lv_obj_set_size(wifi_bar_3, 3, 11);
    lv_obj_align(wifi_bar_3, LV_ALIGN_RIGHT_MID, -16, 1);
    lv_obj_set_style_radius(wifi_bar_3, 0, 0);
    lv_obj_set_style_border_width(wifi_bar_3, 0, 0);

    wifi_bar_4 = lv_obj_create(status_bar);
    lv_obj_set_size(wifi_bar_4, 3, 14);
    lv_obj_align(wifi_bar_4, LV_ALIGN_RIGHT_MID, -11, 0);
    lv_obj_set_style_radius(wifi_bar_4, 0, 0);
    lv_obj_set_style_border_width(wifi_bar_4, 0, 0);

    wifi_bar_x = lv_label_create(status_bar);
    lv_label_set_text(wifi_bar_x, "x");
    lv_obj_align(wifi_bar_x, LV_ALIGN_RIGHT_MID, -2, 0);
    lv_obj_add_flag(wifi_bar_x, LV_OBJ_FLAG_HIDDEN);
    style_label_white(wifi_bar_x);

    wifi_bar_show_level(0, false);

    /* =========================
     * 水质页
     * ========================= */
    page_water = lv_obj_create(scr);
    lv_obj_set_size(page_water, SCREEN_W, PAGE_H);
    lv_obj_align(page_water, LV_ALIGN_TOP_MID, 0, STATUS_BAR_H);
    lv_obj_clear_flag(page_water, LV_OBJ_FLAG_SCROLLABLE);
    style_screen_black(page_water);

    label_ph = lv_label_create(page_water);
    lv_label_set_text(label_ph, "Ph: 7.5");
    lv_obj_align(label_ph, LV_ALIGN_CENTER, 0, -140);
    style_label_white(label_ph);

    label_ph_level = lv_label_create(page_water);
    lv_label_set_text(label_ph_level, "Ph Level: Neutral");
    lv_obj_align(label_ph_level, LV_ALIGN_CENTER, 0, -120);
    style_label_white(label_ph_level);

    label_turb = lv_label_create(page_water);
    lv_label_set_text(label_turb, "Turbidity: 0");
    lv_obj_align(label_turb, LV_ALIGN_CENTER, 0, -100);
    style_label_white(label_turb);

    label_level = lv_label_create(page_water);
    lv_label_set_text(label_level, "Turbidity Level: Clean");
    lv_obj_align(label_level, LV_ALIGN_CENTER, 0, -70);
    style_label_white(label_level);

    label_tds = lv_label_create(page_water);
    lv_label_set_text(label_tds, "TDS: 0.0 ppm");
    lv_obj_align(label_tds, LV_ALIGN_CENTER, 0, -30);
    style_label_white(label_tds);

    label_tds_level = lv_label_create(page_water);
    lv_label_set_text(label_tds_level, "TDS Level: Excellent");
    lv_obj_align(label_tds_level, LV_ALIGN_CENTER, 0, 0);
    style_label_white(label_tds_level);

    label_temp = lv_label_create(page_water);
    lv_label_set_text(label_temp, "Temp: 0.0 C");
    lv_obj_align(label_temp, LV_ALIGN_CENTER, 0, 40);
    style_label_white(label_temp);

    label_temp_level = lv_label_create(page_water);
    lv_label_set_text(label_temp_level, "Temp Level: Normal");
    lv_obj_align(label_temp_level, LV_ALIGN_CENTER, 0, 70);
    style_label_white(label_temp_level);

    lv_obj_t *wifi_btn = lv_btn_create(page_water);
    lv_obj_set_size(wifi_btn, 80, 38);
    lv_obj_align(wifi_btn, LV_ALIGN_BOTTOM_LEFT, 25, -10);
    lv_obj_add_event_cb(wifi_btn, wifi_btn_event, LV_EVENT_CLICKED, NULL);
    style_btn_bw(wifi_btn);
    lv_obj_t *wifi_label = lv_label_create(wifi_btn);
    lv_label_set_text(wifi_label, "WiFi");
    lv_obj_center(wifi_label);
    style_label_white(wifi_label);

    lv_obj_t *saved_btn = lv_btn_create(page_water);
    lv_obj_set_size(saved_btn, 80, 38);
    lv_obj_align(saved_btn, LV_ALIGN_BOTTOM_RIGHT, -25, -10);
    lv_obj_add_event_cb(saved_btn, saved_btn_event, LV_EVENT_CLICKED, NULL);
    style_btn_bw(saved_btn);
    lv_obj_t *saved_label = lv_label_create(saved_btn);
    lv_label_set_text(saved_label, "Saved");
    lv_obj_center(saved_label);
    style_label_white(saved_label);

    /* =========================
     * WiFi 连接页
     * ========================= */
    page_wifi = lv_obj_create(scr);
    lv_obj_set_size(page_wifi, SCREEN_W, PAGE_H);
    lv_obj_align(page_wifi, LV_ALIGN_TOP_MID, 0, STATUS_BAR_H);
    lv_obj_clear_flag(page_wifi, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(page_wifi, LV_OBJ_FLAG_HIDDEN);
    style_screen_black(page_wifi);

    label_wifi_tip = lv_label_create(page_wifi);
    lv_label_set_text(label_wifi_tip, "Scan and select SSID");
    lv_obj_align(label_wifi_tip, LV_ALIGN_TOP_MID, 0, 0);
    style_label_white(label_wifi_tip);

    lv_obj_t *ssid_label = lv_label_create(page_wifi);
    lv_label_set_text(ssid_label, "WiFi List");
    lv_obj_align(ssid_label, LV_ALIGN_TOP_LEFT, 16, 20);
    style_label_white(ssid_label);

    dd_ssid = lv_dropdown_create(page_wifi);
    lv_obj_set_size(dd_ssid, 208, 36);
    lv_obj_align(dd_ssid, LV_ALIGN_TOP_MID, 0, 40);
    lv_dropdown_set_options(dd_ssid, "No WiFi");
    lv_obj_add_event_cb(dd_ssid, ssid_dropdown_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    style_dropdown_bw(dd_ssid);

    lv_obj_t *pass_label = lv_label_create(page_wifi);
    lv_label_set_text(pass_label, "Password");
    lv_obj_align(pass_label, LV_ALIGN_TOP_LEFT, 16, 84);
    style_label_white(pass_label);

    ta_pass = lv_textarea_create(page_wifi);
    lv_obj_set_size(ta_pass, 208, 36);
    lv_obj_align(ta_pass, LV_ALIGN_TOP_MID, 0, 102);
    lv_textarea_set_one_line(ta_pass, true);
    lv_textarea_set_password_mode(ta_pass, true);
    lv_textarea_set_placeholder_text(ta_pass, "Input password");
    lv_obj_add_event_cb(ta_pass, pass_ta_event_cb, LV_EVENT_ALL, NULL);
    style_input_bw(ta_pass);

    label_wifi_status = lv_label_create(page_wifi);
    lv_label_set_text(label_wifi_status, "Status: Idle");
    lv_obj_align(label_wifi_status, LV_ALIGN_TOP_LEFT, 16, 150);
    style_label_white(label_wifi_status);

    btn_info = lv_btn_create(page_wifi);
    lv_obj_set_size(btn_info, 110, 28);
    lv_obj_align(btn_info, LV_ALIGN_TOP_MID, 0, 170);
    lv_obj_add_event_cb(btn_info, info_btn_event, LV_EVENT_CLICKED, NULL);
    lv_obj_add_flag(btn_info, LV_OBJ_FLAG_HIDDEN);
    style_btn_bw(btn_info);

    lv_obj_t *info_label = lv_label_create(btn_info);
    lv_label_set_text(info_label, "Network Info");
    lv_obj_center(info_label);
    style_label_white(info_label);

    lv_obj_t *scan_btn = lv_btn_create(page_wifi);
    lv_obj_set_size(scan_btn, 88, 32);
    lv_obj_align(scan_btn, LV_ALIGN_BOTTOM_LEFT, 18, -34);
    lv_obj_add_event_cb(scan_btn, scan_btn_event, LV_EVENT_CLICKED, NULL);
    style_btn_bw(scan_btn);
    lv_obj_t *scan_label = lv_label_create(scan_btn);
    lv_label_set_text(scan_label, "Scan");
    lv_obj_center(scan_label);
    style_label_white(scan_label);

    lv_obj_t *saved_btn2 = lv_btn_create(page_wifi);
    lv_obj_set_size(saved_btn2, 88, 32);
    lv_obj_align(saved_btn2, LV_ALIGN_BOTTOM_RIGHT, -18, -34);
    lv_obj_add_event_cb(saved_btn2, saved_btn_event, LV_EVENT_CLICKED, NULL);
    style_btn_bw(saved_btn2);
    lv_obj_t *saved_label2 = lv_label_create(saved_btn2);
    lv_label_set_text(saved_label2, "Saved");
    lv_obj_center(saved_label2);
    style_label_white(saved_label2);

    btn_connect = lv_btn_create(page_wifi);
    lv_obj_set_size(btn_connect, 88, 32);
    lv_obj_align(btn_connect, LV_ALIGN_BOTTOM_LEFT, 18, 6);
    lv_obj_add_event_cb(btn_connect, connect_btn_event, LV_EVENT_CLICKED, NULL);
    style_btn_bw(btn_connect);
    label_connect_text = lv_label_create(btn_connect);
    lv_label_set_text(label_connect_text, "Connect");
    lv_obj_center(label_connect_text);
    style_label_white(label_connect_text);

    lv_obj_t *back_btn = lv_btn_create(page_wifi);
    lv_obj_set_size(back_btn, 88, 32);
    lv_obj_align(back_btn, LV_ALIGN_BOTTOM_RIGHT, -18, 6);
    lv_obj_add_event_cb(back_btn, back_to_water_event, LV_EVENT_CLICKED, NULL);
    style_btn_bw(back_btn);
    lv_obj_t *back_label = lv_label_create(back_btn);
    lv_label_set_text(back_label, "Back");
    lv_obj_center(back_label);
    style_label_white(back_label);

    kb = lv_keyboard_create(page_wifi);
    lv_obj_set_size(kb, SCREEN_W, 110);
    lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 10);
    lv_obj_add_event_cb(kb, kb_event_cb, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    style_keyboard_bw(kb);

    /* =========================
     * Saved WiFi 页
     * ========================= */
    page_saved = lv_obj_create(scr);
    lv_obj_set_size(page_saved, SCREEN_W, PAGE_H);
    lv_obj_align(page_saved, LV_ALIGN_TOP_MID, 0, STATUS_BAR_H);
    lv_obj_clear_flag(page_saved, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(page_saved, LV_OBJ_FLAG_HIDDEN);
    style_screen_black(page_saved);

    saved_list = lv_list_create(page_saved);
    lv_obj_set_size(saved_list, 220, 240);
    lv_obj_align(saved_list, LV_ALIGN_TOP_MID, 0, 12);
    style_panel_bw(saved_list);

    lv_obj_t *saved_back_btn = lv_btn_create(page_saved);
    lv_obj_set_size(saved_back_btn, 80, 30);
    lv_obj_align(saved_back_btn, LV_ALIGN_BOTTOM_MID, 0, -4);
    lv_obj_add_event_cb(saved_back_btn, back_to_wifi_event, LV_EVENT_CLICKED, NULL);
    style_btn_bw(saved_back_btn);
    lv_obj_t *saved_back_label = lv_label_create(saved_back_btn);
    lv_label_set_text(saved_back_label, "Back");
    lv_obj_center(saved_back_label);
    style_label_white(saved_back_label);

    /* =========================
     * 网络信息页
     * ========================= */
    page_info = lv_obj_create(scr);
    lv_obj_set_size(page_info, SCREEN_W, PAGE_H);
    lv_obj_align(page_info, LV_ALIGN_TOP_MID, 0, STATUS_BAR_H);
    lv_obj_clear_flag(page_info, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(page_info, LV_OBJ_FLAG_HIDDEN);
    style_screen_black(page_info);

    label_info_status = lv_label_create(page_info);
    lv_label_set_text(label_info_status, "Status: -");
    lv_obj_align(label_info_status, LV_ALIGN_TOP_LEFT, 18, 30);
    style_label_white(label_info_status);

    label_info_ssid = lv_label_create(page_info);
    lv_label_set_text(label_info_ssid, "SSID: -");
    lv_obj_align(label_info_ssid, LV_ALIGN_TOP_LEFT, 18, 65);
    style_label_white(label_info_ssid);

    label_info_pass = lv_label_create(page_info);
    lv_label_set_text(label_info_pass, "PASS: -");
    lv_obj_align(label_info_pass, LV_ALIGN_TOP_LEFT, 18, 100);
    style_label_white(label_info_pass);

    label_info_ip = lv_label_create(page_info);
    lv_label_set_text(label_info_ip, "IP: -");
    lv_obj_align(label_info_ip, LV_ALIGN_TOP_LEFT, 18, 135);
    style_label_white(label_info_ip);

    lv_obj_t *info_back_btn = lv_btn_create(page_info);
    lv_obj_set_size(info_back_btn, 88, 32);
    lv_obj_align(info_back_btn, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_add_event_cb(info_back_btn, back_to_wifi_event, LV_EVENT_CLICKED, NULL);
    style_btn_bw(info_back_btn);
    lv_obj_t *info_back_label = lv_label_create(info_back_btn);
    lv_label_set_text(info_back_label, "Back");
    lv_obj_center(info_back_label);
    style_label_white(info_back_label);

    /* =========================
     * 触摸调试点
     * ========================= */
    touch_dot = lv_obj_create(scr);
    lv_obj_set_size(touch_dot, 8, 8);
    lv_obj_set_style_bg_color(touch_dot, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_radius(touch_dot, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(touch_dot, 0, 0);
    lv_obj_add_flag(touch_dot, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(touch_dot, LV_OBJ_FLAG_FLOATING);
    lv_obj_add_flag(touch_dot, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_clear_flag(touch_dot, LV_OBJ_FLAG_CLICKABLE);
}

/* =========================
 * 显示水质主页面
 * ========================= */
void ui_pages_show_water(void)
{
    hide_keyboard();
    show_only_page(page_water);
    set_nav_title("Water");
}

/* =========================
 * 显示 WiFi 连接页面
 * ========================= */
void ui_pages_show_wifi(void)
{
    hide_keyboard();
    show_only_page(page_wifi);
    set_nav_title("WiFi");

    if (s_selected_ssid[0])
    {
        lv_label_set_text_fmt(label_wifi_tip, "Selected: %s", s_selected_ssid);
    }
    else
    {
        lv_label_set_text(label_wifi_tip, "Scan and select SSID");
    }

    style_dropdown_bw(dd_ssid);
}

/* =========================
 * 显示已保存 WiFi 页面
 * ========================= */
void ui_pages_show_saved(void)
{
    hide_keyboard();
    show_only_page(page_saved);
    set_nav_title("Saved WiFi");
}

/* =========================
 * 显示网络信息页面
 * ========================= */
void ui_pages_show_network_info(void)
{
    hide_keyboard();
    show_only_page(page_info);
    set_nav_title("Network Info");
}

/* =========================
 * 更新水质页数据
 * ========================= */
void ui_pages_update_ph(float ph, const char *level)
{
    char buf[64];

    snprintf(buf, sizeof(buf), "Ph: %.1f", ph);
    lv_label_set_text(label_ph, buf);

    snprintf(buf, sizeof(buf), "Ph Level: %s", level ? level : "Unknown");
    lv_label_set_text(label_ph_level, buf);
}

void ui_pages_update_temp(float temp_c, const char *level)
{
    char buf[64];

    snprintf(buf, sizeof(buf), "Temp: %.1f C", temp_c);
    lv_label_set_text(label_temp, buf);

    snprintf(buf, sizeof(buf), "Temp Level: %s", level ? level : "Unknown");
    lv_label_set_text(label_temp_level, buf);
}

void ui_pages_update_tds(float ppm, const char *level)
{
    char buf[64];

    snprintf(buf, sizeof(buf), "TDS: %.1f ppm", ppm);
    lv_label_set_text(label_tds, buf);

    snprintf(buf, sizeof(buf), "TDS Level: %s", level ? level : "Unknown");
    lv_label_set_text(label_tds_level, buf);
}

void ui_pages_update_turbidity(int turbidity, const char *level)
{
    char buf[64];

    snprintf(buf, sizeof(buf), "Turbidity: %d", turbidity);
    lv_label_set_text(label_turb, buf);

    snprintf(buf, sizeof(buf), "Turbidity Level: %s", level ? level : "Unknown");
    lv_label_set_text(label_level, buf);
}

/* =========================
 * 更新 WiFi 页状态文字
 * ========================= */
void ui_pages_set_wifi_status(const char *status)
{
    if (label_wifi_status)
    {
        lv_label_set_text_fmt(label_wifi_status, "Status: %s", status ? status : "Unknown");
    }
}

/* =========================
 * WiFi 连接页不显示 IP
 * ========================= */
void ui_pages_set_wifi_ip(const char *ip)
{
    (void)ip;
}

/* =========================
 * 更新 WiFi 页顶部提示
 * ========================= */
void ui_pages_set_wifi_tip(const char *tip)
{
    if (label_wifi_tip)
    {
        lv_label_set_text(label_wifi_tip, tip ? tip : "");
    }
}

/* =========================
 * 根据是否连接成功，更新：
 * - Connect 按钮文字
 * - Network Info 按钮显示/隐藏
 * ========================= */
void ui_pages_set_wifi_connected(bool connected)
{
    s_wifi_connected = connected;

    if (label_connect_text)
    {
        lv_label_set_text(label_connect_text, connected ? "Disconnect" : "Connect");
    }

    if (btn_info)
    {
        if (connected)
        {
            lv_obj_clear_flag(btn_info, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_add_flag(btn_info, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

/* =========================
 * 更新网络信息页内容
 * ========================= */
void ui_pages_set_network_info(const char *status, const char *ssid, const char *pass, const char *ip)
{
    if (label_info_status)
    {
        lv_label_set_text_fmt(label_info_status, "Status: %s", status ? status : "-");
    }

    if (label_info_ssid)
    {
        lv_label_set_text_fmt(label_info_ssid, "SSID: %s", (ssid && ssid[0]) ? ssid : "-");
    }

    if (label_info_pass)
    {
        lv_label_set_text_fmt(label_info_pass, "PASS: %s", (pass && pass[0]) ? pass : "-");
    }

    if (label_info_ip)
    {
        lv_label_set_text_fmt(label_info_ip, "IP: %s", (ip && ip[0]) ? ip : "-");
    }
}

/* =========================
 * 更新扫描 WiFi 列表到下拉框
 * ========================= */
void ui_pages_set_wifi_list(const wifi_ap_record_t *records, uint16_t count)
{
    if (!records || count == 0)
    {
        lv_dropdown_set_options(dd_ssid, "No WiFi Found");
        lv_dropdown_set_selected(dd_ssid, 0);
        s_selected_ssid[0] = '\0';
        lv_label_set_text(label_wifi_tip, "No hotspot found");
        style_dropdown_bw(dd_ssid);
        return;
    }

    size_t buf_size = (WIFI_SSID_MAX_LEN + 2) * count + 1;
    char *options = (char *)calloc(1, buf_size);
    if (!options)
    {
        lv_label_set_text(label_wifi_status, "Status: No mem for list");
        return;
    }

    for (int i = 0; i < count; i++)
    {
        const char *ssid = (const char *)records[i].ssid;

        if (ssid[0] == '\0')
        {
            char hidden_name[32];
            snprintf(hidden_name, sizeof(hidden_name), "<Hidden %d>", i + 1);
            strncat(options, hidden_name, buf_size - strlen(options) - 1);
        }
        else
        {
            strncat(options, ssid, buf_size - strlen(options) - 1);
        }

        if (i != count - 1)
        {
            strncat(options, "\n", buf_size - strlen(options) - 1);
        }
    }

    lv_dropdown_set_options(dd_ssid, options);
    lv_dropdown_set_selected(dd_ssid, 0);
    style_dropdown_bw(dd_ssid);

    if (((const char *)records[0].ssid)[0] != '\0')
    {
        strncpy(s_selected_ssid, (const char *)records[0].ssid, sizeof(s_selected_ssid) - 1);
        s_selected_ssid[sizeof(s_selected_ssid) - 1] = '\0';
        lv_label_set_text_fmt(label_wifi_tip, "Selected: %s", s_selected_ssid);
    }
    else
    {
        s_selected_ssid[0] = '\0';
        lv_label_set_text(label_wifi_tip, "First AP hidden, pick one");
    }

    free(options);
}

/* =========================
 * 刷新 Saved WiFi 页面
 * ========================= */
void ui_pages_refresh_saved_wifi(const wifi_book_t *book)
{
    if (!saved_list)
    {
        return;
    }

    lv_obj_clean(saved_list);

    if (!book)
    {
        return;
    }

    bool has_item = false;

    for (int i = 0; i < WIFI_SLOT_COUNT; i++)
    {
        if (!book->slots[i].valid)
        {
            continue;
        }

        has_item = true;

        lv_obj_t *row = lv_obj_create(saved_list);
        lv_obj_set_width(row, lv_pct(100));
        lv_obj_set_height(row, 130);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_pad_all(row, 4, 0);
        style_panel_bw(row);

        lv_obj_t *label_ssid = lv_label_create(row);
        lv_label_set_text_fmt(label_ssid, "SSID: %s", book->slots[i].ssid);
        lv_obj_align(label_ssid, LV_ALIGN_TOP_LEFT, 2, 0);
        style_label_white(label_ssid);

        lv_obj_t *label_pass = lv_label_create(row);
        lv_label_set_text_fmt(label_pass, "PASS: %s",
                              book->slots[i].pass[0] ? book->slots[i].pass : "(empty)");
        lv_obj_align(label_pass, LV_ALIGN_TOP_LEFT, 2, 20);
        style_label_white(label_pass);

        lv_obj_t *label_ip = lv_label_create(row);
        lv_label_set_text_fmt(label_ip, "IP: %s",
                              book->slots[i].last_ip[0] ? book->slots[i].last_ip : "-");
        lv_obj_align(label_ip, LV_ALIGN_TOP_LEFT, 2, 40);
        style_label_white(label_ip);

        lv_obj_t *label_state = lv_label_create(row);
        lv_label_set_text_fmt(label_state, "LAST: %s",
                              book->slots[i].last_ok ? "OK" : "FAIL");
        lv_obj_align(label_state, LV_ALIGN_TOP_LEFT, 2, 60);
        style_label_white(label_state);

        lv_obj_t *use_btn = lv_btn_create(row);
        lv_obj_set_size(use_btn, 56, 28);
        lv_obj_align(use_btn, LV_ALIGN_BOTTOM_LEFT, 0, 0);
        lv_obj_add_event_cb(use_btn, saved_use_btn_event, LV_EVENT_CLICKED, (void *)(intptr_t)i);
        style_btn_bw(use_btn);
        lv_obj_t *use_label = lv_label_create(use_btn);
        lv_label_set_text(use_label, "Use");
        lv_obj_center(use_label);
        style_label_white(use_label);

        lv_obj_t *del_btn = lv_btn_create(row);
        lv_obj_set_size(del_btn, 56, 28);
        lv_obj_align(del_btn, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
        lv_obj_add_event_cb(del_btn, saved_del_btn_event, LV_EVENT_CLICKED, (void *)(intptr_t)i);
        style_btn_bw(del_btn);
        lv_obj_t *del_label = lv_label_create(del_btn);
        lv_label_set_text(del_label, "Del");
        lv_obj_center(del_label);
        style_label_white(del_label);
    }

    if (!has_item)
    {
        lv_obj_t *label_empty = lv_label_create(saved_list);
        lv_label_set_text(label_empty, "No saved WiFi");
        style_label_white(label_empty);
    }
}

/* =========================
 * 获取当前扫描页选中的 SSID
 * 给 main.c 调用
 * ========================= */
void ui_pages_get_selected_ssid(char *buf, size_t len)
{
    if (!buf || len == 0)
    {
        return;
    }

    strncpy(buf, s_selected_ssid, len - 1);
    buf[len - 1] = '\0';
}

/* =========================
 * 获取当前输入的 WiFi 密码
 * 给 main.c 调用
 * ========================= */
void ui_pages_get_password(char *buf, size_t len)
{
    if (!buf || len == 0)
    {
        return;
    }

    const char *pass = lv_textarea_get_text(ta_pass);
    if (!pass)
    {
        buf[0] = '\0';
        return;
    }

    strncpy(buf, pass, len - 1);
    buf[len - 1] = '\0';
}

/* =========================
 * 更新触摸调试点位置
 * ========================= */
void ui_pages_set_touch_point(bool pressed, uint16_t x, uint16_t y)
{
    if (!touch_dot)
    {
        return;
    }

    if (pressed)
    {
        lv_obj_clear_flag(touch_dot, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(touch_dot, x - 4, y - 4);
    }
    else
    {
        lv_obj_add_flag(touch_dot, LV_OBJ_FLAG_HIDDEN);
    }
}

/* =========================
 * 更新顶部 WiFi 柱状图
 * ========================= */
void ui_pages_set_nav_wifi(const char *status, const char *ssid, const char *ip)
{
    (void)ssid;
    (void)ip;

    if (!status)
    {
        wifi_bar_show_level(0, false);
        return;
    }

    if (strstr(status, "Got IP") || strstr(status, "Connected"))
    {
        wifi_bar_show_level(4, false);
    }
    else if (strstr(status, "Connecting") || strstr(status, "waiting"))
    {
        wifi_bar_show_level(2, false);
    }
    else if (strstr(status, "Scanning") || strstr(status, "Scan"))
    {
        wifi_bar_show_level(1, false);
    }
    else if (strstr(status, "Disconnected") || strstr(status, "failed") || strstr(status, "Fail"))
    {
        wifi_bar_show_level(0, true);
    }
    else
    {
        wifi_bar_show_level(0, false);
    }
}