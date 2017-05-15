/*
 * main.c
 *
 *  Created on: 2016/7/26
 *      Author: lc
 */

#include <stdio.h>
#include <string.h>
#include <eat_modem.h>
#include <eat_interface.h>
#include <eat_uart.h>
#include <eat_timer.h>
#include <eat_socket.h>
#include <eat_clib_define.h> //only in main.c
#include <eat_gps.h>

#include "event.h"
#include "watchdog.h"
#include "timer.h"
#include "setting.h"
#include "log.h"
#include "version.h"

/********************************************************************
 * Macros
 ********************************************************************/
#define EAT_UART_RX_BUF_LEN_MAX 2048
#define EAT_MEM_MAX_SIZE 300*1024

/********************************************************************
 * Types
 ********************************************************************/
typedef void (*app_user_func)(void*);

/********************************************************************
 * Extern Variables (Extern /Global)
 ********************************************************************/

/********************************************************************
 * Local Variables:  STATIC
 ********************************************************************/
static u8 s_memPool[EAT_MEM_MAX_SIZE];

/********************************************************************
 * External Functions declaration
 ********************************************************************/
extern void APP_InitRegions(void);

/********************************************************************
 * Local Function declaration
 ********************************************************************/
void app_main(void *data);
void app_func_ext1(void *data);

/********************************************************************
 * Local Function
 ********************************************************************/
#pragma arm section rodata = "APP_CFG"
APP_ENTRY_FLAG
#pragma arm section rodata

#pragma arm section rodata = "APPENTRY"
	const EatEntry_st AppEntry =
	{
		app_main,
		app_func_ext1,
		(app_user_func)EAT_NULL,//app_user1,
		(app_user_func)EAT_NULL,//app_user2,
		(app_user_func)EAT_NULL,//app_user3,
		(app_user_func)EAT_NULL,//app_user4,
		(app_user_func)EAT_NULL,//app_user5,
		(app_user_func)EAT_NULL,//app_user6,
		(app_user_func)EAT_NULL,//app_user7,
		(app_user_func)EAT_NULL,//app_user8,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL,
		EAT_NULL
	};
#pragma arm section rodata

void app_func_ext1(void *data)
{
	/*This function can be called before Task running ,configure the GPIO,uart and etc.
	   Only these api can be used:
		 eat_uart_set_debug: set debug port
		 eat_pin_set_mode: set GPIO mode
		 eat_uart_set_at_port: set AT port
	*/

    EatUartConfig_st cfg =
    {
        EAT_UART_BAUD_115200,
        EAT_UART_DATA_BITS_8,
        EAT_UART_STOP_BITS_1,
        EAT_UART_PARITY_NONE
    };

    eat_uart_set_at_port(EAT_UART_USB);// UART1 is as AT PORT
	eat_uart_set_debug(EAT_UART_1);
    eat_uart_set_debug_config(EAT_UART_DEBUG_MODE_UART, &cfg);
}

void app_main(void *data)
{
    EatEvent_st event;
    eat_bool rc;

    APP_InitRegions();//Init app RAM
    APP_init_clib(); //C library initialize, second step

    LOG_INFO("booting: version:%s, build_time=%s %s. core(version:%s, buildno=%s, buildtime=%s)",
            VERSION_STR, __DATE__, __TIME__, eat_get_version(), eat_get_buildno(), eat_get_buildtime());

    rc = eat_mem_init(s_memPool, EAT_MEM_MAX_SIZE);
    if (!rc)
    {
    	LOG_ERROR("eat memory initial error:%d!", rc);
        return;
    }

    setting_restore();

    startWatchdog();

    eat_gps_power_req(EAT_TRUE);
    eat_timer_start(TIMER_GPS,setting.main_gps_timer_period);

    while(EAT_TRUE)
    {
    	unsigned int event_num = eat_get_event_num();
    	if (event_num != 0)
    	{
    		int i = 0;
    		for (i = 0; i < event_num; i++)
    		{
    			eat_get_event(&event);

    	        event_proc(&event);
    		}
    	}
    }
}




