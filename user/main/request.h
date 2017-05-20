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
void cmd_sendData(const unsigned char data[MAX_INFO_LEN]);

#endif
