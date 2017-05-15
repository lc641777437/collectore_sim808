/*
 * watchdog.c
 *
 *  Created on: 2016/7/26
 *      Author: lc
 */

#include <eat_interface.h>
#include <eat_timer.h>

#include "watchdog.h"
#include "log.h"

#define REBOOT_TIMEOUT  5 * 60 * 1000   // 5 minutes

void startWatchdog(void)
{
	eat_bool rc = eat_watchdog_start(REBOOT_TIMEOUT, 0); //reboot if over time
    if(rc)
    {
        LOG_DEBUG("open watchdog success.");
    }
    else
    {
        LOG_ERROR("open watchdog fail:%d!", rc);
    }

    return;
}

void stopWatchdog(void)
{
	eat_bool rc = eat_watchdog_stop();
    if(!rc)
    {
        LOG_DEBUG("stop watchdog success.");
    }
    else
    {
        LOG_ERROR("stop watchdog fail: %d!", rc);
    }

    return;
}

void feedWatchdog(void)
{
	eat_bool rc = eat_watchdog_feed();
    if(!rc )
    {
        LOG_DEBUG("feed watchdog success.");
    }
    else
    {
        LOG_ERROR("feed watchdog fail:%d!", rc);
    }

    return;
}

