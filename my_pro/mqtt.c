#include "rtthread.h"
#include "dev_sign_api.h"
#include "mqtt_api.h"
#include <board.h>
#include <drv_gpio.h>
#include <stdio.h>
#include <string.h>
#include "aht10.h"
#include <dfs_posix.h>

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

//定义接受文件内容的缓冲区
char buffer[1026] = {};
char tmp[1026];

#define GPIO_LED_B GET_PIN(F,11)
#define GPIO_LED_R GET_PIN(F,12)

// AHT挂载的总线名字
#define AHT10_I2C_BUS "i2c3"

    // AHT设备指针
    aht10_device_t Dev = RT_NULL;

    // Humi:湿度值,Temp:温度值
    float Humi, Temp;


#define EXAMPLE_TRACE(fmt, ...)  \
    do { \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
    } while(0)

static void example_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_t     *topic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
            /* print topic name and topic message */
            EXAMPLE_TRACE("Message Arrived:");
            rt_pin_mode(GPIO_LED_R, PIN_MODE_OUTPUT);
            //topic_info->payload 为发送的内容，可以据此设置命令
            if(rt_pin_read(GPIO_LED_R) == PIN_HIGH)
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

    topic_len = strlen(fmt) + strlen(DEMO_PRODUCT_KEY) + strlen(DEMO_DEVICE_NAME) + 1;
    topic = HAL_Malloc(topic_len);
    if (topic == NULL) {
        EXAMPLE_TRACE("memory not enough");
        return -1;
    }
    memset(topic, 0, topic_len);
    HAL_Snprintf(topic, topic_len, fmt, DEMO_PRODUCT_KEY, DEMO_DEVICE_NAME);

    res = IOT_MQTT_Subscribe(handle, topic, IOTX_MQTT_QOS0, example_message_arrive, NULL);
    if (res < 0) {
        EXAMPLE_TRACE("subscribe failed");
        HAL_Free(topic);
        return -1;
    }

    HAL_Free(topic);
    return 0;
}

void make_file()
{
    //文件描述符
    int fd;

    //用只写方式打开文件,如果没有该文件,则创建一个文件
    fd = open("/fal/test/Data.txt", O_WRONLY | O_CREAT | O_APPEND); //和原来相比，只是把O_TRUNC参数更改为O_APPEND，即更改为打开后，如果再进行写入，将从文件的末尾位置开始写。
    // rt_kprintf("\n%f %f tmp:%s\n",Humi,Temp,String);
    //如果打开成功
    if (fd >= 0)
    {
        //写入文件
        write(fd, tmp, sizeof(tmp));

        // rt_kprintf("Write done.\n");

        //关闭文件
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
        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp, "Temp: %.1f;Humi: %.1f;Count: %d\n", Temp, Humi,++cnt);
        // rt_kprintf("\n%f %f tmp:%s\n",Humi,Temp,tmp);
        make_file();
        sprintf(tmp, "{\"params\":{\"temperature\":%.2f,\"humidity\":%.2f}}", Temp, Humi);
        return;
}
static int example_publish(void *handle)
{
    
    
    tmp_payload();
    int             res = 0;
    const char     *fmt = "/sys/%s/%s/thing/event/property/post";
    // /k1lyriw1yGj/${deviceName}/user/get
    char           *topic = NULL;
    int             topic_len = 0;
    char           *payload = tmp;
    // strcpy(payload,tmp_payload());
    // rt_kprintf("payload:%s\n",payload);
    topic_len = strlen(fmt) + strlen(DEMO_PRODUCT_KEY) + strlen(DEMO_DEVICE_NAME) + 1;
    topic = HAL_Malloc(topic_len);
    if (topic == NULL) {
        EXAMPLE_TRACE("memory not enough");
        return -1;
    }
    memset(topic, 0, topic_len);
    HAL_Snprintf(topic, topic_len, fmt, DEMO_PRODUCT_KEY, DEMO_DEVICE_NAME);

    res = IOT_MQTT_Publish_Simple(0, topic, IOTX_MQTT_QOS0, payload, strlen(payload));
    if (res < 0) {
        EXAMPLE_TRACE("publish failed, res = %d", res);
        HAL_Free(topic);
        return -1;
    }

    HAL_Free(topic);
    return 0;
}

static void example_event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    EXAMPLE_TRACE("msg->event_type : %d", msg->event_type);
}

static void mqtt_example_main(void *parameter)
{
    void                   *pclient = NULL;
    int                     res = 0;
    int                     loop_cnt = 0;
    iotx_mqtt_param_t       mqtt_params;

    HAL_GetProductKey(DEMO_PRODUCT_KEY);
    HAL_GetDeviceName(DEMO_DEVICE_NAME);
    HAL_GetDeviceSecret(DEMO_DEVICE_SECRET);

    EXAMPLE_TRACE("mqtt example");

    
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.handle_event.h_fp = example_event_handle;

    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        return ;
    }

    res = example_subscribe(pclient);
    if (res < 0) {
        IOT_MQTT_Destroy(&pclient);
        return ;
    }

    while (1) {
        if (0 == loop_cnt % 20) {
            example_publish(pclient);
        }

        IOT_MQTT_Yield(pclient, 200);

        loop_cnt += 1;
    }

    return ;
}


#define THREAD_PRIORITY 25
#define THREAD_STACK_SIZE 4096
#define THREAD_TIMESLICE 5

rt_thread_t MQTT_Thread = RT_NULL;

void MQTT_Creat_Thread(void)
{

    // 初始化设备
    Dev = aht10_init(AHT10_I2C_BUS);
    if (Dev == RT_NULL)
    {
        rt_kprintf("AHT10_init Fail");
        return;
    }

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
MSH_CMD_EXPORT(MQTT_Creat_Thread, create a MQTT_Thread);