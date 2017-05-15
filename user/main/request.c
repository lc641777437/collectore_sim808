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
    MSG_LOGIN_REQ* msg = alloc_msg(CMD_LOGIN, sizeof(MSG_LOGIN_REQ));
    u8 imei[MAX_IMEI_LENGTH] = {0};

    if (!msg)
    {
        LOG_ERROR("alloc login message failed!");
        return -1;
    }

    msg->version = htonl(VERSION_NUM);

    eat_get_imei(imei, MAX_IMEI_LENGTH);

    memcpy(msg->IMEI, imei, MAX_IMEI_LENGTH);

    LOG_DEBUG("send login message.");

    socket_sendDataDirectly(msg, sizeof(MSG_LOGIN_REQ));

    return 0;
}


void cmd_Heartbeat(void)
{
    u8 msgLen = sizeof(MSG_HEADER) + sizeof(short);
    MSG_PING_REQ* msg = alloc_msg(CMD_PING, msgLen);
    msg->status = htons(EAT_TRUE);   //TODO: to define the status bits

    socket_sendDataDirectly(msg, msgLen);
}

void cmd_sendInfo(const unsigned char info[148])
{
    MSG_COLLECTOR_INFO *msg = NULL;
    u8 imei[MAX_IMEI_LENGTH + 1] = {0};
    GPS *gps = setting_getGps();

    msg = (MSG_COLLECTOR_INFO *)alloc_msg(CMD_INFO, sizeof(MSG_COLLECTOR_INFO));
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
    memcpy(msg->info, info, 148);
    socket_sendDataDirectly(msg, sizeof(MSG_COLLECTOR_INFO));
    return;
}

