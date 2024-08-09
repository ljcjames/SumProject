#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "infrared.h"
#include <drv_gpio.h>

#define DBG_TAG "main"
#define DBG_LVL DBG_LOG
#include <rtdbg.h>
#include <ulog.h>
#include <drv_lcd.h>

/* 配置 LED 灯引脚 */
#define PIN_LED_B GET_PIN(F, 11) // PF11 :  LED_B        --> LED
#define PIN_LED_R GET_PIN(F, 12) // PF12 :  LED_R        --> LED
#define PAGE_MAX 2

extern rt_atomic_t now_direction;
extern rt_atomic_t snake_pressed;
extern rt_atomic_t page_chosen;
extern rt_atomic_t page_first;
extern rt_atomic_t page_stop;
extern int snake_max;
extern char tmp[10];

void snake_compare(rt_uint8_t key, rt_uint8_t repeat)
{
    rt_sprintf(tmp, "%02X", key);
    rt_atomic_store(&snake_pressed, snake_max + 1);
    // 上
    if (rt_strcmp(tmp, "30") == 0 || rt_strcmp(tmp, "53") == 0)
        if (rt_atomic_load(&now_direction) != 2)
            rt_atomic_store(&now_direction, 0);
    // 左
    if (rt_strcmp(tmp, "E8") == 0 || rt_strcmp(tmp, "99") == 0)
        if (rt_atomic_load(&now_direction) != 3)
            rt_atomic_store(&now_direction, 1);
    // 下
    if (rt_strcmp(tmp, "B0") == 0 || rt_strcmp(tmp, "4B") == 0)
        if (rt_atomic_load(&now_direction) != 0)
            rt_atomic_store(&now_direction, 2);
    // 右
    if (rt_strcmp(tmp, "68") == 0 || rt_strcmp(tmp, "83") == 0)
        if (rt_atomic_load(&now_direction) != 1)
            rt_atomic_store(&now_direction, 3);
    // 菜单（切换页面）
    if (repeat == 0 && (rt_strcmp(tmp, "88") == 0 || rt_strcmp(tmp, "11") == 0))
    {
        page_chosen = (page_chosen % PAGE_MAX) + 1;
        page_first = 1;
        rt_kprintf("page_chosen = %d\n", page_chosen);
    }
    // 确认（暂停、页面冻结）
    if (repeat == 0 && (rt_strcmp(tmp, "73") == 0))
    {
        page_stop = (page_stop + 1) % 2;
        if (page_stop == 1)
        {
            lcd_show_string(240 - 24 * 3, 240 - 24, 24, "Stop");
        }
        else
        {
            lcd_show_string(240 - 24 * 3, 240 - 24, 24, "    ");
        }
    }
}

void myir_entry(void *parameter)
{
    rt_kprintf("myir_entry\n");
    unsigned int count = 1;
    rt_int16_t key;
    struct infrared_decoder_data infrared_data;

    /* 选择 NEC 解码器 */
    ir_select_decoder("nec");

    /* 设置 RGB 引脚为输出模式*/
    rt_pin_mode(PIN_LED_R, PIN_MODE_OUTPUT);
    rt_pin_mode(PIN_LED_B, PIN_MODE_OUTPUT);

    rt_pin_write(PIN_LED_R, PIN_HIGH);
    rt_pin_write(PIN_LED_B, PIN_HIGH);

    while (1)
    {
        if (infrared_read("nec", &infrared_data) == RT_EOK)
        {
            /* 读取到红外数据，红灯亮起 */
            rt_pin_write(PIN_LED_R, PIN_LOW);
            LOG_I("RECEIVE OK: addr:0x%02X key:0x%02X repeat:%d", infrared_data.data.nec.addr,
                  infrared_data.data.nec.key, infrared_data.data.nec.repeat);
            snake_compare(infrared_data.data.nec.key, infrared_data.data.nec.repeat);
        }
        rt_thread_mdelay(50);

        /* 熄灭蓝灯 */
        rt_pin_write(PIN_LED_B, PIN_HIGH);
        /* 熄灭红灯 */
        rt_pin_write(PIN_LED_R, PIN_HIGH);
        // if( count % 100 == 0)
        // rt_kprintf("count = %d\n", count);
    }
}
