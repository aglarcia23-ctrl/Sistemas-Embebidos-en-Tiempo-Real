/**
 * @file    main.c
 * @brief   Punto de entrada del sistema de control de LEDs por potenciometros.
 *
 * =============================================================================
 * CONCLUSION DEL EQUIPO
 * Integrantes: Nombre Apellido, Nombre Apellido, Nombre Apellido
 *
 * El uso de pvParameters con una estructura task_params_t permite instanciar
 * una sola funcion de tarea (vTaskPotLED) tres veces con configuraciones
 * distintas, eliminando codigo duplicado y facilitando el mantenimiento.
 * Al asignar prioridades distintas (1, 2, 3) observamos que el planificador
 * preemptivo de FreeRTOS siempre atiende primero la tarea de mayor prioridad
 * cuando esta sale de su estado bloqueado, independientemente de que tarea
 * estuviera en ejecucion. Como los tres periodos son iguales (50 ms), la
 * diferencia de prioridades se manifiesta en el orden de ejecucion cuando
 * multiples tareas desbloquean al mismo tick del RTOS.
 * =============================================================================
 *
 * Descripcion:
 *   Inicializa los perifericos (ADC y LEDC) y arranca tres tareas FreeRTOS.
 *   Cada tarea lee un potenciometro y ajusta el brillo del LED correspondiente.
 *
 *   La logica de negocio esta completamente encapsulada en los modulos:
 *     - adc_reader : lectura de potenciometros via ADC oneshot
 *     - leds       : control de brillo via LEDC (PWM)
 *     - tasks      : tareas FreeRTOS y sus parametros
 *
 * Plataforma : ESP32 / ESP-IDF 5.x - 6.x  / FreeRTOS
 * IDE        : VS Code + PlatformIO (framework: espidf)
 * Estandar   : C99
 */

#include "adc_reader.h"
#include "leds.h"
#include "tasks.h"


/* ------------------------------------------------------------------ */
/*  Punto de entrada                                                   */
/* ------------------------------------------------------------------ */

void app_main(void)
{
    /* TODO 1. Inicializar subsistema ADC (tres potenciometros) */
    adc_reader_init();
    
    /* TODO 2. Inicializar subsistema LED PWM (tres canales LEDC) */
    leds_init();
    
    /* TODO 3. Crear las tres tareas FreeRTOS e iniciar el scheduler */
    tasks_create_all();
    
    /* app_main retorna; FreeRTOS continua ejecutando las tareas */
}