/*
 * uart.h
 *
 *  Created on: 2016/7/26
 *      Author: lc
 */

#ifndef USER_ELECTROMBILE_UART_H_
#define USER_ELECTROMBILE_UART_H_

#include <eat_interface.h>

int event_uart_ready_rd(const EatEvent_st* event);
int event_uart_ready_wr(const EatEvent_st* event);

int print(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

typedef int UART_WRITER(void);
void uart_setWrite(UART_WRITER writer);

#endif /* USER_ELECTROMBILE_UART_H_ */
