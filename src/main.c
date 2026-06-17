#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "esp_log.h"

#include <stdint.h>
#include "esp_adc/adc_oneshot.h"

#include "esp_freertos_hooks.h"


#define BOTON 4

volatile bool FLAG = false; //bandera para que idle task no entre antes
volatile bool CTI = false; //Check Time in Idle para ver si pasaron 5s

/*------------------------------------------------------------
                    CONFIGURACIONES TASKS
------------------------------------------------------------*/

TaskHandle_t LedRapido, LedLento, Monitor, ADC_Aux;

typedef struct{
    uint8_t gpio;
    uint32_t period_ms; 
    char name[8];
} LEDcfg; //dato tipo LEDcfg contiene la configuracion del LED

typedef struct{
    char hanlde_orgn[15]; //Parametro con nombre del handle
    char evento[45]; //Que evento va a reportar
    float ADC_valor; 
} dato_monitor; //Dato tipo dato_monitor que contiene la conf 
                //de la cola para impresion de mensajes

QueueHandle_t kiwi_monitor = NULL; //Handle para la cola

/*------------------------------------------------------------
                       CONFIGURACIONES ADC
------------------------------------------------------------*/

// Numero de canales ADC utilizados. 
#define ADC_CHANNEL_COUNT   1U
// Resolucion del ADC en bits (12 bits -> 0-4095). 
#define ADC_RESOLUTION_BITS 12U
// Valor maximo del ADC segun la resolucion configurada. 
#define ADC_MAX_VALUE       ((1U << ADC_RESOLUTION_BITS) - 1U)   //4095 
// Canales ADC1 asignados a cada potenciometro (indices 0, 1, 2). 
#define ADC_CH_POT0   ADC_CHANNEL_0   /*< GPIO 36 */

static const char *TAG = "PRACTICA1";

static adc_oneshot_unit_handle_t s_adc_handle = NULL;

/** Tabla: indice logico -> canal fisico ADC1. */
static const adc_channel_t adc_channel_map[ADC_CHANNEL_COUNT] = {
    ADC_CH_POT0,   /* Potenciometro 0 -> GPIO 36 */
};

/*------------------------------------------------------------
                        LECTOR DE ADC
------------------------------------------------------------*/

void adc_reader_init(void){
/* Crear unidad ADC1 en modo oneshot */
adc_oneshot_unit_init_cfg_t init_cfg = {
    .unit_id  = ADC_UNIT_1,
    .ulp_mode = ADC_ULP_MODE_DISABLE,
};
ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &s_adc_handle));

/* Configurar atenuacion y resolucion por canal */
adc_oneshot_chan_cfg_t chan_cfg = {
    .atten    = ADC_ATTEN_DB_12,       /* ~0-3.3 V */
    .bitwidth = ADC_BITWIDTH_DEFAULT,  /* 12 bits en ESP32 */
};

for (uint8_t i = 0; i < ADC_CHANNEL_COUNT; i++) {
    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        s_adc_handle, adc_channel_map[i], &chan_cfg));
}

ESP_LOGI(TAG, "ADC inicializado: %u canales, 12 bits, atten 12 dB",
            ADC_CHANNEL_COUNT);
}

/*------------------------------------------------------------
                        DATOS CRUDOS ADC
------------------------------------------------------------*/

uint16_t adc_reader_get_raw(uint8_t channel)
{
    if (channel >= ADC_CHANNEL_COUNT || s_adc_handle == NULL) {
        return 0U;
    }

    int raw = 0;
    /* adc_oneshot_read devuelve ESP_OK si la lectura es valida */
    if (adc_oneshot_read(s_adc_handle, adc_channel_map[channel], &raw)
            == ESP_OK) {
        return (uint16_t)raw;
    }

    return 0U;
}

/*------------------------------------------------------------
                        TAREA LED RAPIDO
------------------------------------------------------------*/

void vTaskLedRapido(void *pvParameters){
    LEDcfg *conf; //apuntador tipo LEDcfg

    conf = (LEDcfg *) pvParameters; //casteo de conf a pvParameters

    gpio_set_direction(conf->gpio, GPIO_MODE_OUTPUT);
                                        //apuntador a gpio
                                        //modo del gpio
    bool state = false; //estado del LED

    while(1){
        
        dato_monitor msg_ledR = {
        .ADC_valor = 0.0,
        .evento = "LED RAPIDO TASK",
        .hanlde_orgn = "TASK1"
        };
        xQueueSend(kiwi_monitor, &msg_ledR, pdMS_TO_TICKS(10));


        for(int i = 0; i < 20; i++){
            state = !state; //negar el LED(?)
            gpio_set_level(conf -> gpio, state);  
            vTaskDelay(pdMS_TO_TICKS(conf -> period_ms)); //apuntador a la
                                                         //frecuencia            
        }         

        vTaskResume(LedLento);  //Activa a led lento para que 
                                //no se pelien por la RAM 
                                //#peace #wearetheworld
        vTaskSuspend(NULL); //Se suspende a si misma                   
    }
}

/*------------------------------------------------------------
                        TAREA LED LENTO
------------------------------------------------------------*/

void vTaskLedLento(void *pvParameters){
    LEDcfg *conf2; //apuntador tipo LEDcfg

    conf2 = (LEDcfg *) pvParameters; //casteo de conf a pvParameters

    gpio_set_direction(conf2 -> gpio, GPIO_MODE_OUTPUT);
                                        //apuntador a gpio
                                        //modo del gpio
    bool state2 = false; //estado del LED

    vTaskSuspend(NULL);

    while(1){
         dato_monitor msg_ledL = {
        .ADC_valor = 0.0,
        .evento = "LED LENTO TASK",
        .hanlde_orgn = "TASK2"
        };

        xQueueSend(kiwi_monitor, &msg_ledL, pdMS_TO_TICKS(10));    

        for(int i = 0; i < 10; i++){
            state2 = !state2; //negar el LED(?)
            gpio_set_level(conf2 -> gpio, state2);  
            vTaskDelay(pdMS_TO_TICKS(conf2 -> period_ms)); //apuntador a la
                                                            //frecuencia            
        } 
        FLAG = true;

        vTaskSuspend(NULL);//Se suspende a si misma
    }
}


/*------------------------------------------------------------
                        TASK MONITOR
------------------------------------------------------------*/
void vTaskImpresion(void *pvParameters){
    dato_monitor msg_recibido;

    while(1){
        if(xQueueReceive(kiwi_monitor, &msg_recibido, portMAX_DELAY) == pdTRUE){
            if(msg_recibido.ADC_valor > 0.0){
                ESP_LOGI(TAG, "[%s] %s | Valor ADC: %f",
                msg_recibido.hanlde_orgn,
                msg_recibido.evento,
                msg_recibido.ADC_valor);
            }
            else {
                ESP_LOGI(TAG, "[%s] %s" , 
                    msg_recibido.hanlde_orgn, 
                    msg_recibido.evento);
            }
        }
    }
}

/*------------------------------------------------------------
                            IDLE TASK
------------------------------------------------------------*/
bool vIdleAux(void){

    if(!FLAG){
        return true;
    }
    static bool candado = true;
    static uint32_t tiempo = 0;


    if(candado){
        tiempo = xTaskGetTickCount();
        candado = false;

        dato_monitor msg_idleStart = {
            .hanlde_orgn = "IDLE_TASK",
            .evento = "EMPEZANDO 5s IDLE TASK...",
            .ADC_valor = 0.0
        };
        xQueueSend(kiwi_monitor, &msg_idleStart, 0);
    }   
    if(gpio_get_level(BOTON) == 0){
        FLAG = false;
        candado = true;
        CTI = false;
        vTaskResume(ADC_Aux);
    }
 
    
    else if((xTaskGetTickCount() - tiempo) >= pdMS_TO_TICKS(5000)){
        FLAG = false;
        CTI = true;
        candado = true;
        vTaskResume(LedRapido);
        vTaskResume(ADC_Aux);
    }

    return true;//Alimentar al perro miron y salir del idle task
}

/*------------------------------------------------------------
                    FUNCION AUXILIAR ADC
------------------------------------------------------------*/
void vTaskADC_Aux(void *pvParameters){
    vTaskSuspend(NULL);

    while(1){
        if(CTI == true){
            dato_monitor msg_tiempo ={
                .hanlde_orgn = "IDLE_TASK",
                .evento = "HAN PASADO 5s DE IDLE TASK",
                .ADC_valor = 0
            };
            xQueueSend(kiwi_monitor, &msg_tiempo, 0);
        }
        
        else{
            float ADC_Lectura = ((float)adc_reader_get_raw(0) / 4095.0)*3.3;
            
            dato_monitor msg_boton = {
                .hanlde_orgn = "IDLE_TASK",
                .evento = "BOTON DETECTADO",
                .ADC_valor = ADC_Lectura
            };
            xQueueSend(kiwi_monitor, &msg_boton, portMAX_DELAY);
        }
        vTaskResume(LedRapido);
        vTaskSuspend(NULL);
    }
}


/*------------------------------------------------------------
                            APP MAIN
------------------------------------------------------------*/
void app_main(void) {
    adc_reader_init(); //inicializar ADC
    gpio_set_direction(BOTON, GPIO_MODE_INPUT); //Configuracion del boton
    gpio_set_pull_mode(BOTON, GPIO_PULLUP_ONLY);

    kiwi_monitor = xQueueCreate(10, sizeof(dato_monitor)); //crear la cola

    esp_register_freertos_idle_hook(vIdleAux);

    static LEDcfg LED1 = {.gpio = 2, .period_ms = 250};
    strcpy(LED1.name, "LED 1");         //configuraciones del LED con
                                        //apuntadores tipo . ya que 
                                        //LED1 no es un apuntador
    static LEDcfg LED2 = {.gpio = 2, .period_ms = 500};
    strcpy(LED2.name, "LED 2"); 

    
    xTaskCreate(vTaskImpresion, "TASK3", 2048, &Monitor, 3, NULL);
    xTaskCreate(vTaskLedRapido, "TASK1", 2048, &LED1, 2, &LedRapido);
    xTaskCreate(vTaskLedLento, "TASK2", 2048, &LED2, 1, &LedLento);
    xTaskCreate(vTaskADC_Aux, "TASK 4", 2048, NULL, 1, &ADC_Aux);                                   
}