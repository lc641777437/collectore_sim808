/*
 * request.h
 *
 *  Created on: 2016/11/8
 *      Author: lc
 */

#ifndef USER_ELECTROMBILE_REQUEST_H_
#define USER_ELECTROMBILE_REQUEST_H_
#include "protocol.h"
int cmd_Login(void);
void cmd_Heartbeat(void);
void cmd_sendData(const unsigned char *data, int length);
void cmd_sendDataDynamic(const unsigned char *data, int length);
void cmd_setResponse(const unsigned char *data, int length);

typedef enum{
    SIM808_SET_PARAM    = 0X01,
    SIM808_GET_PARAM,
    SIM808_START_DYNAMIC,
    SIM808_STOP_DYNAMIC,
    SIM808_SET_FACTORY,
    SIM808_SET_TIMESTAMP,
    SIM808_SET_SERVER,


    SIM808_SET_ERROR    = 0X99
} CMD_SIM808_TYPE;

#endif
