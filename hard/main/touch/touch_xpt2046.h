#ifndef TOUCH_XPT2046_H
#define TOUCH_XPT2046_H

#include <stdint.h>
#include <stdbool.h>
#include "driver/spi_master.h"

void touch_init(spi_host_device_t host, int cs, int irq);

bool touch_read(uint16_t *x, uint16_t *y);

#endif