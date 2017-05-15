/*
 * protocol.h
 *
 *  Created on: 2016-11-8
 *      Author: lc
 *
 */

#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

enum
{
    PROTOCOL_VESION_251 = 215,
};

#define PROTOCOL_VERSION    PROTOCOL_VESION_251

#define START_FLAG (0xAA66)
#define MAX_IMEI_LENGTH 15
#define MAX_CCID_LENGTH 20
#define MAX_IMSI_LENGTH 15
#define TEL_NO_LENGTH 11
#define MAX_INFO_LEN 148


enum
{
    CMD_LOGIN           = 0,
    CMD_PING            = 1,
    CMD_INFO            = 2,
};

enum
{
    MSG_SUCCESS = 0,
    MSG_VERSION_NOT_SUPPORTED = -1,
    MSG_DISK_NO_SPACE = -2,
    MSG_UPGRADE_CHECKSUM_FAILED = -3,
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
    int version;
    char deciveType;
    char IMEI[MAX_IMEI_LENGTH];
}__attribute__((__packed__)) MSG_LOGIN_REQ;

typedef MSG_HEADER MSG_LOGIN_RSP;


/*
 * ping message structure
 */
typedef struct
{
    MSG_HEADER header;
    short status;   //TODO: to define the status bits
}__attribute__((__packed__)) MSG_PING_REQ;

typedef MSG_HEADER MSG_PING_RSP;


/*
 * CMD_COLLECTOR_INFO message structure
 */

typedef struct
{
    MSG_HEADER header;
    char imei[MAX_IMEI_LENGTH];
    int timestamp;
    float longitude;
    float latitude;
    char isGps;
    char info[MAX_INFO_LEN];
}__attribute__((__packed__)) MSG_COLLECTOR_INFO;

typedef MSG_HEADER MSG_COLLECTOR_INFO_RSP;

#pragma pack(pop)

#endif /* _PROTOCOL_H_ */
