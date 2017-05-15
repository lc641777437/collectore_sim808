/*
 * setting.h
 *
 *  Created on: 2016/7/26
 *      Author: lc
 */

#ifndef USER_ELECTROMBILE_SETTING_H_
#define USER_ELECTROMBILE_SETTING_H_

#include <eat_type.h>

#define SETTINGFILE_NAME L"C:\\set.conf"

#define MAX_DOMAIN_NAME_LEN 32

typedef enum
{
	ADDR_TYPE_IP,
	ADDR_TYPE_DOMAIN
}ADDR_TYPE;

#pragma anon_unions
typedef struct
{
	//Server configuration
	ADDR_TYPE addr_type;
	union
	{
		char domain[MAX_DOMAIN_NAME_LEN];
		u8 ipaddr[4];
	};
	u16 port;

	//Timer configuration
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
extern GPS gps;

eat_bool setting_restore(void);
eat_bool setting_save(void);

#endif /* USER_ELECTROMBILE_SETTING_H_ */
