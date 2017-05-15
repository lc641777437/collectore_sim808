/*
 * setting.h
 *
 *  Created on: 2016/7/26
 *      Author: lc
 */

#ifndef USER_ELECTROMBILE_SETTING_H_
#define USER_ELECTROMBILE_SETTING_H_

#include <eat_interface.h>

#define SETTINGFILE_NAME L"C:\\setting.conf"

#define MAX_DOMAIN_NAME_LEN (32)

typedef enum
{
	ADDR_TYPE_IP,
	ADDR_TYPE_DOMAIN
}ADDR_TYPE;

#pragma anon_unions
typedef struct
{
	/* Server configuration */
	u16 port;
	ADDR_TYPE addr_type;
    u8 ipaddr[4];
    char domain[MAX_DOMAIN_NAME_LEN];

	/* Timer configuration */
    struct
    {
        u32 main_loop_timer_period;
        u32 main_gps_timer_period;
    };
}SETTING;

typedef struct
{
    eat_bool isGPS;
    int timestamp;
    float longitude;
    float latitude;
}GPS;

extern SETTING setting;

eat_bool setting_restore(void);
eat_bool setting_save(void);

void setting_saveGps(GPS gps);
GPS *setting_getGps(void);

#endif /* USER_ELECTROMBILE_SETTING_H_ */
