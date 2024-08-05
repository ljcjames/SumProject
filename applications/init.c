#include <rtthread.h>
#include <rthw.h>
#include <wlan_mgnt.h>
#include <wlan_cfg.h>
#include <wlan_prot.h>
#include <ap3216c.h>
#include "my_func.h"

static int board_init(void)
{
    
    return 0;
}
INIT_BOARD_EXPORT(board_init);

static int prev_init(void)
{
    
    return 0;
}
INIT_PREV_EXPORT(prev_init);

static int device_init(void)
{

    return 0;
}
INIT_DEVICE_EXPORT(device_init);

static int component_init(void)
{
    
    return 0;
}

INIT_COMPONENT_EXPORT(component_init);
static int env_init(void)
{
    
    return 0;
}
INIT_ENV_EXPORT(env_init);

static int app_init(void)
{
    return 0;
}
INIT_APP_EXPORT(app_init);
    // extern int wifi_join(int argc, char *argv[]);
int main_init(void)
{
    
    char str[] = "wifi join Dong abcd07691234";
    my_round(20);
    system(str);
    // rt_thread_mdelay(18000);
    // char *argv[] = {"wifi", "join", "Dong", "abcd07691234"};
    // wifi_join(4, argv);
    // char *ssid = "Dong";
    // char *key = "abcd07691234";
    // rt_wlan_connect(ssid, key);
    return 0;
}
