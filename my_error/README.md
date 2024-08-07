![语法错误图](syntax_env1.5.png)


换了env2就可以？

![alt text](image-1.png)
移出来+头文件可以编译
但
![alt text](image.png)
对照示例0.1.1版什么都不用做，也不会报错什么没定义
对比.config/rtconfig.h
但发了红外线也没反应……
再加回接收、发送
又是没定义
对比.config/rtconfig.h
调整好pin，timer，#define ***timer
总算可以了
调整好针脚等可以运行了，但一闪一闪的，估计是线程切换的原因？
注释掉红外线线程，也没有用，改了优先级终于可以了
**破案了！lcd、红外线用的同一个timer14，[哭笑不得]😂**
换了个timer11就不闪了！！！
``` c
Snake_Thread = rt_thread_create("Snake_Thread", snake_entry, RT_NULL, THREAD_STACK_SIZE, 10, THREAD_TIMESLICE);
```
不注释又闪了，
加到myproject，没唤醒时没事，唤醒后闪，重启也闪
![alt text](image-2.png)
### GET_PIN(F, 11) 的头文件

``` c
#include <drv_gpio.h>
/* 配置 LED 灯引脚 */
#define PIN_LED_B GET_PIN(F, 11) // PF11 :  LED_B        --> LED
#define PIN_LED_R GET_PIN(F, 12) // PF12 :  LED_R        --> LED
```

### 红外线遥控器按键值
原来要按两次key才变，搞得还以为昨天找到的都是乱输出的

| 编号（key）| 功能 |
| :----: | :----: |
| 0x30| 上 |
| 0xE8 | 左 |
| 0xB0 | 下 |
| 0x68 | 右 |
| 0xFF | OK |
| 0x38 | 电源键 |
| 0xA8 | 静音 |
| 0x88 | 菜单 |
| 0x28 | 退出 |

### 等待消息
写成这样，导致一直等待，所以一直没反应
``` c
if(page_chosen == 1)
{
    rt_mdelay(1000);
}
```

### 附：ringbuffer（环形缓冲区）
注意：RT-Thread 的 ringbuffer 组件并未提供线程阻塞的功能，因此 ringbuffer 本质上是一个全局共享的对象，多线程使用时注意使用互斥锁保护。
![alt text](image-3.png)
``` c
#include <rtthread.h>
#include <ipc/ringbuffer.h>

typedef struct rb_example {
    int a;
    int b;
} rb_example_t;

int ringbuffer_example(void)
{
    rb_example_t data = {
        .a = 1,
        .b = 2,
    };

    struct rt_ringbuffer * rb = rt_ringbuffer_create(sizeof(rb_example_t) * 2);
    RT_ASSERT(rb != RT_NULL);

    rt_kprintf("Put data to   ringbuffer, a: %d b: %d size: %d\n", data.a, data.b, sizeof(data));
    rt_ringbuffer_put(rb, (rt_uint8_t *)&data, sizeof(data));


    rb_example_t recv_data;
    rt_size_t recv = rt_ringbuffer_get(rb, (rt_uint8_t *)&recv_data, sizeof(recv_data));
    RT_ASSERT(recv == sizeof(recv_data));
    rt_kprintf("Get data from ringbuffer, a: %d b: %d size: %d\n", recv_data.a, recv_data.b, sizeof(recv_data));

    return 0;
}

MSH_CMD_EXPORT(ringbuffer_example, ringbuffer example);

```

### union（联合体）
约等于结构体
1. 可以通过v.u来操作一个uint32类型的对象，当需要将uint32变量的低端字节看做一个字符的时候，只需要访问v.c就可以了
2. 共享同一块大小的内存