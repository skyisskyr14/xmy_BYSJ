#pragma once

#include "esp_adc/adc_oneshot.h"

void turbidity_init(void);

int turbidity_read(void);

int turbidity_read_avg(void);

const char* turbidity_level(int value);

adc_oneshot_unit_handle_t turbidity_get_adc_handle(void);