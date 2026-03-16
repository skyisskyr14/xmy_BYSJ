#include "delay_utils.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/**
 * @brief 毫秒延时（基于 FreeRTOS）
 */
void delay_ms(uint32_t ms)
{
    if (ms == 0) {
        taskYIELD(); // 让出 CPU
        return;
    }
    vTaskDelay(pdMS_TO_TICKS(ms));
}

/**
 * @brief 微秒延时（忙等）
 */
void delay_us(uint32_t us)
{
    if (us == 0) return;
    esp_rom_delay_us(us);
}

/**
 * @brief 稳定周期延时：避免循环抖动、避免累计漂移
 */
void delay_until_ms(uint32_t *next_wake_tick, uint32_t period_ms)
{
    if (!next_wake_tick || period_ms == 0) {
        return;
    }

    // 第一次调用：初始化 next_wake_tick 为当前 tick
    if (*next_wake_tick == 0) {
        *next_wake_tick = xTaskGetTickCount();
    }

    // vTaskDelayUntil 需要 TickType_t
    TickType_t last = (TickType_t)(*next_wake_tick);
    vTaskDelayUntil(&last, pdMS_TO_TICKS(period_ms));

    // 把更新后的 tick 写回去，保证下次继续稳定节拍
    *next_wake_tick = (uint32_t)last;
}