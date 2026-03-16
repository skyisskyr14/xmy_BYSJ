#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

void sensor_service_init(void);
int sensor_service_get_turbidity(void);
float sensor_service_get_tds(void);
float sensor_service_get_temp(void);
float sensor_service_get_ph(void);

#endif