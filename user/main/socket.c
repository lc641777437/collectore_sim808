/*
 * socket.c
 *
 *  Created on: 2016/7/26
 *      Author: lc
 */
#include <stdio.h>
#include <eat_interface.h>
#include <eat_socket.h>

#include "socket.h"
#include "setting.h"
#include "log.h"
#include "client.h"
#include "msg.h"
#include "timer.h"
#include "fsm.h"
#include "error.h"


static s8 socket_id = 0;

static u32 request_id = 0;

#define DESC_DEF(x)	case x:\
                        return #x

/*
SOC_READ    = 0x01,   Notify for read
SOC_WRITE   = 0x02,   Notify for write
SOC_ACCEPT  = 0x04,   Notify for accept
SOC_CONNECT = 0x08,   Notify for connect
SOC_CLOSE   = 0x10,   Notify for close
SOC_ACKED   = 0x20,   Notify for acked
*/
static char* getEventDescription(soc_event_enum event)
{
	switch (event)
	{
#ifdef APP_DEBUG
		DESC_DEF(SOC_READ);
		DESC_DEF(SOC_WRITE);
		DESC_DEF(SOC_ACCEPT);
		DESC_DEF(SOC_CONNECT);
		DESC_DEF(SOC_CLOSE);
		DESC_DEF(SOC_ACKED);
#endif
		default:
		{
			static char soc_event[10] = {0};
			snprintf(soc_event, 10, "%d", event);
			return soc_event;
		}
	}
}

/*
CBM_DEACTIVATED             = 0x01,  deactivated
CBM_ACTIVATING              = 0x02,  activating
CBM_ACTIVATED               = 0x04,  activated
CBM_DEACTIVATING            = 0x08,  deactivating
CBM_CSD_AUTO_DISC_TIMEOUT   = 0x10,  csd auto disconnection timeout
CBM_GPRS_AUTO_DISC_TIMEOUT  = 0x20,  gprs auto disconnection timeout
CBM_NWK_NEG_QOS_MODIFY      = 0x040,  negotiated network qos modify notification
CBM_WIFI_STA_INFO_MODIFY
*/
static char* getStateDescription(cbm_bearer_state_enum state)
{
	switch (state)
	{
#ifdef APP_DEBUG
		DESC_DEF(CBM_DEACTIVATED);
		DESC_DEF(CBM_ACTIVATING);
		DESC_DEF(CBM_ACTIVATED);
		DESC_DEF(CBM_DEACTIVATING);
		DESC_DEF(CBM_CSD_AUTO_DISC_TIMEOUT);
		DESC_DEF(CBM_GPRS_AUTO_DISC_TIMEOUT);
		DESC_DEF(CBM_NWK_NEG_QOS_MODIFY);
		DESC_DEF(CBM_WIFI_STA_INFO_MODIFY);
#endif
		default:
		{
			static char bearer_state[10] = {0};
			snprintf(bearer_state, 10, "%d", state);
			return bearer_state;
		}
	}
}

static void soc_notify_cb(s8 s,soc_event_enum event,eat_bool result, u16 ack_size)
{
    u8 buffer[1152] = {0};//1K + 128 for upgrade module
    s32 rc = 0;

    LOG_DEBUG("SOCKET notify:socketid(%d), event(%s).", s, getEventDescription(event));

    switch (event)
    {
        case SOC_READ:

            rc = eat_soc_recv(socket_id, buffer, 1152);//1K + 128 for upgrade module
            if (rc > 0)
            {
                client_proc(buffer, rc);
            }
            else
            {
                LOG_ERROR("eat_soc_recv error:rc=%d!", rc);
            }

            break;

        case SOC_CONNECT:
            if(result)
            {
                LOG_DEBUG("SOC_CONNECT success.");
                fsm_run(EVT_SOCKET_CONNECTED);
            }
            else
            {
                LOG_ERROR("SOC_CONNECT failed, maybe the server is OFF!");
                fsm_run(EVT_SOCKET_CONNECT_FAILED);
                eat_soc_close(s);
            }

            break;

        case SOC_CLOSE:
            LOG_INFO("SOC_CLOSE:socketid = %d", s);

            eat_soc_close(s);

            fsm_run(EVT_SOCKET_DISCONNECTED);

            break;

        case SOC_ACKED:
            LOG_DEBUG("acked size of send data: %d.", ack_size);
            break;

        default:
            LOG_INFO("SOC_NOTIFY %d not handled", event);
            break;
    }

}


int socket_connect(u8 ip_addr[4])
{
    s8 rc = SOC_SUCCESS;
    s8 val = EAT_TRUE;

    sockaddr_struct address = {SOC_SOCK_STREAM};

    eat_soc_notify_register(soc_notify_cb);
    socket_id = eat_soc_create(SOC_SOCK_STREAM, SOC_SOCK_STREAM);
    if (socket_id < 0)
    {
        LOG_ERROR("eat_soc_create return error :%d!", socket_id);
        return ERR_SOCKET_CREAT_FAILED;
    }
    else
    {
        LOG_DEBUG("eat_soc_create ok, socket_id = %d.", socket_id);
    }

    rc = eat_soc_setsockopt(socket_id, SOC_NBIO, &val, sizeof(val));
    if (rc != SOC_SUCCESS)
    {
        LOG_ERROR("eat_soc_setsockopt set SOC_NBIO failed: %d!", rc);
        return ERR_SOCKET_OPTION_FAILED;
    }

    rc = eat_soc_setsockopt(socket_id, SOC_NODELAY, &val, sizeof(val));
    if (rc != SOC_SUCCESS)
    {
        LOG_ERROR("eat_soc_setsockopt set SOC_NODELAY failed: %d!", rc);
        return ERR_SOCKET_OPTION_FAILED;
    }

    val = SOC_READ | SOC_WRITE | SOC_CLOSE | SOC_CONNECT;
    rc = eat_soc_setsockopt(socket_id, SOC_ASYNC, &val, sizeof(val));
    if (rc != SOC_SUCCESS)
    {
        LOG_ERROR("eat_soc_setsockopt set SOC_ASYNC failed: %d!", rc);
        return ERR_SOCKET_OPTION_FAILED;
    }



    address.sock_type = SOC_SOCK_STREAM;
    address.addr_len = 4;

    address.addr[0] = ip_addr[0];
    address.addr[1] = ip_addr[1];
    address.addr[2] = ip_addr[2];
    address.addr[3] = ip_addr[3];

    LOG_DEBUG("ip: %d.%d.%d.%d:%d.", address.addr[0], address.addr[1], address.addr[2], address.addr[3], setting.port);


    address.port = setting.port;                /* TCP server port */
    rc = eat_soc_connect(socket_id, &address);
    if(rc >= 0)
    {
        LOG_DEBUG("socket id of new connection is :%d.", rc);
        return ERR_SOCKET_CONNECTED;
    }
    else if (rc == SOC_WOULDBLOCK)
    {
        LOG_DEBUG("Connection is in progressing...");
        return ERR_SOCKET_WAITING;
    }
    else
    {
        LOG_ERROR("Connect return error:%d!", rc);
        return ERR_SOCKET_FAILED;
    }
}


static void hostname_notify_cb(u32 request_id, eat_bool result, u8 ip_addr[4])
{
	if (result == EAT_TRUE)
	{
		LOG_DEBUG("hostname notify:%s -> %d.%d.%d.%d.", setting.domain, ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3], setting.port);

		socket_connect(ip_addr); //TODO:this should be done in function action_hostname2ip
		fsm_run(EVT_HOSTNAME2IP);
	}
	else
	{
	    LOG_ERROR("hostname_notify_cb error: request_id = %d", request_id);
	    fsm_run(EVT_HOSTNAME2IP_FAILED);
	}

    return;
}


static void bear_notify_cb(cbm_bearer_state_enum state, u8 ip_addr[4])
{
    LOG_DEBUG("bear_notify state: %s.", getStateDescription(state));

	switch (state)
	{
        case CBM_ACTIVATED:
            LOG_DEBUG("local ip is %d.%d.%d.%d", ip_addr[0], ip_addr[1], ip_addr[2], ip_addr[3]);

		    fsm_run(EVT_BEARER_HOLD);
            break;

        case CBM_DEACTIVATED:
            fsm_run(EVT_BEARER_DEACTIVATED);
            break;

        case CBM_GPRS_AUTO_DISC_TIMEOUT:
            LOG_ERROR("CBM_GPRS_AUTO_DISC_TIMEOUT happened!");
            eat_reset_module();
            break;

        case CBM_ACTIVATING:
        case CBM_DEACTIVATING:
            //no need to handle
            break;

        default:
            LOG_ERROR("BEAR_NOTIFY %d not handled", state);
            break;
	}
}

int socket_init(void)
{
    s8 rc = eat_gprs_bearer_open("CMNET", NULL, NULL, bear_notify_cb);
    if (rc == CBM_WOULDBLOCK)
    {
        LOG_DEBUG("opening bearer...");
    }
    else if (rc == CBM_OK)
    {
        LOG_DEBUG("open bearer success.");

        rc = eat_gprs_bearer_hold();
        if (rc == CBM_OK)
        {
            LOG_DEBUG("hold bearer success.");

            return socket_setup();
        }
        else
        {
            LOG_ERROR("hold bearer failed!");
            return ERR_HOLD_BEARER_FAILED;
        }
    }
    else
    {
        LOG_ERROR("open bearer failed: rc = %d", rc);

        return ERR_OPEN_BEARER_FAILED;
    }

    return SUCCESS;
}

int socket_setup(void)
{

    s8 rc = SOC_SUCCESS;

    if (setting.addr_type == ADDR_TYPE_IP)
    {
        return socket_connect(setting.ipaddr);
    }
    else
    {
        u8 ipaddr[4] = {0};
        u8 len = 0;

        eat_soc_gethost_notify_register(hostname_notify_cb);
        rc = eat_soc_gethostbyname(setting.domain, ipaddr, &len, request_id++);
        if (rc == SOC_WOULDBLOCK)
        {
            LOG_DEBUG("eat_soc_gethostbyname wait callback.");
            return ERR_WAITING_HOSTNAME2IP;
        }
        else if (rc == SOC_SUCCESS)
        {
            LOG_DEBUG("host:%s -> %d.%d.%d.%d:%d.", setting.domain, ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3], setting.port);
            return socket_connect(ipaddr);
        }
        else
        {
            LOG_ERROR("eat_soc_gethostbyname error!");
            return ERR_GET_HOSTBYNAME_FAILED;
        }
    }
}

int socket_sendData(void* data, s32 len)
{
    s32 rc;

    LOG_HEX((const char*)data, len);

    rc = eat_soc_send(socket_id, data, len);
    if (rc >= 0)
    {
        LOG_DEBUG("socket send data successful.");
    }
    else
    {
        if (rc == SOC_PIPE || rc == SOC_NOTCONN)
        {
            fsm_run(EVT_SOCKET_DISCONNECTED);       //这个地方仅为保护作用，正常的socket断链应该会通过soc_notify_cb来通知
        }
        LOG_ERROR("sokcet send data failed:%d!", rc);
    }


    return rc;
}

s32 socket_sendDataDirectly(void* data, s32 len)
{
    s32 rc = socket_sendData(data, len);

    free_msg(data);

    return rc;
}

