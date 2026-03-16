#include "lvgl.h"

lv_obj_t *label_value;
lv_obj_t *label_status;

void ui_init()
{
    lv_obj_t *scr = lv_scr_act();

    lv_obj_set_style_bg_color(scr, lv_color_black(), 0);

    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "Water Monitor");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    label_value = lv_label_create(scr);
    lv_label_set_text(label_value, "Turbidity: 0");
    lv_obj_align(label_value, LV_ALIGN_CENTER, 0, -20);

    label_status = lv_label_create(scr);
    lv_label_set_text(label_status, "Status: Clean");
    lv_obj_align(label_status, LV_ALIGN_CENTER, 0, 20);
}

void ui_update(int turb, const char *level)
{
    char buf[32];

    sprintf(buf, "Turbidity: %d", turb);
    lv_label_set_text(label_value, buf);

    sprintf(buf, "Status: %s", level);
    lv_label_set_text(label_status, buf);
}