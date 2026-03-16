#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 传入 UTF-8 指针（指向一个汉字的3字节），返回 16x16 字模（32字节），找不到返回 NULL
const uint8_t* hz16x16_get_utf8(const char *hz);

#ifdef __cplusplus
}
#endif