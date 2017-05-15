/*
 * event.c
 *
 *  Created on: 2016/7/26
 *      Author: lc
 */
#include <string.h>

#include <eat_interface.h>
#include <eat_uart.h>
#include <eat_gps.h>

#include "timer.h"
#include "log.h"
#include "uart.h"
#include "socket.h"
#include "setting.h"
#include "msg.h"
#include "client.h"
#include "fsm.h"
#include "request.h"
#include "mem.h"
#include "modem.h"
#include "utils.h"

typedef int (*EVENT_FUNC)(const EatEvent_st* event);
typedef struct
{
	EatEvent_enum event;
	EVENT_FUNC pfn;
}EVENT_PROC;


#define DESC_DEF(x) case x:\
                            return #x
void event_gps_proc(void)
{
    unsigned char buf[1024] = {0};
    unsigned char* buf_gps = NULL;
    int rc;

    eat_bool iGpsFixed = EAT_FALSE;
    double iGpstime = 0.0;
    int satellite = 0;
    float latitude = 0.0;
    float longitude = 0.0;
    float altitude = 0.0;
    float speed = 0.0;
    float course = 0.0;

    /*
     * the output format of eat_gps_nmea_info_output
     * $GPSIM,<latitude>,<longitude>,<altitude>,<UTCtime>,<TTFF>,<num>,<speed>,<course>
     * note:
            <TTFF>:time to first fix(in seconds)
            <num> :satellites in view for fix
     * example:$GPSIM,114.5,30.15,28.5,1461235600.123,3355,7,2.16,179.36
     */

    rc = eat_gps_nmea_info_output(EAT_NMEA_OUTPUT_SIMCOM,buf,1024);
    if(rc == EAT_FALSE)
    {
        LOG_ERROR("get gps error ,and erturn is %d",rc);
    }

    LOG_DEBUG("%s",buf);

    buf_gps = string_bypass(buf, "$GPSIM,");

    rc = sscanf(buf_gps,"%f,%f,%f,%lf,%*d,%d,%f,%f",\
        &latitude,&longitude,&altitude,&iGpstime,&satellite,&speed,&course);

    if(isTimeFixed((long long)iGpstime))
    {
        setting.main_gps_timer_period = 24 * 60 * 60 *1000;
        print("%f",iGpstime);
    }

    if(!rtc_synced())
    {
        rtc_update((long long)iGpstime);
    }

    if(longitude > 0 && latitude > 0)//get GPS
    {
        gps.isGPS = EAT_TRUE;
        gps.timestamp = rtc_getTimestamp();
        gps.latitude = latitude;
        gps.longitude = longitude;
    }
    else
    {
        gps.isGPS = EAT_FALSE;
    }
}

static int event_timer(const EatEvent_st* event)
{
    switch (event->data.timer.timer_id)
    {
        case TIMER_LOOP:
            LOG_DEBUG("TIMER_LOOP expire.");
            fsm_run(EVT_LOOP);
            eat_timer_start(event->data.timer.timer_id, setting.main_loop_timer_period);
            break;

        case TIMER_GPS:
            LOG_DEBUG("TIMER_GPS expire.");
            event_gps_proc();
            eat_timer_start(event->data.timer.timer_id, setting.main_gps_timer_period);
            break;

        default:
            LOG_ERROR ("timer(%d) not processed!", event->data.timer.timer_id);
            break;
    }
    return 0;
}

static int event_adc(const EatEvent_st* event)
{
    unsigned int value = event->data.adc.v;

    LOG_DEBUG("ad value=%d", value);

    if (event->data.adc.pin == 0)
    {
        return 0;
    }
    else
    {
        LOG_INFO("not processed adc pin:%d", event->data.adc.pin);
    }

    return 0;
}

static int event_mod_ready_rd(const EatEvent_st* event)
{
	u8 buf[256] = {0};
	u16 len = 0;

	len = eat_modem_read(buf, 256);
	if (!len)
	{
	    LOG_ERROR("modem received nothing.");
	    return -1;
	}
    LOG_DEBUG("modem recv: %s", buf);

    if (modem_IsCallReady(buf))
    {

        fsm_run(EVT_CALL_READY);
    }

	return 0;
}

static EVENT_PROC eventProcs[] =
{
    {EAT_EVENT_TIMER,               event_timer},
    {EAT_EVENT_MDM_READY_RD,        event_mod_ready_rd},
    {EAT_EVENT_MDM_READY_WR,        EAT_NULL},
    {EAT_EVENT_UART_READY_RD,       event_uart_ready_rd},
    {EAT_EVENT_UART_READY_WR,       event_uart_ready_wr},
    {EAT_EVENT_UART_SEND_COMPLETE,  EAT_NULL},
    {EAT_EVENT_USER_MSG,            EAT_NULL},
    {EAT_EVENT_ADC,                 event_adc},
};


int event_proc(EatEvent_st* event)
{
    int i = 0;


    for (i = 0; i < sizeof(eventProcs) / sizeof(eventProcs[0]); i++)
    {
        if (eventProcs[i].event == event->event)
        {
            EVENT_FUNC pfn = eventProcs[i].pfn;
            if (pfn)
            {
                return pfn(event);
            }
            else
            {
                LOG_ERROR("event(%s) not processed!", event->event);
                return -1;
            }
        }
    }
    return -1;
}
