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