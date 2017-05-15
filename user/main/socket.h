/*
 * socket.h
 *
 *  Created on: 2016/7/26
 *      Author: lc
 */

#ifndef USER_ELECTROMBILE_SOCKET_H_
#define USER_ELECTROMBILE_SOCKET_H_

typedef int (*MSG_RESEND_FAILED_HANDLER)(void* msg, int length, void* userdata);

int socket_init(void);
int socket_setup(void);

int socket_sendDataDirectly(void* data, int len);


#endif /* USER_ELECTROMBILE_SOCKET_H_ */
