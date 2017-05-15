/*
 * fsm.c
 *
 *  Created on: 2016/7/26
 *      Author: lc
 */

#include <stdio.h>

#include <eat_timer.h>

#include "fsm.h"
#include "log.h"
#include "timer.h"
#include "setting.h"
#include "error.h"
#include "modem.h"
#include "request.h"
#include "socket.h"
#include "watchdog.h"


typedef int ACTION(void);

typedef struct
{
    STATE state;
    EVENT event;
    ACTION  *action;
}STATE_TRANSITIONS;


typedef void (*STATE_TRIGGER)(void);

static void fsm_intoRunning(void)
{
    modem_readCCIDInfo();
}

static void fsm_exitRunning(void)
{
}


//TODO: 如果后期状态触发处理较多的话，以下数组可以改为链表，并采用注册回掉函数的机制
static STATE_TRIGGER state_in_trigger[STATE_MAX] =
{
        NULL,               //    STATE_INITIAL
        NULL,               //    STATE_WAIT_GPRS
        NULL,               //    STATE_WAIT_BEARER
        NULL,               //    STATE_WAIT_SOCKET
        NULL,               //    STATE_WAIT_IPADDR
        NULL,               //    STATE_WAIT_LOGIN
        fsm_intoRunning,    //    STATE_RUNNING
};

static STATE_TRIGGER state_out_trigger[STATE_MAX] =
{
        NULL,               //    STATE_INITIAL
        NULL,               //    STATE_WAIT_GPRS
        NULL,               //    STATE_WAIT_BEARER
        NULL,               //    STATE_WAIT_SOCKET
        NULL,               //    STATE_WAIT_IPADDR
        NULL,               //    STATE_WAIT_LOGIN
        fsm_exitRunning,    //    STATE_RUNNING
};

#define DESC_DEF(x) case x:\
                        return #x

static STATE current_state = STATE_INITIAL;

static char* fsm_getStateName(STATE state)
{
    switch (state)
    {
#ifdef APP_DEBUG
        DESC_DEF(STATE_INITIAL);
        DESC_DEF(STATE_WAIT_GPRS);
        DESC_DEF(STATE_WAIT_BEARER);
        DESC_DEF(STATE_WAIT_SOCKET);
        DESC_DEF(STATE_WAIT_IPADDR);
        DESC_DEF(STATE_WAIT_LOGIN);
        DESC_DEF(STATE_RUNNING);
#endif
        default:
        {
            static char state_name[10] = {0};
            snprintf(state_name, 10, "%d", state);
            return state_name;
        }
    }
}

static char* fsm_getEventName(EVENT event)
{
    switch (event)
    {
#ifdef APP_DEBUG
        DESC_DEF(EVT_LOOP);
        DESC_DEF(EVT_CALL_READY);
        DESC_DEF(EVT_GPRS_ATTACHED);
        DESC_DEF(EVT_BEARER_HOLD);
        DESC_DEF(EVT_HOSTNAME2IP);
        DESC_DEF(EVT_SOCKET_CONNECTED);
        DESC_DEF(EVT_LOGINED);
        DESC_DEF(EVT_HEARTBEAT_LOSE);
        DESC_DEF(EVT_SOCKET_DISCONNECTED);
        DESC_DEF(EVT_BEARER_DEACTIVATED);
        DESC_DEF(EVT_HOSTNAME2IP_FAILED);
#endif
        default:
        {
            static char event_name[10] = {0};
            snprintf(event_name, 10, "%d", event);
            return event_name;
        }
    }
}

static void fsm_stateEnterTrigger(STATE state)
{
    STATE_TRIGGER trigger = state_in_trigger[state];

    if (trigger)
    {
        trigger();
    }
}

static void fsm_stateExitTrigger(STATE state)
{
    STATE_TRIGGER trigger = state_out_trigger[state];

    if (trigger)
    {
        trigger();
    }
}

static void fsm_trans(STATE state)
{
    LOG_DEBUG("state: %s -> %s", fsm_getStateName(current_state), fsm_getStateName(state));

    fsm_stateExitTrigger(current_state);

    current_state = state;

    fsm_stateEnterTrigger(current_state);
}

static void start_mainloop(void)
{
    eat_timer_start(TIMER_LOOP, setting.main_loop_timer_period);
}


static int action_onCallReady(void)
{
    if (modem_GPRSAttach())
    {
        LOG_DEBUG("gprs attach success");
        fsm_run(EVT_GPRS_ATTACHED);
    }
    else
    {
        fsm_trans(STATE_WAIT_GPRS);
    }


    start_mainloop();

    return 0;
}

/*
 * eat_gprs_bearer_open
 * eat_soc_gethostbyname
 * eat_soc_connect
 *
 */
static int action_onGprsAttached(void)
{
    int rc = socket_init();

    switch (rc)
    {
    case SUCCESS:
        fsm_trans(STATE_WAIT_BEARER);
        break;

    case ERR_SOCKET_WAITING:
        fsm_trans(STATE_WAIT_SOCKET);
        break;

    case ERR_SOCKET_CONNECTED:
        cmd_Login();
        fsm_trans(STATE_WAIT_LOGIN);
        break;

    case ERR_WAITING_HOSTNAME2IP:
        fsm_trans(STATE_WAIT_IPADDR);
        break;

    default:
        //状态不变，继续attach GPRS
        break;

    }

    return 0;

}


static int action_onBearHold(void)
{
    int rc = socket_setup();

    switch (rc)
    {
    case ERR_SOCKET_WAITING:
        fsm_trans(STATE_WAIT_SOCKET);
        break;

    case ERR_WAITING_HOSTNAME2IP:
        fsm_trans(STATE_WAIT_IPADDR);
        break;

    case ERR_SOCKET_CONNECTED:
        cmd_Login();
        fsm_trans(STATE_WAIT_LOGIN);

        break;
    default:
        break;
    }

    return 0;
}

static int action_onBearDeactivated(void)
{
    fsm_trans(STATE_WAIT_GPRS);

    return 0;
}

static int action_onSocketConnected(void)
{
	cmd_Login();

    fsm_trans(STATE_WAIT_LOGIN);

    return 0;
}

static int action_onSocketConnectFailed(void)
{
    fsm_trans(STATE_WAIT_GPRS);
    return 0;
}

static int action_onLogined(void)
{
    fsm_trans(STATE_RUNNING);

    return 0;
}

static int action_onDNS(void)
{
    fsm_trans(STATE_WAIT_SOCKET);

    return 0;
}

static int action_onDNSFailed(void)
{
    fsm_trans(STATE_WAIT_GPRS);
    return 0;
}


static int action_onSocketDisconnected(void)
{
    cmd_Login();
    fsm_trans(STATE_WAIT_GPRS);
    return 0;
}

static int action_waitGprsOnLoop(void)
{
    if (modem_GPRSAttach())
    {
        fsm_run(EVT_GPRS_ATTACHED);
    }

    return 0;
}

static int action_runningOnLoop(void)
{
    static unsigned int heartbeat_times = 1;
    static unsigned int watchdog_times = 1;

    if (heartbeat_times++ % (3 * 60 / 5) == 0)  // the loop timer is 5 seconds, the heartbeat timer is 3 minutes
    {
        cmd_Heartbeat();
    }

    if (watchdog_times++ % (50 / 5) == 0)   // the loop timer is 5 seconrd, the watch dog timer is 50 seconds
    {
        feedWatchdog();
    }

    return 0;
}

static int action_waitloginOnLoop(void)
{
    static unsigned int login_times = 0;

    if (login_times++ > 5)
    {
        login_times = 0;
        cmd_Login();
    }

    return 0;
}

STATE_TRANSITIONS state_transitions[] =
{
    {STATE_INITIAL,     EVT_CALL_READY,             action_onCallReady},
    {STATE_WAIT_GPRS,   EVT_LOOP,                   action_waitGprsOnLoop},
    {STATE_WAIT_GPRS,   EVT_GPRS_ATTACHED,          action_onGprsAttached},
    {STATE_INITIAL,     EVT_GPRS_ATTACHED,          action_onGprsAttached},
    {STATE_WAIT_BEARER, EVT_BEARER_HOLD,            action_onBearHold},
    {STATE_WAIT_BEARER, EVT_BEARER_DEACTIVATED,     action_onBearDeactivated},
    {STATE_WAIT_SOCKET, EVT_SOCKET_CONNECTED,       action_onSocketConnected},
    {STATE_WAIT_SOCKET, EVT_SOCKET_CONNECT_FAILED,  action_onSocketConnectFailed},
    {STATE_WAIT_IPADDR, EVT_HOSTNAME2IP,            action_onDNS},
    {STATE_WAIT_IPADDR, EVT_HOSTNAME2IP_FAILED,     action_onDNSFailed},
    {STATE_WAIT_LOGIN,  EVT_LOGINED,                action_onLogined},
    {STATE_WAIT_LOGIN,  EVT_SOCKET_DISCONNECTED,    action_onSocketDisconnected},
    {STATE_WAIT_LOGIN,  EVT_LOOP,                   action_waitloginOnLoop},
    {STATE_RUNNING,     EVT_LOOP,                   action_runningOnLoop},
    {STATE_RUNNING,     EVT_BEARER_DEACTIVATED,     action_onBearDeactivated},
    {STATE_RUNNING,     EVT_SOCKET_DISCONNECTED,    action_onSocketDisconnected},
};

int fsm_run(EVENT event)
{
    int i = 0;
    ACTION* action = 0;

    for (i = 0; i < sizeof(state_transitions) / sizeof(state_transitions[0]); i++)
    {
        if (state_transitions[i].state == current_state && state_transitions[i].event == event)
        {
            action = state_transitions[i].action;
        }
    }

    LOG_DEBUG("run FSM State(%s), Event(%s), handler(%p)", fsm_getStateName(current_state), fsm_getEventName(event), action);
    if (action)
    {
        return action();
    }

    return 0;
}

