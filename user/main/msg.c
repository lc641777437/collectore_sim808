/*
 * msg.c
 *
 *  Created on: 2016/7/26
 *      Author: lc
 */
#include <eat_interface.h>
#include <eat_mem.h>

#include "msg.h"

static char seq = 0;

void* alloc_msg(char cmd, size_t length)
{
    MSG_HEADER* msg = eat_mem_alloc(length);

    if (msg)
    {
        msg->signature = htons(START_FLAG);
        msg->cmd = cmd;
        msg->seq = seq++;
        msg->length = htons(length - MSG_HEADER_LEN);
    }

    return msg;
}

void* alloc_rspMsg(const MSG_HEADER* pMsg)
{
    MSG_HEADER* msg = NULL;
    int i = 0;
    size_t msgLen = 0;
    typedef struct
    {
        char cmd;
        size_t size;
    }MSG_LENGTH;


    MSG_LENGTH rsp_msg_length_map[] =
    {
            {CMD_DATA,0},
            {CMD_PING,0}
    };

    for (i = 0; i < sizeof(rsp_msg_length_map) / sizeof(rsp_msg_length_map[0]); i++)
    {
        if (pMsg->cmd == rsp_msg_length_map[i].cmd)
        {
            msgLen = rsp_msg_length_map[i].size;
        }
    }

    if (msgLen == 0)
    {
        return NULL;
    }

    msg = eat_mem_alloc(msgLen);
    if (!msg)
    {
        return msg;
    }
    msg->signature = htons(START_FLAG);
    msg->cmd = pMsg->cmd;
    msg->length = htons(msgLen - MSG_HEADER_LEN);
    msg->seq = pMsg->seq;

    return msg;
}


void free_msg(void* msg)
{
    eat_mem_free(msg);
}

