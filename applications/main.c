/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-5-10      ShiHao       first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "drv_common.h"
#include <ntp.h>
#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
#include "init.h"
#include <drv_lcd.h>
#include "my_func.h"
#include <drv_gpio.h>


/* 配置 LED 灯引脚 */
#define PIN_LED_B GET_PIN(F, 11) // PF11 :  LED_B        --> LED
#define PIN_LED_R GET_PIN(F, 12) // PF12 :  LED_R        --> LED





int main(void)
{   
    main_init();
    rt_pin_mode(PIN_LED_B, PIN_MODE_OUTPUT);
    rt_pin_write(PIN_LED_B, PIN_LOW);
    system("snake game");
    rt_wlan_config_autoreconnect(RT_TRUE);
    rt_wlan_connect("Dong", "abcd07691234");
    mytime();

    /* init Wi-Fi auto connect feature */
    // wlan_autoconnect_init();
    /* enable auto reconnect on WLAN device */

    // lcd_clear(WHITE);

    // /* show RT-Thread logo */
    // lcd_show_image(0, 0, 240, 69, image_rttlogo);

    // /* set the background color and foreground color */
    // lcd_set_color(WHITE, BLACK);

    // /* show some string on lcd */
    // lcd_show_string(10, 69, 16, "Hello, RT-Thread!");
    // lcd_show_string(10, 69 + 16, 24, "RT-Thread");
    // lcd_show_string(10, 69 + 16 + 24, 32, "RT-Thread");

    // /* draw a line on lcd */
    // lcd_draw_line(0, 69 + 16 + 24 + 32, 240, 69 + 16 + 24 + 32);

    // /* draw a concentric circles */
    // lcd_draw_point(120, 194);
    // for (int i = 0; i < 46; i += 4)
    // {
    //     lcd_draw_circle(120, 194, i);
    // }
    return 0;
}
// #include <rtthread.h>
// #include <board.h>
// #include "init.h"
// #include <drv_gpio.h>
// #ifndef RT_USING_NANO
// #include <rtdevice.h>
// #endif // !RT_USING_NANO

// #define GPIO_LED_B GET_PIN(F,11)
// #define GPIO_LED_R GET_PIN(F,12)

// int main(void)
// {
//     main_init();
//     // rt_pin_mode(GPIO_LED_B, PIN_MODE_OUTPUT);
//     // while(1)
//     // {
//     //     rt_pin_write(GPIO_LED_B, PIN_HIGH);
//     //     rt_thread_mdelay(500);
//     //     rt_pin_write(GPIO_LED_B, PIN_LOW);
//     //     rt_thread_mdelay(500);
//     // }
//     return 0;
// }

// static int onboard_sdcard_mount(void)
// {
//     if (dfs_mount("sd0", "/sdcard", "elm", 0, 0) == RT_EOK)
//     {
//         LOG_I("SD card mount to '/sdcard'");
//     }
//     else
//     {
//         LOG_E("SD card mount to '/sdcard' failed!");
//     }

//     return RT_EOK;
// }

// #define WIFI_CS GET_PIN(F, 10)
// void WIFI_CS_PULL_DOWM(void)
// {
//     rt_pin_mode(WIFI_CS, PIN_MODE_OUTPUT);
//     rt_pin_write(WIFI_CS, PIN_LOW);
// }
// INIT_BOARD_EXPORT(WIFI_CS_PULL_DOWM);
