#include "turbidity.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TURBIDITY_ADC_CHANNEL ADC_CHANNEL_3
#define TURBIDITY_ADC_UNIT    ADC_UNIT_1

static adc_oneshot_unit_handle_t adc_handle;

void turbidity_init(void)
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = TURBIDITY_ADC_UNIT,
    };

    adc_oneshot_new_unit(&init_config, &adc_handle);

    adc_oneshot_chan_cfg_t channel_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    adc_oneshot_config_channel(adc_handle, TURBIDITY_ADC_CHANNEL, &channel_config);
}

int turbidity_read(void)
{
    int adc_raw = 0;
    adc_oneshot_read(adc_handle, TURBIDITY_ADC_CHANNEL, &adc_raw);
    return adc_raw;
}

int turbidity_read_avg(void)
{
    int sum = 0;

    for(int i = 0; i < 10; i++)
    {
        int val = 0;
        adc_oneshot_read(adc_handle, TURBIDITY_ADC_CHANNEL, &val);
        sum += val;

        vTaskDelay(pdMS_TO_TICKS(5));
    }

    return sum / 10;
}

const char* turbidity_level(int value)
{
    if(value > 3000)
        return "Clean";

    if(value > 1500)
        return "Normal";

    return "Dirty";
}

adc_oneshot_unit_handle_t turbidity_get_adc_handle(void)
{
    return adc_handle;
}