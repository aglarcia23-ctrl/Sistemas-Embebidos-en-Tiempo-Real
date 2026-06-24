#include "sensor_task.hpp"
#include "app_config.hpp"
#include "app_context.hpp"
#include "messages.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

namespace App
{
    static const char *TAG = "SENSOR";

    static uint16_t median_u16(uint16_t *v, uint8_t n)
    {

        /* AQUI SE IMPLEMENTA EL FILTRO :) */
        for(uint8_t i = 0; i < n-1; i++){
            for(uint8_t j = 0; j < n-1; j++){
                if(v[j] > v[j + 1]){
                    uint16_t temp = v[j];
                    v[j] = v[j + 1];
                    v[j + 1] = temp;
                }
            }
        }
        return v[n / 2];
    }

    static uint8_t target_from_ldr(uint16_t filtered)
    {
        if (filtered >= AppConfig::LDR_THRESHOLD_HIGH)
        {
            return AppConfig::SERVO_ANGLE_LIGHT;
        }
            
        if (filtered <= AppConfig::LDR_THRESHOLD_LOW) 
        {
            return AppConfig::SERVO_ANGLE_DARK;
        }
        return (filtered > ((AppConfig::LDR_THRESHOLD_LOW + AppConfig::LDR_THRESHOLD_HIGH) / 2)) ? AppConfig::SERVO_ANGLE_LIGHT : AppConfig::SERVO_ANGLE_DARK;
    }

    void SensorTask::run(void *pvParameters)
    {
        auto *cfg = static_cast<SensorTaskConfig *>(pvParameters);

        adc_oneshot_unit_handle_t adc_handle;
        adc_oneshot_unit_init_cfg_t unit_cfg = {};
        unit_cfg.unit_id = cfg->unit_id;
        ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc_handle));

        adc_oneshot_chan_cfg_t chan_cfg = {};
        chan_cfg.atten = ADC_ATTEN_DB_12;
        chan_cfg.bitwidth = ADC_BITWIDTH_12;
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, cfg->channel, &chan_cfg));

        ESP_LOGI(TAG, "%s iniciado", cfg->name);

        while (true)
        {
            /* IMPLEMENTAR TAREA :)*/
            uint16_t samples[cfg->filter_window];

            for(uint8_t i = 0; i < cfg->filter_window; i++){
                int raw_val = 0;
                adc_oneshot_read(adc_handle, cfg->channel, &raw_val);
                samples[i] = static_cast<uint16_t>(raw_val);
                vTaskDelay(pdMS_TO_TICKS(5));
            }

            uint16_t filtered = median_u16(samples, cfg->filter_window);

            uint8_t target = target_from_ldr(filtered);

            SensorMsg msg = {samples[0], filtered, target, xTaskGetTickCount()};

            xQueueOverwrite(g_queues.sensor, &msg);

            vTaskDelay(pdMS_TO_TICKS(cfg->period_ms));
        }
    }
}
