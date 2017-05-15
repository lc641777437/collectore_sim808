/*
 * modem.c
 *
 *  Created on: 2016/7/26
 *      Author: lc
 */

#include <string.h>
#include <stdio.h>

#include <eat_interface.h>
#include <eat_modem.h>
#include <eat_type.h>
#include <eat_network.h>

#include "modem.h"
#include "log.h"

#define MODEM_TEST_CMD  "=?"
#define MODEM_READ_CMD  "?"
#define MODEM_WRITE_CMD "="

#define CR  "\r"    //CR (carriage return)
#define LF  "\n"    //LF (line feed - new line)


#define AT_CGATT    "AT+CGATT"
#define AT_CENG     "AT+CENG"
#define AT_CGNSINF  "AT+CGNSINF"
#define AT_CCID     "AT+CCID"


eat_bool modem_cmd(const unsigned char *cmd)
{
    unsigned short len = strlen((const char *)cmd);
    unsigned short rc = eat_modem_write(cmd, len);

    if(rc != len)
    {
        LOG_ERROR("modem write failed: should be %d, but %d", len, rc);
        return EAT_FALSE;
    }
    else
    {
        return EAT_TRUE;
    }
}


eat_bool modem_IsCallReady(char* modem_rsp)
{
    char* ptr = strstr((const char *) modem_rsp, "Call Ready");

    if (ptr)
    {
        return EAT_TRUE;
    }

    return EAT_FALSE;
}

eat_bool modem_IsCCIDOK(char* modem_rsp)
{
    char* ptr = strstr((const char *) modem_rsp, "AT+CCID");

    if (ptr)
    {
        return EAT_TRUE;
    }

    return EAT_FALSE;
}

eat_bool modem_GPRSAttach(void)
{
    int rc = eat_network_get_creg();

    if (rc == EAT_REG_STATE_REGISTERED)
    {
        return eat_network_get_cgatt()?EAT_TRUE:EAT_FALSE;
    }
    else
    {
        LOG_DEBUG("network register status: %d", rc);
    }

    return EAT_FALSE;

}


/*
 * Write Command AT+CENG=<mode>[,<Ncell>]
 * Response
 * Switch on or off engineering mode. It will report +CENG: (network information) automatically if <mode>=2.
 *      OK
 *      ERROR
 *
 * Parameters
 * <mode>   0  Switch off engineering mode
 *          1  Switch on engineering mode
 *          2  Switch on engineering mode, and activate the URC report of network information
 *          3 Switch on engineering mode, with limited network information
 *          4 Switch on engineering mode, with extern information
 * <Ncell>  0 Un-display neighbor cell ID
 *          1 Display neighbor cell ID
 *  If <mode> = 3, ignore this parameter.
 *
 *  refer to AT command manual for detail
 */
eat_bool modem_switchEngineeringMode(int mode, int Ncell)
{
    unsigned char cmd[32] = {0};

    snprintf((char *)cmd, 32, "%s%s%d,%d\r", AT_CENG, MODEM_WRITE_CMD, mode, Ncell);

    return modem_cmd(cmd);
}

eat_bool modem_readCellInfo(void)
{
    unsigned char* cmd = AT_CENG MODEM_READ_CMD CR;


    return modem_cmd(cmd);
}

eat_bool modem_readCCIDInfo(void)
{
    unsigned char* cmd = AT_CCID CR;


    return modem_cmd(cmd);
}


eat_bool modem_GNSS(void)
{
    unsigned char* cmd = AT_CGNSINF CR;

    return modem_cmd(cmd);
}

eat_bool modem_AT(unsigned char *cmd)
{
    return modem_cmd(cmd);
}

