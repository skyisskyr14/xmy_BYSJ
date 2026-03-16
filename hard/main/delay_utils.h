#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 毫秒延时（基于 FreeRTOS tick，最推荐）
 *
 * 注意：
 * - 精度取决于 tick 周期（CONFIG_FREERTOS_HZ）
 * - 适用于任务中延时、UI刷新节拍、轮询等
 */
void delay_ms(uint32_t ms);

/**
 * @brief 微秒延时（忙等，精度更高但会占用 CPU）
 *
 * 适用：
 * - SPI 设备复位时序、短等待、bitbang 等
 * 不适用：
 * - 长时间延时（会一直占 CPU）
 */
void delay_us(uint32_t us);

/**
 * @brief 稳定周期延时：每次都“睡到下一个周期点”，不会越跑越漂移
 *
 * 用法：
 *   uint32_t next = 0;
 *   while (1) {
 *      // do work
 *      delay_until_ms(&next, 50); // 固定 50ms 周期
 *   }
 */
void delay_until_ms(uint32_t *next_wake_tick, uint32_t period_ms);

#ifdef __cplusplus
}
#endif