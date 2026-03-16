#ifndef TDS_SENSOR_H
#define TDS_SENSOR_H

void tds_sensor_init(void);
int tds_sensor_read_raw(void);
float tds_sensor_read_voltage(void);
float tds_sensor_read_ppm(void);
const char *tds_sensor_level(float ppm);

#endif