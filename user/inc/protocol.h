/*
 * protocol.h
 *
 *  Created on: 2016-11-8
 *      Author: lc
 *
 */

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

#define START_FLAG (0xAA66)
#define MAX_IMEI_LENGTH (15)

#define AD_DATA_LEN (146)
#define AD_DATADYNAMIC_LEN (52)

enum
{
    CMD_LOGIN   = 0,
    CMD_PING    = 1,
    CMD_SERVER,
    CMD_DATA,
    CMD_DYNAMIC,
    CMD_SET
};

#pragma pack(push, 1)
/*
 * Message header definition
 */
typedef struct
{
    short signature;
    char cmd;
    char seq;
    short length;
}__attribute__((__packed__)) MSG_HEADER;
#define MSG_HEADER_LEN sizeof(MSG_HEADER)

/*
 * Login message structure
 */
typedef struct
{
    MSG_HEADER header;
    char imei[MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MSG_LOGIN_REQ;
typedef MSG_HEADER MSG_LOGIN_RSP;


/*
 * ping message structure
 */
typedef struct
{
    MSG_HEADER header;
    char imei[MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MSG_PING_REQ;
typedef MSG_HEADER MSG_PING_RSP;

/*
*server set_ip/domain message structure
*this message has no response
*/
typedef struct
{
    MSG_HEADER header;
    int port;
    char server[];
}__attribute__((__packed__)) MSG_SET_SERVER;

/*
 * MSG_DATA_REQ message structure
 */
typedef struct
{
    MSG_HEADER header;
    char imei[MAX_IMEI_LENGTH];
    char isGps;
    int timestamp;
    float longitude;
    float latitude;
    char data[AD_DATA_LEN];
}__attribute__((__packed__)) MSG_DATA_REQ;
typedef MSG_HEADER MSG_DATA_RSP;

/*
 * MSG_DATADYNAMIC_REQ message structure
 */
typedef struct
{
    MSG_HEADER header;
    char imei[MAX_IMEI_LENGTH];
    char data[AD_DATADYNAMIC_LEN];
}__attribute__((__packed__)) MSG_DATADYNAMIC_REQ;
typedef MSG_HEADER MSG_DATADYNAMIC_RSP;


/*
 * MSG_SET_REQ message structure
 */
typedef struct
{
    MSG_HEADER header;
    char data[];
}__attribute__((__packed__)) MSG_SET_REQ;

/*
 * MSG_SET_REQ message structure
 */
typedef struct
{
    MSG_HEADER header;
    char data[];
}__attribute__((__packed__)) MSG_SET_RSP;


#pragma pack(pop)
#endif /* _PROTOCOL_H_ */
