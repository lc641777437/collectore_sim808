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
#include "uart.h"
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

void cmd_sendData(const unsigned char *data, int length)
{
    GPS *gps = setting_getGps();
    u8 imei[MAX_IMEI_LENGTH + 1] = {0};
    MSG_DATA_REQ *msg = (MSG_DATA_REQ *)alloc_msg(CMD_DATA, sizeof(MSG_DATA_REQ));
    if(!msg)
    {
        LOG_ERROR("malloc failed");
        return;
    }

    msg->isGps = gps->isGPS;
    eat_get_imei(imei, MAX_IMEI_LENGTH + 1);
    memcpy(msg->imei, imei, MAX_IMEI_LENGTH);
    msg->timestamp = htonl(rtc_getTimestamp());
    if(gps->isGPS) {
        msg->latitude = gps->latitude;
        msg->longitude = gps->longitude;
    }else {
        msg->latitude = 0.0;
        msg->longitude = 0.0;
    }
    memcpy(msg->data, data, AD_DATA_LEN);

    socket_sendDataDirectly(msg, sizeof(MSG_DATA_REQ));
    return;
}

void cmd_sendDataDynamic(const unsigned char *data, int length)
{
    u8 imei[MAX_IMEI_LENGTH + 1] = {0};
    MSG_DATADYNAMIC_REQ *msg = (MSG_DATADYNAMIC_REQ *)alloc_msg(CMD_DYNAMIC, sizeof(MSG_DATADYNAMIC_REQ));
    if(!msg)
    {
        LOG_ERROR("malloc failed");
        return;
    }

    eat_get_imei(imei, MAX_IMEI_LENGTH);
    memcpy(msg->imei, imei, MAX_IMEI_LENGTH);
    memcpy(msg->data, data, AD_DATADYNAMIC_LEN);

    socket_sendDataDirectly(msg, sizeof(MSG_DATADYNAMIC_REQ));
    return;
}


static void pc_set_server(const unsigned char *data)
{
    int port;
    int count;
    u32 ip[4] = {0};

    count = sscanf((const char *)data,"%u.%u.%u.%u:%d",&ip[0],&ip[1],&ip[2],&ip[3],&port);
    if(5 == count)
    {
        send_message_stm("\xA6\xA6\x07", 3);
        send_message_stm("\r\n", 2);
        setting.addr_type = ADDR_TYPE_IP;
        setting.ipaddr[0] = (u8)ip[0];
        setting.ipaddr[1] = (u8)ip[1];
        setting.ipaddr[2] = (u8)ip[2];
        setting.ipaddr[3] = (u8)ip[3];
        setting.port = (u16)port;

        socket_init();
        //eat_reset_module();
    }
    else
    {
        char domain[MAX_DOMAIN_NAME_LEN] = {0};
        count = sscanf((const char *)data, "%[^:]:%d", domain, &port);
        if(2 == count)
        {
            send_message_stm("\xA6\xA6\x07", 3);
            send_message_stm("\r\n", 2);
            setting.addr_type = ADDR_TYPE_DOMAIN;
            strncpy(setting.domain, domain,MAX_DOMAIN_NAME_LEN);
            setting.port = port;

            setting_save();
            socket_init();
        }
        else
        {
            send_message_stm("\xA6\xA6\x99", 3);
            send_message_stm("\r\n", 2);
        }
    }
}
void cmd_setResponse(const unsigned char *data, int length)
{
    int len = 0;
    MSG_SET_RSP *msg = NULL;

    switch(data[0])
    {
        case SIM808_SET_SERVER:
            pc_set_server(data + 1);
            break;

        case SIM808_SET_TIMESTAMP:
            break;

        default:
            len = sizeof(MSG_SET_RSP) + length;
            msg = (MSG_SET_RSP *)alloc_msg(CMD_SET, len);
            if(!msg)
            {
                LOG_ERROR("malloc failed");
                return;
            }
            memcpy(msg->data, data, length);
            socket_sendDataDirectly(msg, len);
            break;
    }
    return;
}

