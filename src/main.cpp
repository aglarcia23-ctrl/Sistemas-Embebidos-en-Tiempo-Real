 /** =============================================================================
 * CONCLUSION DEL EQUIPO
 * Integrantes: Alan Efrén García Ortiz 9539
 *  
1. ¿Cuál es la diferencia entre usar una variable global y una cola para comunicar datos?
     Una variable global no es segura en un entorno multitarea, ya que, puede sufrir corrupción 
     de datos o condiciones de carrera si dos tareas acceden al mismo tiempo y requiere 
     de revisión constante. Las colas de FreeRTOS son seguras y despiertan automáticamente 
     a las tareas solo cuando hay información nueva, ahorrando CPU.

2. ¿Qué tarea queda bloqueada cuando espera datos de una cola? 
    Queda bloqueada la tarea receptora que activa a xQueueReceive. Pasa al estado 
    Blocked y cede el procesador hasta que otra tarea envíe un dato a esa cola o 
    expire el tiempo máximo de espera.

3. ¿Por qué TaskManager debe concentrar las decisiones del sistema?
    Concentra las decisiones para mantener la arquitectura limpia como el
    patrón de una máquina de estados. Si cada tarea intentara controlar a las demás o 
    tomar decisiones globales, el sistema se volvería impredecible e imposible de depurar.
    El TaskManager actúa como un organizador.

4. ¿Por qué pvParameters es más flexible que crear una función distinta por tarea?
    Permite la reutilización de código. se puede escribir una sola función base
    e instanciarla múltiples veces pasándole pines o configuraciones diferentes al 
    momento de crearla. Esto ahorra memoria Flash y evita duplicar funciones idénticas.

5. ¿Qué diferencia existe entre suspender una tarea y bloquearla esperando una cola? 
    Suspender (vTaskSuspend) saca a la tarea del planificador indefinidamente, es decir,
    solo volverá a correr si otra tarea la llama manualmente con vTaskResume.

    Bloquear por cola es automático, la tarea se detiene esperando un dato y el 
    RTOS la despierta por sí solo en el instante exacto en que el dato llega.

6. ¿Qué efecto tiene aumentar el tamaño de la ventana del filtro de mediana? 
    Hace que la señal resultante sea mucho más suave y resistente a ruidos o 
    fluctuaciones de luz. Sin embargo, su desventaja es que aumenta la latencia.
    El sistema tardará más tiempo en reaccionar ante un cambio real y repentino 
    de iluminación.

7. ¿Por qué un filtro de mediana rechaza picos mejor que un promedio simple? 
    Un promedio matemático suma todos los valores, por lo que un solo pico de ruido 
    extremo alterará drásticamente el resultado final. La mediana, en cambio, 
    ordena los valores y toma el del centro. Los picos extremos quedan en las orillas 
    del arreglo y son ignorados por completo.

8. ¿Qué ocurre si se presiona Start-operation mientras el servo está en movimiento?
    No ocurre nada porque la tarea responsable es suspendida por el TaskManager.

9. ¿Cómo se garantiza que el botón de velocidad solo funcione durante la operación? 
    El TaskManager mantiene la tarea Task_ButtonSpeed suspendida mientras el sistema 
    está en reposo. Solo la reanuda cuando comienza la operación del servo y la vuelve
     a suspender en cuanto pasan los 8 segundos de espera.

10. ¿Por qué un constexpr en C++ es preferible a #define para constantes tipadas? 
    #define es una macro ciega de texto que ignora las reglas de C++ 
    el alcance de las variables y los tipos de datos. constexpr es una constante 
    real evaluada en tiempo de compilación que respeta el tipo de dato y el ámbito, 
    lo que hace al código más seguro, predecible y mucho más fácil de rastrear 
    si hay errores.

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
 * 
 **/

#include "task_manager.hpp"

extern "C" void app_main(void)
{
    App::app_tasks_create();
}
