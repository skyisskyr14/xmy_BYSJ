#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <stdbool.h>

void temp_sensor_init(void);
bool temp_sensor_read(float *temperature);
const char *temp_sensor_level(float temp_c);

#endif