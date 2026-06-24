 /**
* * =============================================================================
 * CONCLUSION DEL EQUIPO
 * Integrantes: Alan Efrén García Ortiz 9539
 *  
1. ¿Qué diferencia existe entre BLOCKED y SUSPENDED? 
    cuando está en blocked, la tarea está esperando que ocurra un evento. 
    Se despierta automáticamente cuando el evento sucede o el tiempo expira.

    Cuando la tarea está en suspended, está completamente detenida y nunca volverá 
    a ejecutarse por sí sola. La única forma de sacarla de este estado es que otra tarea llame 
    explícitamente a vTaskResume() pasándole su Handle.

2. ¿Por qué vTaskDelay() coloca una tarea en estado BLOCKED? 
    Porque al bloquearse, la tarea le avisa al scheduler que no necesita el procesador durante ese tiempo. 
    Esto libera el CPU inmediatamente para que otras tareas de igual o menor prioridad puedan ejecutarse 
    en lugar de desperdiciar ciclos de reloj.

3. ¿Qué diferencia existe entre vTaskDelay() y un Software Timer? 
    con vTaskDelay() se pausa el flujo de ejecución de tu propia tarea por un tiempo determinado. 
    La tarea se bloquea y luego continúa justo donde se quedó.

    Software Timer es un objeto del RTOS que ejecuta una función callback independiente cuando 
    su periodo expira. Esta función no corre en una tarea, sino en una tarea del sistema.

4. ¿Qué función cumple el Idle Task? 
    Su función es garantizar que el procesador siempre tenga algo que ejecutar cuando todas las 
    demás tareas del usuario estén bloqueadas o suspendidas, cuando un auto está encendido, el
    motor sigue trabajando aunque no esté avanzando.

5. ¿Cómo decide FreeRTOS cuál tarea ejecutar cuando varias tienen la misma prioridad? 
    Utiliza una técnica llamada Round Robin, la cual da una fracción del procesador a cada
    tarea de forma equitativa.

6. ¿Qué ventajas aporta pvParameters? 
    Permite pasar un puntero genérico (usualmente una estructura de datos) 
    a la tarea al momento de crearla. La gran ventaja es la reutilización de código: 
    se puede escribir una sola función para controlar un botón o un LED, y crear múltiples 
    tareas basadas en esa misma función, pasándole pines y configuraciones diferentes a cada 
    una a través de pvParameters, por ejemplo.

7. ¿Qué ventajas aporta TaskHandle_t? 
    Otorga un control para la tarea. Proporciona una referencia o puntero a la tarea que se 
    está ejecutando. Con él, un Task Manager u otra tarea puede intervenir la tarea de forma 
    externa para obtener su estado (eTaskGetState()), pausarla (vTaskSuspend), reanudarla (vTaskResume) 
    o cambiar su prioridad en tiempo de ejecución.

8. ¿Qué ocurriría si el contador se implementara con variables globales únicamente?
    Aparte de causar un código desordenado, el código depende de usar flags y estar verificándolas 
    constantemente con el CPU. 
     
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
 **/
#include "system_state.h"
#include "app_task.h"


void app_main(void){
    system_state_init();

    app_tasks_create();
}