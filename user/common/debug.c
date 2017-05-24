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

static int cmd_Data_Send(const unsigned char* cmdString, unsigned short length)
{
    LOG_DEBUG("%d", length);
    if(length >= AD_DATA_LEN)
    {
        cmd_sendData(cmdString + 2, AD_DATA_LEN);//"CT"
    }
    return 0;
}

static int cmd_DataDynamic_Send(const unsigned char* cmdString, unsigned short length)
{
    LOG_DEBUG("%d", length);
    if(length >= AD_DATADYNAMIC_LEN)
    {
        cmd_sendDataDynamic(cmdString + 2, AD_DATADYNAMIC_LEN);//"DT"
    }
    return 0;
}

static int cmd_SetResponse_Send(const unsigned char* cmdString, unsigned short length)
{
    LOG_DEBUG("%d", length);
    cmd_setResponse(cmdString + 2, length - 2);//"ST"
    return 0;
}

static int cmd_imei(const unsigned char* cmdString, unsigned short length)
{
    u8 imei[MAX_IMEI_LENGTH + 1] = {0};
    eat_get_imei(imei, MAX_IMEI_LENGTH + 1);
    DBG_OUT("IMEI = %s", imei);
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

static CMD_MAP cmd_map[MAX_CMD_NUMBER] =
{
    {"CT",          cmd_Data_Send},
    {"DT",          cmd_DataDynamic_Send},
    {"ST",          cmd_SetResponse_Send},
    {"imei",        cmd_imei},
    {"reboot",      cmd_reboot},
    {"rtc",         cmd_rtc}
};

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

