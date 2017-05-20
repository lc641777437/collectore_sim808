/*
 * request.c
 *
 *  Created on: 2016/11/8
 *      Author: lc
 */
#include <string.h>

#include <eat_interface.h>

#include "rtc.h"
#include "log.h"
#include "msg.h"
#include "utils.h"
#include "socket.h"
#include "request.h"
#include "setting.h"
#include "version.h"
#include "protocol.h"

int cmd_Login(void)
{
    u8 imei[MAX_IMEI_LENGTH + 1] = {0};
    MSG_LOGIN_REQ* msg = alloc_msg(CMD_LOGIN, sizeof(MSG_LOGIN_REQ));

    if (!msg)
    {
        LOG_ERROR("alloc login message failed!");
        return -1;
    }

    eat_get_imei(imei, MAX_IMEI_LENGTH);
    memcpy(msg->imei, imei, MAX_IMEI_LENGTH);

    LOG_DEBUG("send login message.");

    socket_sendDataDirectly(msg, sizeof(MSG_LOGIN_REQ));
    return 0;
}


void cmd_Heartbeat(void)
{
    u8 msgLen = sizeof(MSG_PING_REQ);
    u8 imei[MAX_IMEI_LENGTH + 1] = {0};
    MSG_PING_REQ *msg = alloc_msg(CMD_PING, msgLen);

    eat_get_imei(imei, MAX_IMEI_LENGTH);
    memcpy(msg->imei, imei, MAX_IMEI_LENGTH);

    LOG_DEBUG("send heartbeat message.");
    socket_sendDataDirectly(msg, msgLen);
}

void cmd_sendData(const unsigned char *data)
{
    MSG_DATA_REQ *msg = NULL;
    u8 imei[MAX_IMEI_LENGTH + 1] = {0};
    GPS *gps = setting_getGps();

    msg = (MSG_DATA_REQ *)alloc_msg(CMD_DATA, sizeof(MSG_DATA_REQ));
    if(!msg)
    {
        LOG_ERROR("malloc failed");
        return;
    }

    msg->isGps = gps->isGPS;
    eat_get_imei(imei, MAX_IMEI_LENGTH);
    memcpy(msg->imei, imei, MAX_IMEI_LENGTH);
    msg->timestamp = htonl(rtc_getTimestamp());

    if(gps->isGPS)
    {
        msg->latitude = gps->latitude;
        msg->longitude = gps->longitude;
    }
    else
    {
        msg->latitude = 0.0;
        msg->longitude = 0.0;
    }

    memcpy(msg->data, data, AD_DATA_LEN);
    socket_sendDataDirectly(msg, sizeof(MSG_DATA_REQ));
    return;
}

void cmd_sendDataDynamic(const unsigned char *data)
{
    MSG_DATADYNAMIC_REQ *msg = NULL;
    u8 imei[MAX_IMEI_LENGTH + 1] = {0};

    msg = (MSG_DATADYNAMIC_REQ *)alloc_msg(CMD_DYNAMIC, sizeof(MSG_DATADYNAMIC_REQ));
    if(!msg)
    {
        LOG_ERROR("malloc failed");
        return;
    }

    eat_get_imei(imei, MAX_IMEI_LENGTH);
    memcpy(msg->imei, imei, MAX_IMEI_LENGTH);
    memcpy(msg->data, data, AD_DATADYNAMIC_LEN);

    socket_sendDataDirectly(msg, sizeof(MSG_DATA_REQ));
    return;
}

