/*
 * event.c
 *
 *  Created on: 2016/7/26
 *      Author: lc
 */
#include <string.h>

#include <eat_interface.h>
#include <eat_gps.h>

#include "log.h"
#include "msg.h"
#include "fsm.h"
#include "mem.h"
#include "uart.h"
#include "modem.h"
#include "utils.h"
#include "timer.h"
#include "client.h"
#include "socket.h"
#include "setting.h"
#include "request.h"

typedef int (*EVENT_FUNC)(const EatEvent_st* event);
typedef struct
{
	EatEvent_enum event;
	EVENT_FUNC pfn;
}EVENT_PROC;

#define DESC_DEF(x) case x:\
                            return #x
static void event_gps_proc(void)
{
    int rc;
    GPS gps = {EAT_FALSE, 0, 0.0, 0.0};
    unsigned char* buf_gps = NULL;
    unsigned char buf[MAX_READ_LEN] = {0};

    int satellite = 0;
    double iGpstime = 0.0;
    float latitude = 0.0, longitude = 0.0, altitude = 0.0, speed = 0.0, course = 0.0;

    /*
     * the output format of eat_gps_nmea_info_output
     * $GPSIM,<latitude>,<longitude>,<altitude>,<UTCtime>,<TTFF>,<num>,<speed>,<course>
     * note:
            <TTFF>:time to first fix(in seconds)
            <num> :satellites in view for fix
     * example:$GPSIM,114.5,30.15,28.5,1461235600.123,3355,7,2.16,179.36
     */
    rc = eat_gps_nmea_info_output(EAT_NMEA_OUTPUT_SIMCOM, (char *)buf, MAX_READ_LEN);
    if(rc == EAT_FALSE)
    {
        LOG_ERROR("get gps error, result is %d", rc);
    }

    LOG_DEBUG("%s", buf);

    buf_gps = (u8 *)string_bypass((const char *)buf, "$GPSIM,");

    rc = sscanf((const char *)buf_gps, "%f,%f,%f,%lf,%*d,%d,%f,%f",
        &latitude, &longitude, &altitude, &iGpstime, &satellite, &speed, &course);

    rtc_update((long long)iGpstime);

    if(longitude > 0 && latitude > 0 && rc == 7)//get GPS
    {
        gps.isGPS = EAT_TRUE;
        gps.timestamp = rtc_getTimestamp();
        gps.latitude = latitude;
        gps.longitude = longitude;
        setting_saveGps(&gps);
    }
    else
    {
        gps.isGPS = EAT_FALSE;
    }
}

static void event_upgradeTime_proc(void)
{
    char buf[32] = {0};
    GPS *position = setting_getGps();
    if(position->isGPS)
    {
        u32 timestamp = rtc_getTimestamp();
        buf[0] = 0XA6;
        buf[1] = 0XA6;
        buf[2] = SIM808_SET_TIMESTAMP;
        buf[3] = timestamp & 0X000000FF;
        timestamp >>= 8;
        buf[4] = timestamp & 0X000000FF;
        timestamp >>= 8;
        buf[5] = timestamp & 0X000000FF;
        timestamp >>= 8;
        buf[6] = timestamp & 0X000000FF;
        send_message_stm(buf, 7);
        send_message_stm("\r\n", 2);
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

        case TIMER_UPGRADETIME:
            event_upgradeTime_proc();
            eat_timer_start(event->data.timer.timer_id, setting.main_upgrade_time_period);
            break;

        default:
            LOG_ERROR ("timer(%d) not processed!", event->data.timer.timer_id);
            break;
    }
    return 0;
}

static int event_mod_ready_rd(const EatEvent_st* event)
{
	u16 len = 0;
	u8 buf[MAX_READ_LEN] = {0};

	len = eat_modem_read(buf, MAX_READ_LEN);
	if (!len)
	{
	    LOG_ERROR("modem received nothing.");
	    return -1;
	}

    LOG_DEBUG("modem recv: %s", buf);

    if (modem_IsCallReady((char *)buf))
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
    {EAT_EVENT_ADC,                 EAT_NULL},
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

            LOG_ERROR("event(%s) not processed!", event->event);
            break;
        }
    }
    return -1;
}

