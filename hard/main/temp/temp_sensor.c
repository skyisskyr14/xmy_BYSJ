#include "temp_sensor.h"

#include "esp_log.h"
#include "onewire_bus.h"
#include "ds18b20.h"

#define TEMP_GPIO 20

static const char *TAG = "temp_sensor";

static onewire_bus_handle_t s_bus = NULL;
static ds18b20_device_handle_t s_ds18b20 = NULL;

void temp_sensor_init(void)
{
    onewire_bus_config_t bus_config = {
        .bus_gpio_num = TEMP_GPIO,
        .flags = {
            .en_pull_up = true,
        }
    };

    onewire_bus_rmt_config_t rmt_config = {
        .max_rx_bytes = 10,
    };

    ESP_ERROR_CHECK(onewire_new_bus_rmt(&bus_config, &rmt_config, &s_bus));

    onewire_device_iter_handle_t iter = NULL;
    onewire_device_t next_onewire_device;
    esp_err_t search_result = ESP_OK;

    ESP_ERROR_CHECK(onewire_new_device_iter(s_bus, &iter));
    ESP_LOGI(TAG, "Searching DS18B20 on GPIO %d...", TEMP_GPIO);

    while ((search_result = onewire_device_iter_get_next(iter, &next_onewire_device)) == ESP_OK) {
        ds18b20_config_t ds_cfg = {};

        if (ds18b20_new_device_from_enumeration(&next_onewire_device, &ds_cfg, &s_ds18b20) == ESP_OK) {
            ESP_LOGI(TAG, "DS18B20 found");
            break;
        }
    }

    ESP_ERROR_CHECK(onewire_del_device_iter(iter));

    if (!s_ds18b20) {
        ESP_LOGE(TAG, "No DS18B20 found");
    }
}

bool temp_sensor_read(float *temperature)
{
    if (!temperature) {
        return false;
    }

    if (!s_ds18b20 || !s_bus) {
        return false;
    }

    ESP_ERROR_CHECK(ds18b20_trigger_temperature_conversion_for_all(s_bus));

    float temp = 0.0f;
    esp_err_t err = ds18b20_get_temperature(s_ds18b20, &temp);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "ds18b20 read failed: %d", err);
        return false;
    }

    *temperature = temp;
    return true;
}

const char *temp_sensor_level(float temp_c)
{
    if (temp_c < 15.0f) {
        return "Low";
    } else if (temp_c < 30.0f) {
        return "Normal";
    } else if (temp_c < 40.0f) {
        return "Warm";
    } else {
        return "High";
    }
}