/**
 * @file    main.c
 * @brief   Punto de entrada del sistema de control de LEDs por potenciometros.
 *
 * =============================================================================
 * CONCLUSION DEL EQUIPO
 * Integrantes: Alan Efrén García Ortiz 9539
 *  
¿Por qué es preferible pasar la configuración de canal mediante pvParameters en lugar de usar variables 
globales o funciones de tarea distintas para cada canal? 
    Al usar pvParameters, se puede reutilizar una sola función (vTaskPotLED) para los tres canales sin duplicar código. 
    Si se usa variables globales, se corre el riesgo de sobrescribir datos entre tareas, y si 
    se usa una función distinta para cada LED, se tendría un código redundante y poco escalable.

2. ¿Qué sucedería si los bloques task_params_t se declararan como variables locales en tasks_create_all() en 
lugar de static? ¿Cómo lo detectarías experimentalmente? 
    Si se declara la estructura como variable local, su espacio en memoria se libera en cuanto la función 
    tasks_create_all() termina. La tarea en ejecución quedaría apuntando a un espacio de memoria vacío.

3. Las tres tareas tienen el mismo periodo (50 ms) pero prioridades distintas. ¿En qué situación se hace visible 
la diferencia de prioridad en la salida del terminal? 
    La diferencia de prioridad es cuando dos o más tareas salen de su periodo de bloqueo temporal al mismo tiempo. 
    Al competir por el procesador, FreeRTOS siempre ejecutará y enviará al monitor serial primero el mensaje de la 
    tarea con mayor prioridad, determinando el orden en que se imprimen los mensajes en la terminal.
 
4. ¿Por qué se usa vTaskDelay(pdMS_TO_TICKS(50)) en lugar de un for con retardo de software? ¿Qué 
diferencia implica para el planificador? 
    Un retardo por software (bucle for) acapara activamente el CPU, manteniendo a la tarea en estado Running e 
    impidiendo que tareas de menor prioridad se ejecuten. Por el contrario, vTaskDelay cambia el estado de la 
    tarea a Blocked, liberando inmediatamente el procesador para que el planificador pueda ejecutar otras 
    tareas útiles durante ese tiempo de espera.

5. ¿Qué valor esperarías en el stack watermark de la tarea con menor prioridad comparado con el de mayor 
prioridad? ¿Por qué podrían diferir? 
    El watermark debería ser muy similar, ya que todas las tareas ejecutan el mismo bloque de código. 
    Sin embargo, la tarea de menor prioridad podría registrar un watermark ligeramente menor. 

6. Si se añadiera un cuarto potenciómetro y LED, ¿qué cambios mínimos requeriría el código dado el diseño 
modular actual y la restricción de no modificar las firmas existentes?
    Definir los pines correspondientes al nuevo canal en las configuraciones de inicialización.
    Declarar un cuarto bloque de parámetros static const task_params_t con los nuevos índices en tasks.c.  
    Añadir una cuarta llamada a la función xTaskCreate dentro de tasks_create_all(), pasándole el puntero 
    a esa nueva estructura.  

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