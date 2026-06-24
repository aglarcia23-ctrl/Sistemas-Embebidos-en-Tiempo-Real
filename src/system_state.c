#include "system_state.h"
#include "app_config.h"

SystemState_t g_system;

void system_state_init(void)
{
	/*TODO:*/ 
		//1. Establecer el valor de value a cero,
		g_system.value = 0; 
		//2. Establecer periodo_ms a una velocidad,
		g_system.period_ms = SPEED_SLOW_MS;
		//3. Establecer dirección,
		g_system.direction = COUNT_UP;
		//4. Establecer modo,
		g_system.mode = SYSTEM_PAUSED;
		//5. Establecer el estado del manager. :)
		g_system.pending_event = MANAGER_EVENT_NONE;

}