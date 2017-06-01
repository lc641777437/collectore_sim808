/*
 * log.h
 *
 *  Created on: 2016/7/26
 *      Author: lc
 */

#ifndef USER_ELECTROMBILE_LOG_H_
#define USER_ELECTROMBILE_LOG_H_

#include <eat_interface.h>
#include "rtc.h"

#ifdef APP_DEBUG
#define LOG_DEBUG(fmt, ...) eat_trace("[%d][DBG][%s:%d %s]"fmt, eat_get_task_id(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) eat_trace("[%d][INF][%s:%d %s]"fmt, eat_get_task_id(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) eat_trace("[%d][ERR][%s:%d %s]"fmt, eat_get_task_id(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_HEX(data, len)  log_hex(data, len);
#else
#define LOG_DEBUG(fmt, ...)
#define LOG_INFO(fmt, ...)
#define LOG_ERROR(fmt, ...)
#define LOG_HEX(data, len)
#endif

void log_hex(const char* data, int length);

#endif /* USER_ELECTROMBILE_LOG_H_ */

