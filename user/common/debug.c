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

static int cmd_debug(const unsigned char* cmdString, unsigned short length);
static int cmd_version(const unsigned char* cmdString, unsigned short length);
static int cmd_imei(const unsigned char* cmdString, unsigned short length);
static int cmd_imsi(const unsigned char* cmdString, unsigned short length);
static int cmd_chipid(const unsigned char* cmdString, unsigned short length);
static int cmd_AT(const unsigned char* cmdString, unsigned short length);
static int cmd_info(const unsigned char* cmdString, unsigned short length);
static int cmd_reboot(const unsigned char* cmdString, unsigned short length);
static int cmd_halt(const unsigned char* cmdString, unsigned short length);
static int cmd_rtc(const unsigned char* cmdString, unsigned short length);

static CMD_MAP cmd_map[MAX_CMD_NUMBER] =
{
    {"version",     cmd_version},
    {"imei",        cmd_imei},
    {"imsi",        cmd_imsi},
    {"chipid",      cmd_chipid},
    {"AT",          cmd_AT},
    {"at",          cmd_AT},
    {"reboot",      cmd_reboot},
    {"halt",        cmd_halt},
    {"rtc",         cmd_rtc},
    {"CT",          cmd_info}
};

static int cmd_info(const unsigned char* cmdString, unsigned short length)
{
    LOG_DEBUG("%d", length);
    if(length >= MAX_INFO_LEN)
    {
        cmd_sendInfo(cmdString);
    }
    return 0;
}

static int cmd_debug(const unsigned char* cmdString, unsigned short length)
{
    int i = 0;
    DBG_OUT("support cmd:");
    for (i = 0; i < MAX_CMD_NUMBER && cmd_map[i].action; i++)
    {
        DBG_OUT("\t%s\t%p", cmd_map[i].cmd, cmd_map[i].action);
    }

    return 0;
}

static int cmd_version(const unsigned char* cmdString, unsigned short length)
{
    DBG_OUT("version:%s(%s %s) core:%s(buildNo:%s@%s)", VERSION_STR, __DATE__, __TIME__, eat_get_version(), eat_get_buildno(), eat_get_buildtime());
    return 0;
}

static int cmd_imei(const unsigned char* cmdString, unsigned short length)
{
    u8 imei[32] = {0};
    eat_get_imei(imei, 31);
    DBG_OUT("IMEI = %s", imei);
    return 0;
 }

static int cmd_imsi(const unsigned char* cmdString, unsigned short length)
{
    u8 imsi[32] = {0};
    eat_get_imsi(imsi, 31);
    DBG_OUT("IMSI = %s", imsi);
    return 0;

}

static int cmd_chipid(const unsigned char* cmdString, unsigned short length)
{
#define MAX_CHIPID_LEN  16
    char chipid[MAX_CHIPID_LEN + 1] = {0};
    char chipid_desc[MAX_CHIPID_LEN * 2 + 1] = {0}; //hex character
    int i = 0; //loop var

    eat_get_chipid(chipid, MAX_CHIPID_LEN);
    for (i = 0; i < MAX_CHIPID_LEN; i++)
    {
        snprintf(chipid_desc + i * 2, 2, "%02X", chipid[i]);
    }
    DBG_OUT("chipd = %s", chipid_desc);
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

static int cmd_halt(const unsigned char* cmdString, unsigned short length)
{
    eat_power_down();
    return 0;
}

static int cmd_rtc(const unsigned char* cmdString, unsigned short length)
{
    const int RTC_BASE = 1954;
    EatRtc_st rtc = {0};
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
        rc = strncmp(cmd, cmd_map[i].cmd, strlen(cmd_map[i].cmd));
        LOG_DEBUG("%d",rc);
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

    //寻找第一个空位命令
    while (i < MAX_CMD_NUMBER && cmd_map[i].action) i++;

    if ( i >= MAX_CMD_NUMBER)
    {
        LOG_ERROR("exceed MAX command number: %d", MAX_CMD_NUMBER);
        return -1;
    }

    strncpy(cmd_map[i].cmd, cmd, MAX_CMD_LENGTH);
    cmd_map[i].action = action;

    LOG_DEBUG("register cmd %s(%p) at position %d success", cmd, action, i);
    return 0;
}
