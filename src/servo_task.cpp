#include "servo_task.hpp"
#include "app_context.hpp"
#include "messages.hpp"
#include "app_config.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

namespace App
{
    static const char *TAG = "SERVO";

    static uint32_t angle_to_duty(const ServoTaskConfig *cfg, uint8_t angle)
    {
        uint32_t max_duty = (1UL << cfg->resolution) - 1UL;
        uint32_t pulse_us = cfg->min_us + ((uint32_t)angle * (cfg->max_us - cfg->min_us)) / 180UL;
        return (pulse_us * max_duty * cfg->freq_hz) / 1000000UL;
    }

    static void servo_write_angle(const ServoTaskConfig *cfg, uint8_t angle)
    {
        uint32_t duty = angle_to_duty(cfg, angle);
        ledc_set_duty(cfg->mode, cfg->channel, duty);
        ledc_update_duty(cfg->mode, cfg->channel);
    }

    void ServoTask::run(void *pvParameters)
    {
        auto *cfg = static_cast<ServoTaskConfig *>(pvParameters);

        ledc_timer_config_t timer_cfg = {};
        timer_cfg.speed_mode = cfg->mode;
        timer_cfg.duty_resolution = cfg->resolution;
        timer_cfg.timer_num = cfg->timer;
        timer_cfg.freq_hz = cfg->freq_hz;
        timer_cfg.clk_cfg = LEDC_AUTO_CLK;
        ESP_ERROR_CHECK(ledc_timer_config(&timer_cfg));

        ledc_channel_config_t ch_cfg = {};
        ch_cfg.gpio_num = cfg->gpio;
        ch_cfg.speed_mode = cfg->mode;
        ch_cfg.channel = cfg->channel;
        //ch_cfg.intr_type = LEDC_INTR_DISABLE;
        ch_cfg.timer_sel = cfg->timer;
        ch_cfg.duty = 0;
        ch_cfg.hpoint = 0;
        ESP_ERROR_CHECK(ledc_channel_config(&ch_cfg));

        uint8_t current_angle = 0;
        servo_write_angle(cfg, current_angle);
        ESP_LOGI(TAG, "Tarea de motor iniciada");
        
        ServoCmd current_cmd = { AppConfig::SERVO_ANGLE_DARK, 
            AppConfig::SERVO_TOLERANCE_DEG, 
            AppConfig::SERVO_DELAY_SLOW_MS, 
            AppConfig::SERVO_STEP_DEG };

        

        while (true)
        {
            xQueueReceive(g_queues.servo_cmd, &current_cmd, 0);

            ServoStatusMsg status_msg;
            status_msg.target_angle = current_cmd.target_angle;
            status_msg.tick = xTaskGetTickCount();

            int diff = (int)current_cmd.target_angle - (int)current_angle;

            if (abs(diff) <= current_cmd.tolerance_deg)
            {
                status_msg.status = ServoStatusType::Reached;
                status_msg.current_angle = current_angle;
                
                xQueueOverwrite(g_queues.servo_status, &status_msg);
                
                vTaskDelay(pdMS_TO_TICKS(50)); 
            }

            else
            {
                // Decidimos si sumamos o restamos grados
                if (diff > 0)
                {
                    current_angle += current_cmd.step_deg;
                    if (current_angle > 180) current_angle = 180; // Tope de seguridad
                }
                else
                {
                    // Evitamos que el valor baje de cero (underflow)
                    if (current_angle < current_cmd.step_deg) current_angle = 0; 
                    else current_angle -= current_cmd.step_deg;
                }
            }

            servo_write_angle(cfg, current_angle);

            status_msg.status = ServoStatusType::Moving;
            status_msg.current_angle = current_angle;
            
            xQueueOverwrite(g_queues.servo_status, &status_msg);

            vTaskDelay(pdMS_TO_TICKS(current_cmd.step_delay_ms));
        }
    }
}
