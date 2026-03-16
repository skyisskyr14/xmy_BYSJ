#ifndef PH_SENSOR_H
#define PH_SENSOR_H

void ph_sensor_init(void);
int ph_sensor_read_raw(void);
float ph_sensor_read_voltage(void);
float ph_sensor_read_value(void);
const char *ph_sensor_level(float ph);

#endif