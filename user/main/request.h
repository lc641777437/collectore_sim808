/*
 * request.h
 *
 *  Created on: 2016/11/8
 *      Author: lc
 */

#ifndef USER_ELECTROMBILE_REQUEST_H_
#define USER_ELECTROMBILE_REQUEST_H_

int cmd_Login(void);
void cmd_Heartbeat(void);
void cmd_sendInfo(const unsigned char info[148]);

#endif
