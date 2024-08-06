![语法错误图](syntax_env1.5.png)


换了env2就可以？

![alt text](image-1.png)
移出来+头文件可以编译
但
![alt text](image.png)
对照示例0.1.1版什么都不用做，也不会报错什么没定义
.config/rtconfig.h
### GET_PIN(F, 11) 的头文件

``` c
#include <drv_gpio.h>
/* 配置 LED 灯引脚 */
#define PIN_LED_B GET_PIN(F, 11) // PF11 :  LED_B        --> LED
#define PIN_LED_R GET_PIN(F, 12) // PF12 :  LED_R        --> LED
```