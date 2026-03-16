#include "lvgl.h"
#include "esp_timer.h"

static void lv_tick_task(void *arg)
{
    lv_tick_inc(1);
}

void lvgl_port_init()
{
    lv_init();

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "lvgl_tick"
    };

    esp_timer_handle_t periodic_timer;

    esp_timer_create(&periodic_timer_args, &periodic_timer);
    esp_timer_start_periodic(periodic_timer, 1000);
}