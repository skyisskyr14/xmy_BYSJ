#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 返回 8x16 字符字模（16字节），找不到返回 NULL
const uint8_t* font8x16_get(char c);

#ifdef __cplusplus
}
#endif