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


void free_msg(void* msg)
{
    eat_mem_free(msg);
}

