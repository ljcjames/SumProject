#include "rtthread.h"
#include "dev_sign_api.h"
#include "mqtt_api.h"
#include <board.h>
#include <drv_gpio.h>
#include <stdio.h>
#include <string.h>
#include "aht10.h"
#include <ap3216c.h>
#include <dfs_posix.h>
#include <drv_lcd.h>
#include "mysnake.h"
#include "infrared.h"
#include <ulog.h>
#include "my_func.h"

#define THREAD_PRIORITY 25
#define THREAD_STACK_SIZE 4096
#define THREAD_TIMESLICE 5

rt_thread_t MQTT_Thread = RT_NULL;
rt_thread_t Snake_Thread = RT_NULL;
rt_thread_t Infrared_Thread = RT_NULL;
rt_thread_t Test_Thread = RT_NULL;

char DEMO_PRODUCT_KEY[IOTX_PRODUCT_KEY_LEN + 1] = {0};
char DEMO_DEVICE_NAME[IOTX_DEVICE_NAME_LEN + 1] = {0};
char DEMO_DEVICE_SECRET[IOTX_DEVICE_SECRET_LEN + 1] = {0};

void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);
void HAL_Printf(const char *fmt, ...);
int HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN + 1]);
int HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN + 1]);
int HAL_GetDeviceSecret(char device_secret[IOTX_DEVICE_SECRET_LEN]);
uint64_t HAL_UptimeMs(void);
int HAL_Snprintf(char *str, const int len, const char *fmt, ...);

// 定义接受文件内容的缓冲区
char buffer[1026] = {};
char tmp[1026];
extern int snake_len;
rt_atomic_t page_chosen = 1;
rt_atomic_t page_first = 1;
rt_atomic_t page_stop = 0;
rt_atomic_t last_stop = 0;
rt_atomic_t mqtt_enable = 0;

void *pclient = NULL;

#define GPIO_LED_B GET_PIN(F, 11)
#define GPIO_LED_R GET_PIN(F, 12)

// AHT挂载的总线名字
#define AHT10_I2C_BUS "i2c3"

// AHT,ap3216c设备指针
aht10_device_t Dev = RT_NULL;
ap3216c_device_t dev;

// Humi:湿度值,Temp:温度值
float Humi, Temp;
rt_uint16_t ps_data;
float brightness;
int lcd_y;
int int_tmp;
struct infrared_decoder_data infrared_data;

extern void myir_entry(void *parameter);

void ath_init(void);
void mqt_init(void);
int ap3_init(void);
void inf_init(void);
void tst_init(void);

#define EXAMPLE_TRACE(fmt, ...)                        \
    do                                                 \
    {                                                  \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__);                \
        HAL_Printf("%s", "\r\n");                      \
    } while (0)

static void example_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_t *topic_info = (iotx_mqtt_topic_info_pt)msg->msg;

    switch (msg->event_type)
    {
    case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
        /* print topic name and topic message */
        EXAMPLE_TRACE("Message Arrived:");
        rt_pin_mode(GPIO_LED_R, PIN_MODE_OUTPUT);
        // topic_info->payload 为发送的内容，可以据此设置命令
        if (rt_pin_read(GPIO_LED_R) == PIN_HIGH)
        {
            // rt_kprintf("LED_R should be ON\n");
            rt_pin_write(GPIO_LED_R, PIN_LOW);
        }
        else
        {
            // rt_kprintf("LED_R should be OFF\n");
            rt_pin_write(GPIO_LED_R, PIN_HIGH);
        }
        EXAMPLE_TRACE("Topic  : %.*s", topic_info->topic_len, topic_info->ptopic);
        EXAMPLE_TRACE("Payload: %.*s", topic_info->payload_len, topic_info->payload);
        EXAMPLE_TRACE("\n");
        break;
    default:
        break;
    }
}

static int example_subscribe(void *handle)
{
    int res = 0;
    const char *fmt = "/%s/%s/user/get";
    char *topic = NULL;
    int topic_len = 0;
    
    topic_len = rt_strlen(fmt) + rt_strlen(DEMO_PRODUCT_KEY) + rt_strlen(DEMO_DEVICE_NAME) + 1;
    topic = rt_malloc(topic_len);
    if (topic == NULL)
    {
        EXAMPLE_TRACE("memory not enough");
        return -1;
    }
    memset(topic, 0, topic_len);
    // HAL_Snprintf(topic, topic_len, fmt, DEMO_PRODUCT_KEY, DEMO_DEVICE_NAME);
    rt_snprintf(topic, topic_len, fmt, DEMO_PRODUCT_KEY, DEMO_DEVICE_NAME);

    res = IOT_MQTT_Subscribe(handle, topic, IOTX_MQTT_QOS0, example_message_arrive, NULL);
    if (res < 0)
    {
        EXAMPLE_TRACE("subscribe failed");
        // HAL_Free(topic);
        rt_free(topic);
        return -1;
    }

    // HAL_Free(topic);
    rt_free(topic);
    return 0;
}
int plus_lcd_y(int pls)
{
    int_tmp = lcd_y;
    lcd_y += pls;
    return int_tmp;
}
void easy_show_lcd(char *title, float Temp)
{
    lcd_show_string(10, plus_lcd_y(24), 24, title);
    rt_sprintf(tmp, "%f", Temp);
    lcd_show_string(10, plus_lcd_y(32), 32, tmp);
}
void show_lcd()
{
    lcd_y = 10;
    easy_show_lcd("Temperature:", Temp);
    easy_show_lcd("Humidity:", Humi);
    easy_show_lcd("Brightness:(lux)", brightness);
    easy_show_lcd("Ps data:", (float)ps_data);
    // lcd_show_string(10, plus_lcd_y(10), 24, "Temperature:");
    // rt_sprintf(tmp, "%f", Temp);
    // lcd_show_string(10, plus_lcd_y(24), 32, tmp);

    // lcd_show_string(10, plus_lcd_y(32), 24, "Humidity:");
    // rt_sprintf(tmp, "%f", Humi);
    // lcd_show_string(10, plus_lcd_y(24), 32, tmp);

    // lcd_show_string(10, plus_lcd_y(32), 24, "Brightness:");
    // rt_sprintf(tmp, "%f(lux)", brightness);
    // lcd_show_string(10, plus_lcd_y(24), 32, tmp);
}

void make_file()
{
    // 文件描述符
    int fd;
    // 用只写方式打开文件,如果没有该文件,则创建一个文件
    fd = open("/fal/test/Data.txt", O_WRONLY | O_CREAT | O_APPEND); // 和原来相比，只是把O_TRUNC参数更改为O_APPEND，即更改为打开后，如果再进行写入，将从文件的末尾位置开始写。
    // rt_kprintf("\n%f %f tmp:%s\n",Humi,Temp,String);
    // 如果打开成功
    if (fd >= 0)
    {
        // 写入文件
        write(fd, tmp, sizeof(tmp));
        // rt_kprintf("Write done.\n");
        // 关闭文件
        close(fd);
    }
    else
    {
        rt_kprintf("File Open Fail.\n");
    }
    return;
}
int cnt = 0;
void tmp_payload(void)
{
    // 读取温湿度值
    Humi = aht10_read_humidity(Dev);
    Temp = aht10_read_temperature(Dev);
    brightness = ap3216c_read_ambient_light(dev);
    ps_data = ap3216c_read_ps_data(dev);
    // if (infrared_read("nec", &infrared_data) == RT_EOK)
    // {
    //     /* 读取到红外数据，红灯亮起 */
    //     // rt_pin_write(PIN_LED_R, PIN_LOW);
    //     LOG_I("RECEIVE OK: addr:0x%02X key:0x%02X repeat:%d", infrared_data.data.nec.addr,
    //           infrared_data.data.nec.key, infrared_data.data.nec.repeat);
    // }
    // icm20608_get_accel(icm20608_device_t dev, rt_int16_t *accel_x, rt_int16_t *accel_y, rt_int16_t *accel_z)
    // memset(tmp, 0, sizeof(tmp));
    // rt_sprintf(tmp, "Temp: %.1f;Humi: %.1f;Count: %d\n", Temp, Humi,++cnt);
    // rt_kprintf("\n%f %f tmp:%s\n",Humi,Temp,tmp);
    // make_file();
    if (page_chosen == 2 && !page_stop)
    {
        if (page_first)
        {
            // lcd_fill(40, 240/2-32-24, 240-50, 240/2+24+32, WHITE);
            rt_kprintf("page:Data\n");
            my_round(20);
            page_first = 0;
        }
        show_lcd();
    }
    if (page_chosen == 3 && !page_stop)
    {
        if (page_first)
        {
            lcd_fill(0, 0, 240, 240, WHITE);
            rt_kprintf("page:Time\n");
            my_round(20);
            page_first = 0;
        }
        greattime();
    }
    if (page_chosen == 4 && !page_stop)
    {
        if (page_first)
        {
            lcd_fill(0, 0, 240, 240, WHITE);
            lcd_show_string(240 / 2 - 24, 240 / 2 - 12, 24, "MQTT");
            rt_kprintf("page:MQTT\n");
            my_round(20);
            page_first = 0;
        }
        if (!mqtt_enable)
        {
            lcd_show_string(240 / 2 - 24 * 2, 240 / 2 + 12, 32, "disable");
            if (last_stop)
            {
                last_stop = 0;
                IOT_MQTT_Destroy(&pclient);
                rt_thread_delete(MQTT_Thread);
            }
        }
        else
        {
            lcd_show_string(240 / 2 - 24 * 2, 240 / 2 + 12, 32, "enable ");
            if (mqtt_enable && !last_stop)
            {
                last_stop = 1;
                mqt_init();
            }
        }
    }
    if (ps_data > 1022)
    {
        page_chosen = (page_chosen % PAGE_MAX) + 1;
        page_first = 1;
    }
    rt_sprintf(tmp, "{\"params\":{\"temperature\":%.2f,\"humidity\":%.2f,\"LightLux\":%.2f,\"Psdata\":%d,\"Snakelen\":%d}}", Temp, Humi, brightness, ps_data, snake_len);
    return;
}
void test_lcd()
{
    // show_str(10, 10+24+32+24+32, 100, 32, "你好", 32);
    // ath_init();
    // ap3_init();
    while (1)
    {
        tmp_payload();
        rt_thread_mdelay(100);
    }
}
MSH_CMD_EXPORT(test_lcd, run my project);
static int example_publish(void *handle)
{

    // tmp_payload();
    int res = 0;
    const char *fmt = "/sys/%s/%s/thing/event/property/post";
    // /k1lyriw1yGj/${deviceName}/user/get
    char *topic = NULL;
    int topic_len = 0;
    char *payload = tmp;
    // strcpy(payload,tmp_payload());
    // rt_kprintf("payload:%s\n",payload);
    topic_len = rt_strlen(fmt) + rt_strlen(DEMO_PRODUCT_KEY) + rt_strlen(DEMO_DEVICE_NAME) + 1;
    // topic = HAL_Malloc(topic_len);
    topic = rt_malloc(topic_len);
    if (topic == NULL)
    {
        EXAMPLE_TRACE("memory not enough");
        return -1;
    }
    memset(topic, 0, topic_len);
    // HAL_Snprintf(topic, topic_len, fmt, DEMO_PRODUCT_KEY, DEMO_DEVICE_NAME);
    rt_snprintf(topic, topic_len, fmt, DEMO_PRODUCT_KEY, DEMO_DEVICE_NAME);

    res = IOT_MQTT_Publish_Simple(0, topic, IOTX_MQTT_QOS0, payload, rt_strlen(payload));
    if (res < 0)
    {
        EXAMPLE_TRACE("publish failed, res = %d", res);
        // HAL_Free(topic);
        rt_free(topic);
        return -1;
    }

    // HAL_Free(topic);
    rt_free(topic);
    return 0;
}

static void example_event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    EXAMPLE_TRACE("msg->event_type : %d", msg->event_type);
}

static void mqtt_example_main(void *parameter)
{
    int res = 0;
    int loop_cnt = 0;
    iotx_mqtt_param_t mqtt_params;

    HAL_GetProductKey(DEMO_PRODUCT_KEY);
    HAL_GetDeviceName(DEMO_DEVICE_NAME);
    HAL_GetDeviceSecret(DEMO_DEVICE_SECRET);

    EXAMPLE_TRACE("mqtt example");

    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.handle_event.h_fp = example_event_handle;

    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient)
    {
        EXAMPLE_TRACE("MQTT construct failed");
        return;
    }

    res = example_subscribe(pclient);
    if (res < 0)
    {
        IOT_MQTT_Destroy(&pclient);
        return;
    }

    while (1)
    {
        if (0 == loop_cnt % 20)
        {
            example_publish(pclient);
        }

        IOT_MQTT_Yield(pclient, 200);

        loop_cnt += 1;
    }

    return;
}





void ath_init(void)
{
    // 初始化设备
    Dev = aht10_init(AHT10_I2C_BUS);
    if (Dev == RT_NULL)
    {
        rt_kprintf("AHT10_init Fail");
        return;
    }
}
void mqt_init(void)
{
    MQTT_Thread = rt_thread_create("MTQQ_Thread", mqtt_example_main, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);

    if (MQTT_Thread != RT_NULL)
    {
        rt_thread_startup(MQTT_Thread);
    }
    else
    {
        rt_kprintf("MQTT Thread Create Failed!\n");
    }
}
MSH_CMD_EXPORT_ALIAS(mqt_init, mqtt, run my mqtt);
int ap3_init(void)
{
    const char *i2c_bus_name = "i2c2";

    dev = ap3216c_init(i2c_bus_name);

    return 0;
}
void snk_init(void)
{
    Snake_Thread = rt_thread_create("Snake_Thread", snake_entry, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);

    if (Snake_Thread != RT_NULL)
    {
        rt_thread_startup(Snake_Thread);
    }
    else
    {
        rt_kprintf("Snake Thread Create Failed!\n");
    }
}
MSH_CMD_EXPORT_ALIAS(snk_init, snake, "snake game");
// void i20_init(void)
// {
//     const char* i2c_bus_name = "i2c2";

//     icm20608_init(i2c_bus_name)

//     return 0;

// }
void inf_init(void)
{
    Infrared_Thread = rt_thread_create("Infrared_Thread", myir_entry, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);

    if (Infrared_Thread != RT_NULL)
    {
        rt_thread_startup(Infrared_Thread);
    }
    else
    {
        rt_kprintf("Infrared Thread Create Failed!\n");
    }
}
MSH_CMD_EXPORT_ALIAS(inf_init, inf, "Infrared");
void tst_init(void)
{
    Test_Thread = rt_thread_create("Test_Thread", test_lcd, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);

    if (Test_Thread != RT_NULL)
    {
        rt_thread_startup(Test_Thread);
    }
    else
    {
        rt_kprintf("Test_Thread Create Failed!\n");
    }
}
MSH_CMD_EXPORT_ALIAS(tst_init, no_mqtt, "Infrared");
// void evn_init(void)
// {
//     if(rt_event_create("my_event2", RT_IPC_FLAG_FIFO) != RT_EOK)
//     {
//         rt_kprintf("Create Event Failed!\n");
//     }
// }
void my_project(void)
{
    // /* 选择 NEC 解码器 */
    // ir_select_decoder("nec");

    // evn_init();

    ath_init();

    // mqt_init();

    ap3_init();

    inf_init();

    tst_init(); // 不知道为什么不能在mqtt_init()之前，不然报错
}
MSH_CMD_EXPORT_ALIAS(my_project, myproject, run my project);