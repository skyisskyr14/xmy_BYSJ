#include "sensor_service.h"
#include "turbidity.h"
#include "tds_sensor.h"
#include "temp_sensor.h"
#include "ph_sensor.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static volatile int s_turbidity = 0;
static volatile float s_tds_ppm = 0.0f;
static volatile float s_temp_c = 0.0f;
static volatile float s_ph = 7.0f;

static void sensor_task(void *arg)
{
    (void)arg;

    while (1)
    {
        s_turbidity = turbidity_read_avg();
        vTaskDelay(pdMS_TO_TICKS(50));
        s_tds_ppm = tds_sensor_read_ppm();
        vTaskDelay(pdMS_TO_TICKS(50));
        s_ph = ph_sensor_read_value();
        vTaskDelay(pdMS_TO_TICKS(50));
        float temp = 0.0f;
        if (temp_sensor_read(&temp))
        {
            s_temp_c = temp;
        }

        printf("%f \n",s_ph);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void sensor_service_init(void)
{
    turbidity_init();
    vTaskDelay(pdMS_TO_TICKS(50));
    tds_sensor_init();
    vTaskDelay(pdMS_TO_TICKS(50));
    ph_sensor_init();
    vTaskDelay(pdMS_TO_TICKS(50));
    temp_sensor_init();

    xTaskCreate(sensor_task, "sensor_task", 4096, NULL, 5, NULL);
}

int sensor_service_get_turbidity(void)
{
    return s_turbidity;
}

float sensor_service_get_tds(void)
{
    return s_tds_ppm;
}

float sensor_service_get_temp(void)
{
    return s_temp_c;
}

float sensor_service_get_ph(void){
    return s_ph;
}