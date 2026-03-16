#include "ph_sensor.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

#include "turbidity.h"

#define PH_GPIO          ADC_CHANNEL_1
#define PH_ADC_ATTEN     ADC_ATTEN_DB_12
#define PH_ADC_BITWIDTH  ADC_BITWIDTH_DEFAULT

static const char *TAG = "ph_sensor";

static adc_oneshot_unit_handle_t s_ph_adc_handle = NULL;
static adc_channel_t s_ph_channel;

void ph_sensor_init(void)
{
    adc_unit_t unit = ADC_UNIT_1;

    esp_err_t err = adc_oneshot_io_to_channel(PH_GPIO, &unit, &s_ph_channel);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "GPIO%d is not valid ADC pin", PH_GPIO);
        return;
    }

    s_ph_adc_handle = turbidity_get_adc_handle();
    if (s_ph_adc_handle == NULL) {
        ESP_LOGE(TAG, "ph init failed: adc handle is NULL");
        return;
    }

    adc_oneshot_chan_cfg_t chan_config = {
        .atten = PH_ADC_ATTEN,
        .bitwidth = PH_ADC_BITWIDTH,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(s_ph_adc_handle, s_ph_channel, &chan_config));

    ESP_LOGI(TAG, "ph init ok, gpio=%d channel=%d", PH_GPIO, s_ph_channel);
}

int ph_sensor_read_raw(void)
{
    if (!s_ph_adc_handle) {
        return 0;
    }

    int raw = 0;
    if (adc_oneshot_read(s_ph_adc_handle, s_ph_channel, &raw) != ESP_OK) {
        return 0;
    }

    return raw;
}

float ph_sensor_read_voltage(void)
{
    int raw = ph_sensor_read_raw();
    return (raw / 4095.0f) * 3.3f;
}

float ph_sensor_read_value(void)
{
    /*
     * 先给一版“可运行”的近似值：
     * 假设 2.50V 对应 pH 7.00
     * 这类板后面一定要用标准液校准
     */
    float voltage = ph_sensor_read_voltage();

    float ph = 7.0f + ((2.50f - voltage) / 0.18f);

    if (ph < 0.0f) ph = 0.0f;
    if (ph > 14.0f) ph = 14.0f;

    return ph;
}

const char *ph_sensor_level(float ph)
{
    if (ph < 6.5f) {
        return "Acidic";
    } else if (ph <= 7.5f) {
        return "Neutral";
    } else {
        return "Alkaline";
    }
}