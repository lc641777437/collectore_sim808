/*
 * client.c
 *
 *  Created on: 2016/7/26
 *      Author: lc
 */

#include <stdio.h>
#include <string.h>


#include <eat_interface.h>

#include "protocol.h"
#include "client.h"
#include "socket.h"
#include "msg.h"
#include "log.h"
#include "uart.h"
#include "setting.h"
#include "timer.h"
#include "fsm.h"


typedef int (*MSG_PROC)(const void* msg);       //TODO: add the message length parameter
typedef struct
{
    char cmd;
    MSG_PROC pfn;
}MC_MSG_PROC;

static int cmd_Login_rsp(const void* msg)
{
    LOG_DEBUG("get login respond.");

    fsm_run(EVT_LOGINED);

    return 0;
}

static int cmd_Ping_rsp(const void* msg)
{
    LOG_DEBUG("get ping respond.");

    return 0;
}

static int cmd_Info_rsp(const void* msg)
{
    LOG_DEBUG("get ping respond.");

    return 0;
}


static MC_MSG_PROC msgProcs[] =
{
    {CMD_LOGIN,cmd_Login_rsp},
    {CMD_PING,cmd_Ping_rsp},
    {CMD_INFO,cmd_Info_rsp},
};

int client_handleOnePkt(const void* m, int msgLen)
{
    MSG_HEADER* msg = (MSG_HEADER*)m;
    size_t i = 0;

    for (i = 0; i < sizeof(msgProcs) / sizeof(msgProcs[0]); i++)
    {
        if (msgProcs[i].cmd == msg->cmd)
        {
            MSG_PROC pfn = msgProcs[i].pfn;
            if (pfn)
            {
                return pfn(msg);
            }
            else
            {
                LOG_ERROR("Message %d not processed!", msg->cmd);
                return -1;
            }
        }
    }

    LOG_ERROR("unknown message %d!", msg->cmd);
    return -1;
}

int client_proc(const void *m, int msgLen)
{
    const MSG_HEADER *msg = (const MSG_HEADER *)m;
    size_t leftLen = 0;

    LOG_HEX(m, msgLen);

    if(msgLen < MSG_HEADER_LEN)
    {
        LOG_ERROR("message length not enough: %zu(at least(%zu)", msgLen, sizeof(MSG_HEADER));
        return -1;
    }

    leftLen = msgLen;
    while(leftLen >= ntohs(msg->length) + MSG_HEADER_LEN)
    {
        if (ntohs(msg->signature) != START_FLAG)        {
            LOG_ERROR("receive message header signature error:%#x", (unsigned)ntohs(msg->signature));
            return -1;
        }

        client_handleOnePkt(msg, ntohs(msg->length) + MSG_HEADER_LEN);
        leftLen = leftLen - MSG_HEADER_LEN - ntohs(msg->length);
        msg = (const MSG_HEADER *)((const char *)m + msgLen - leftLen);
    }
    return 0;
}

