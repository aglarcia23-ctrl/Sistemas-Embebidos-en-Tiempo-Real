#include "app_task.h"

#include "app_config.h"
#include "system_state.h"
#include "leds.h"
#include "buttons.h"
#include "counter.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

static const char *TAG = "MANAGER";

static TaskHandle_t h_leds[4];
static TaskHandle_t h_btn_start;
static TaskHandle_t h_btn_dir;
static TaskHandle_t h_btn_speed;
static TaskHandle_t h_counter;
static TaskHandle_t h_manager;

static LedTaskParams_t led_params[4] =
{
    { LED_B0, 0, "LED_BIT_0" },
    { LED_B1, 1, "LED_BIT_1" },
    { LED_B2, 2, "LED_BIT_2" },
    { LED_B3, 3, "LED_BIT_3" }
};

static ButtonTaskParams_t btn_start =
{
    .gpio = BTN_START,
    .name = 'S',
    .type = BUTTON_START_PAUSE
};

static ButtonTaskParams_t btn_dir =
{
    .gpio = BTN_DIR,
    .name = 'D',
    .type = BUTTON_DIRECTION
};

static ButtonTaskParams_t btn_speed =
{
    .gpio = BTN_SPEED,
    .name = 'V',
    .type = BUTTON_SPEED
};

static const char *state_to_string(eTaskState state)
{
    switch (state)
    {
        case eRunning:
            return "RUNNING";

        case eReady:
            return "READY";

        case eBlocked:
            return "BLOCKED";

        case eSuspended:
            return "SUSPENDED";

        case eDeleted:
            return "DELETED";

        default:
            return "UNKNOWN";
    }
}

static void manager_pause_system(void)
{
    g_system.mode = SYSTEM_PAUSED;

    /* Suspendemos contador y botones de control */
    vTaskSuspend(h_counter);
    vTaskSuspend(h_btn_dir);
    vTaskSuspend(h_btn_speed);

    ESP_LOGW(TAG, "Sistema PAUSADO");
}

static void manager_run_system(void)
{
    g_system.mode = SYSTEM_RUNNING;

    /* Despertamos botones de control y al contador */
    vTaskResume(h_btn_dir);
    vTaskResume(h_btn_speed);
    vTaskResume(h_counter);

    ESP_LOGW(TAG, "Sistema RUNNING");
}

static void manager_toggle_direction(void)
{
    if (g_system.direction == COUNT_UP)
    {
        g_system.direction = COUNT_DOWN;
    }
    else
    {
        g_system.direction = COUNT_UP;
    }

    ESP_LOGI(TAG, "Nueva direccion: %s", g_system.direction == COUNT_UP ? "UP" : "DOWN");
}

static void manager_toggle_speed(void)
{
    if (g_system.period_ms == SPEED_SLOW_MS)
    {
        g_system.period_ms = SPEED_FAST_MS;
    }
    else
    {
        g_system.period_ms = SPEED_SLOW_MS;
    }

    ESP_LOGI(TAG, "Nueva velocidad: %lu ms", (unsigned long)g_system.period_ms);
}

static void manager_print_states(void)
{
    ESP_LOGI(TAG, "------ ESTADOS ------");
    ESP_LOGI(TAG, "COUNTER: %s", state_to_string(eTaskGetState(h_counter)));
    ESP_LOGI(TAG, "BTN_START: %s", state_to_string(eTaskGetState(h_btn_start)));
    ESP_LOGI(TAG, "BTN_DIR: %s", state_to_string(eTaskGetState(h_btn_dir)));
    ESP_LOGI(TAG, "BTN_SPEED: %s", state_to_string(eTaskGetState(h_btn_speed)));

    for (int i = 0; i < 4; i++)
    {
        ESP_LOGI(TAG, "%s: %s", led_params[i].name, state_to_string(eTaskGetState(h_leds[i])));
    }

    ESP_LOGI(TAG, "Valor=%u | Modo=%s | Direccion=%s | Periodo=%lu ms",
             g_system.value,
             g_system.mode == SYSTEM_RUNNING ? "RUNNING" : "PAUSED",
             g_system.direction == COUNT_UP ? "UP" : "DOWN",
             (unsigned long)g_system.period_ms);
}

static void task_manager(void *pvParameters)
{
    (void)pvParameters;

    TickType_t last_print;
    last_print = xTaskGetTickCount();

    while (1)
    {
        ManagerEvent_t event_to_process;

        event_to_process = g_system.pending_event;

        if (event_to_process != MANAGER_EVENT_NONE)
        {
            g_system.pending_event = MANAGER_EVENT_NONE;

            switch (event_to_process)
            {
                case MANAGER_EVENT_START_PAUSE:
                    if (g_system.mode == SYSTEM_RUNNING)
                    {
                        manager_pause_system();
                    }
                    else
                    {
                        manager_run_system(); // <-- Ahora sí: si está pausado, ¡ARRANCA!
                    }
                    break;

                case MANAGER_EVENT_DIRECTION:
                    if (g_system.mode == SYSTEM_RUNNING)
                    {
                        manager_toggle_direction();
                    }
                    else
                    {
                        ESP_LOGW(TAG, "Direccion ignorada: sistema pausado");
                    }
                    break;

                case MANAGER_EVENT_SPEED:
                    if (g_system.mode == SYSTEM_RUNNING)
                    {
                        manager_toggle_speed();
                    }
                    else
                    {
                        ESP_LOGW(TAG, "Velocidad ignorada: sistema pausado");
                    }
                    break;

                default:
                    break;
            }
        }

        if ((xTaskGetTickCount() - last_print) >= pdMS_TO_TICKS(2000))
        {
            last_print = xTaskGetTickCount();
            manager_print_states();
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void app_tasks_create(void)
{
    xTaskCreate(counter_task, "COUNTER", 2048, NULL, 2, &h_counter);
    xTaskCreate(button_task, "BTN_START", 2048, &btn_start, 2, &h_btn_start);
    xTaskCreate(button_task, "BTN_DIR",   2048, &btn_dir,   2, &h_btn_dir);
    xTaskCreate(button_task, "BTN_SPEED", 2048, &btn_speed, 2, &h_btn_speed);

    for (int i = 0; i < 4; i++)
    {
        xTaskCreate(led_task, led_params[i].name, 2048, &led_params[i], 2, &h_leds[i]);
    }

    xTaskCreate(task_manager, "MANAGER", 2048, NULL, 3, &h_manager);

    manager_pause_system();
}