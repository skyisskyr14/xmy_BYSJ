#include "tds_sensor.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

#include "turbidity.h"

// #define TDS_ADC_UNIT       ADC_UNIT_1
#define TDS_ADC_CHANNEL    ADC_CHANNEL_0
#define TDS_ADC_ATTEN      ADC_ATTEN_DB_12
#define TDS_ADC_BITWIDTH   ADC_BITWIDTH_DEFAULT

static const char *TAG = "tds_sensor";
static adc_oneshot_unit_handle_t s_tds_adc_handle = NULL;

void tds_sensor_init(void)
{
    s_tds_adc_handle = turbidity_get_adc_handle();

    adc_oneshot_chan_cfg_t chan_config = {
        .atten = TDS_ADC_ATTEN,
        .bitwidth = TDS_ADC_BITWIDTH,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_tds_adc_handle, TDS_ADC_CHANNEL, &chan_config));

    ESP_LOGI(TAG, "tds init ok");
}

int tds_sensor_read_raw(void)
{
    if (!s_tds_adc_handle) {
        return 0;
    }

    int raw = 0;
    if (adc_oneshot_read(s_tds_adc_handle, TDS_ADC_CHANNEL, &raw) != ESP_OK) {
        return 0;
    }

    return raw;
}

float tds_sensor_read_voltage(void)
{
    int raw = tds_sensor_read_raw();
    return (raw / 4095.0f) * 3.3f;
}

float tds_sensor_read_ppm(void)
{
    float voltage = tds_sensor_read_voltage();
    float ppm = voltage * 500.0f;

    if (ppm < 0) {
        ppm = 0;
    }

    return ppm;
}

const char *tds_sensor_level(float ppm)
{
    if (ppm < 300.0f) {
        return "Excellent";
    } else if (ppm < 600.0f) {
        return "Good";
    } else if (ppm < 900.0f) {
        return "Normal";
    } else {
        return "High";
    }
}