/*
 * client.h
 *
 *  Created on: 2016/7/26
 *      Author: lc
 */

#include <stdio.h>
#include <string.h>

#include <eat_interface.h>

#include "debug.h"
#include "log.h"
#include "rtc.h"
#include "utils.h"
#include "version.h"
#include "protocol.h"

#include "request.h"

#define MAX_CMD_LENGTH (16)
#define MAX_CMD_NUMBER  (32)

typedef struct
{
    unsigned char cmd[MAX_CMD_LENGTH];
    CMD_ACTION  action;
}CMD_MAP;

#define DBG_OUT(fmt, ...) eat_trace("[DEBUG]>"fmt, ##__VA_ARGS__)

static int cmd_imei(const unsigned char* cmdString, unsigned short length);
static int cmd_AT(const unsigned char* cmdString, unsigned short length);
static int cmd_sim808(const unsigned char* cmdString, unsigned short length);
static int cmd_reboot(const unsigned char* cmdString, unsigned short length);
static int cmd_rtc(const unsigned char* cmdString, unsigned short length);

static CMD_MAP cmd_map[MAX_CMD_NUMBER] =
{
    {"CT",          cmd_sim808},
    {"imei",        cmd_imei},
    {"reboot",      cmd_reboot},
    {"rtc",         cmd_rtc}
};

static int cmd_sim808(const unsigned char* cmdString, unsigned short length)
{
    LOG_DEBUG("%d", length);
    if(length >= MAX_INFO_LEN)
    {
        cmd_sendData(cmdString);
    }
    return 0;
}

static int cmd_imei(const unsigned char* cmdString, unsigned short length)
{
    u8 imei[32] = {0};
    eat_get_imei(imei, 31);
    DBG_OUT("IMEI = %s", imei);
    return 0;
 }

static int cmd_AT(const unsigned char* cmdString, unsigned short length)
{
    eat_modem_write(cmdString, length);
    eat_modem_write("\n", 1);
    return 0;
}

static int cmd_reboot(const unsigned char* cmdString, unsigned short length)
{
    eat_reset_module();
    return 0;
}

static int cmd_rtc(const unsigned char* cmdString, unsigned short length)
{
    EatRtc_st rtc = {0};
    const int RTC_BASE = 1954;

    eat_bool result = eat_get_rtc(&rtc);
    if (result)
    {
        DBG_OUT("RTC:%d-%02d-%02d %02d:%02d:%02d UTC, timestamp:%d", rtc.year + RTC_BASE, rtc.mon, rtc.day, rtc.hour, rtc.min, rtc.sec, rtc_getTimestamp());
    }
    else
    {
        LOG_ERROR("get rtc time failed:%d", result);
    }

    return 0;
}

int debug_proc(const unsigned char* cmdString, unsigned short length)
{
    int i = 0;
    int rc = 0;

    const unsigned char* cmd = string_trimLeft(cmdString);

    for (i = 0; i < MAX_CMD_NUMBER && cmd_map[i].action; i++)
    {
        rc = strncmp((const char *)cmd, (const char *)cmd_map[i].cmd, strlen((const char *)cmd_map[i].cmd));
        if (rc == 0)
        {
            return cmd_map[i].action(cmdString, length);
        }
    }

    LOG_INFO("CMD not processed");
    return 0;
}


int regist_cmd(const unsigned char* cmd, CMD_ACTION action)
{
    int i = 0;

    // get the count of cmd_map
    while (i < MAX_CMD_NUMBER && cmd_map[i].action) i++;

    if ( i >= MAX_CMD_NUMBER )
    {
        return -1;
    }

    strncpy((char *)cmd_map[i].cmd, (const char *)cmd, MAX_CMD_LENGTH);
    cmd_map[i].action = action;

    LOG_DEBUG("register cmd %s(%p) at position %d success", cmd, action, i);
    return 0;
}
