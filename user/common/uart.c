/*
 * uart.c
 *
 *  Created on: 2016/7/26
 *      Author: lc
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <eat_interface.h>
#include <eat_uart.h>
#include <eat_modem.h>

#include "uart.h"
#include "debug.h"
#include "log.h"


UART_WRITER* uart_writer = 0;
void uart_setWrite(UART_WRITER writer)
{
    uart_writer = writer;
}

int event_uart_ready_rd(const EatEvent_st* event)
{
	u16 length = 0;
	EatUart_enum uart = event->data.uart.uart;
	unsigned char buf[1024] = {0};	//TODO: use macro
	length = eat_uart_read(uart, buf, 1024);
	if (length)
	{
		buf[length] = 0;
	}
	else
	{
		return 0;
	}
	debug_proc(buf, length);

	return 0;
}

int event_uart_ready_wr(const EatEvent_st* event)
{
    if (uart_writer)
    {
        uart_writer();
    }
    else
    {
        LOG_INFO("uart wr event not handled");
    }

    return 0;
}

/*
 * print message via UART
 *
 * Note:
 *      the message length is limited to 1024 bytes
 */
int print(const char* fmt, ...)
{
    char buf[1024] = {0};
    int length = 0;

    va_list arg;
    va_start (arg, fmt);
    vsnprintf(buf, 1024, fmt, arg);
    va_end (arg);

    length = strlen(buf);

    return eat_uart_write(EAT_UART_1,(const unsigned char *)buf, length);
}
