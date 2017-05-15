/*
 * debug.h
 *
 *  Created on: 2016年2月4日
 *      Author: jk
 */

#ifndef USER_INC_DEBUG_H_
#define USER_INC_DEBUG_H_

typedef int (*CMD_ACTION)(const unsigned char* cmdString, unsigned short length);
int regist_cmd(const unsigned char* cmd, CMD_ACTION action);

int debug_proc(const unsigned char* cmdString, unsigned short length);

#endif /* USER_INC_DEBUG_H_ */
