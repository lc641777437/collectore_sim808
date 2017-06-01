/*
 * setting.c
 *
 *  Created on: 2016/7/26
 *      Author: lc
 */
#include <stdio.h>
#include <string.h>

#include <eat_interface.h>

#include "mem.h"
#include "log.h"
#include "cJSON.h"
#include "utils.h"
#include "setting.h"
#include "version.h"

static GPS position;
SETTING setting;

//for debug command
#define CMD_STRING_SERVER   "server"

//for JSON tag
#define TAG_SERVER      "SERVER"
#define TAG_ADDR_TYPE   "ADDR_TYPE"
#define TAG_ADDR        "ADDR"
#define TAG_PORT        "PORT"

static void setting_initial(void)
{
    cJSON_Hooks mem_hooks;

    mem_hooks.malloc_fn = malloc;
    mem_hooks.free_fn = free;

    LOG_DEBUG("setting initial to default value.");

    //initial the cJSON memory hook
    cJSON_InitHooks(&mem_hooks);

    /* Server configuration */
#if 1
    setting.addr_type = ADDR_TYPE_DOMAIN;
    strncpy(setting.domain, "test.xiaoan110.com",MAX_DOMAIN_NAME_LEN);
#else
    setting.addr_type = ADDR_TYPE_IP;
    setting.ipaddr[0] = 121;
    setting.ipaddr[1] = 42;
    setting.ipaddr[2] = 38;
    setting.ipaddr[3] = 93;
#endif

    setting.port = 9881;

    /* Timer configuration */
    setting.main_loop_timer_period = 5 * 1000;// 5s as normal
    setting.main_gps_timer_period = 5 * 1000;// 5s as initial, if OK, 1 hour as normal;
    return;
}

eat_bool setting_restore(void)
{
    int rc;
    FS_HANDLE fh;
    char *buf = 0;
    cJSON *conf = 0;
    cJSON *addr = 0;
    UINT filesize = 0;

    setting_initial();

    LOG_DEBUG("restore setting from file");

    fh = eat_fs_Open(SETTINGFILE_NAME, FS_READ_ONLY);
    if(EAT_FS_FILE_NOT_FOUND == fh)
    {
        LOG_INFO("setting file not exists.");
        return EAT_TRUE;
    }

    if (fh < EAT_FS_NO_ERROR)
    {
        LOG_ERROR("read setting file fail, rc: %d", fh);
        return EAT_FALSE;
    }

    rc = eat_fs_GetFileSize(fh, &filesize);
    if(EAT_FS_NO_ERROR != rc)
    {
        LOG_ERROR("get file size error, return %d",rc);
        eat_fs_Close(fh);
        return EAT_FALSE;
    }
    else
    {
        LOG_DEBUG("file size %d",filesize);
    }

    buf = malloc(filesize);
    if (!buf)
    {
        LOG_ERROR("malloc file content buffer failed");
        eat_fs_Close(fh);
        return EAT_FALSE;
    }
    else
    {
        LOG_DEBUG("malloc %d bytes for read setting", filesize);
    }

    rc = eat_fs_Read(fh, buf, filesize, NULL);
    if (rc != EAT_FS_NO_ERROR)
    {
        LOG_ERROR("read file fail, and return error: %d", fh);
        eat_fs_Close(fh);
        free(buf);
        return EAT_FALSE;
    }

    conf = cJSON_Parse(buf);
    if (!conf)
    {
        LOG_ERROR("setting config file format error!");
        eat_fs_Close(fh);
        free(buf);
        cJSON_Delete(conf);
        return EAT_FALSE;
    }

    addr = cJSON_GetObjectItem(conf, TAG_SERVER);
    if (!addr)
    {
        LOG_ERROR("no server config in setting file!");
        eat_fs_Close(fh);
        free(buf);
        cJSON_Delete(conf);
        return EAT_FALSE;
    }
    setting.addr_type = cJSON_GetObjectItem(addr, TAG_ADDR_TYPE)->valueint?ADDR_TYPE_DOMAIN:ADDR_TYPE_IP;
    if (setting.addr_type == ADDR_TYPE_DOMAIN)
    {
        char *domain = cJSON_GetObjectItem(addr, TAG_ADDR)->valuestring;
        LOG_DEBUG("restore domain name");
        strncpy(setting.domain, domain, MAX_DOMAIN_NAME_LEN);
    }
    else
    {
        char *ipaddr = cJSON_GetObjectItem(addr, TAG_ADDR)->valuestring;
        int ip[4] = {0};
        int count = sscanf(ipaddr, "%u.%u.%u.%u", ip, ip + 1, ip + 2, ip + 3);

        LOG_DEBUG("restore ip address");
        if (count != 4) // 4 means got four number of ip
        {
            LOG_ERROR("restore ip address failed");
            eat_fs_Close(fh);
            free(buf);
            cJSON_Delete(conf);
            return EAT_FALSE;
        }
        setting.ipaddr[0] = ip[0];
        setting.ipaddr[1] = ip[1];
        setting.ipaddr[2] = ip[2];
        setting.ipaddr[3] = ip[3];

    }

    setting.port = cJSON_GetObjectItem(addr, TAG_PORT)->valueint;

    free(buf);
    eat_fs_Close(fh);
    cJSON_Delete(conf);

    return EAT_TRUE;
}


eat_bool setting_save(void)
{
    FS_HANDLE fh, rc;
    eat_bool ret = EAT_FALSE;

    char *content = 0;
    cJSON *root = cJSON_CreateObject();
    cJSON *address = cJSON_CreateObject();

    cJSON_AddNumberToObject(address, TAG_ADDR_TYPE, setting.addr_type);
    if (setting.addr_type == ADDR_TYPE_DOMAIN)
    {
        cJSON_AddStringToObject(address, "ADDR", setting.domain);
    }
    else
    {
        char server[MAX_DOMAIN_NAME_LEN] = {0};
        snprintf(server, MAX_DOMAIN_NAME_LEN, "%d.%d.%d.%d", setting.ipaddr[0], setting.ipaddr[1], setting.ipaddr[2], setting.ipaddr[3]);
        cJSON_AddStringToObject(address, TAG_ADDR, server);
    }
    cJSON_AddNumberToObject(address, TAG_PORT, setting.port);

    cJSON_AddItemToObject(root, TAG_SERVER, address);

    content = cJSON_Print(root);
    LOG_DEBUG("save setting...");

    fh = eat_fs_Open(SETTINGFILE_NAME, FS_READ_WRITE|FS_CREATE);
    if(EAT_FS_NO_ERROR <= fh)
    {
        LOG_DEBUG("open file success, fh=%d.", fh);

        rc = eat_fs_Write(fh, content, strlen(content), 0);
        if(EAT_FS_NO_ERROR == rc)
        {
            LOG_DEBUG("write file success.");
        }
        else
        {
            LOG_ERROR("write file failed, and Return Error is %d", rc);
        }
    }
    else
    {
        LOG_ERROR("open file failed, fh=%d.", fh);
    }

    free(content);
    cJSON_Delete(root);
    eat_fs_Close(fh);

    return ret;
}

void setting_saveGps(GPS gps)
{
    position = gps;
}

GPS *setting_getGps(void)
{
    return &position;
}


