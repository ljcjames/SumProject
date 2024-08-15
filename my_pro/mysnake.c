// #include <stdio.h>
#include <rtthread.h>
#include <stdbool.h>
#include "my_func.h"
#include "mysnake.h"
#include <time.h>
#include <rtatomic.h>

#define LCD_MAX 240 //屏幕大小(长宽像素)
#define SNAKE_SIZE 40 //蛇身大小(格子大小)
#define SNAKE_MAX (LCD_MAX / SNAKE_SIZE) //每行格子数（最大蛇长+1）
rt_atomic_t now_direction = 3;
rt_atomic_t snake_pressed = 0;
extern rt_atomic_t page_chosen;
extern rt_atomic_t page_first;
extern rt_atomic_t page_stop;
int snake_max = SNAKE_MAX * 3;
int snake_len = 3;

// bool snake_table[SNAKE_MAX][SNAKE_MAX] = {0};
// struct My_snake
// {
//     int x;
//     int y;
// }snake_list[SNAKE_MAX*SNAKE_MAX] = {0};
// /* rt_event_t 是指向事件结构体的指针类型  */
// typedef struct My_snake* my_snake_t;

void snake_entry(void *parameter)
{
    system("irq");
    time_t t;
    /* 初始化随机数发生器 */
    srand((unsigned)time(&t));
    int snake_list[SNAKE_MAX + 1][2] = {0};//储存蛇头、蛇尾、蛇身坐标
    int snake_direction[4][2] = {{0, -1}, {-1, 0}, {0, 1}, {1, 0}}; // 上，左，下，右
    int snake_food[2];//果子位置
    char snake_dirshow[4][7] = {"upup", "left", "down", "rigt"};//显示操作用词
    char tmp[10];//用于字符串转换
    int snake_head = 2, snake_tail = 0; // 蛇头，蛇尾
    //设置初始位置
    snake_list[1][0] = SNAKE_MAX / 2;
    snake_list[1][1] = SNAKE_MAX / 2;
    snake_list[0][0] = snake_list[1][0] - 1;
    snake_list[0][1] = snake_list[1][1];
    snake_list[2][0] = snake_list[1][0] + 1;
    snake_list[2][1] = snake_list[1][1];
    // snake_table[snake_list[0][0]][snake_list[0][1]] = 1;
    // snake_table[snake_list[1][0]][snake_list[1][1]] = 1;
    // snake_table[snake_list[2][0]][snake_list[2][1]] = 1;
    // snake_address(snake_list[0][0], snake_list[0][1], SNAKE_SIZE, BLACK);
    // snake_address(snake_list[1][0], snake_list[1][1], SNAKE_SIZE, BLACK);
    // snake_address(snake_list[2][0], snake_list[2][1], SNAKE_SIZE, BLACK);
    snake_food[0] = rand() % SNAKE_MAX;
    snake_food[1] = rand() % SNAKE_MAX;
    // snake_address(snake_food[0], snake_food[1], SNAKE_SIZE, GREEN);
    int new_head_x = 0, new_head_y = 0;//新的蛇头位置
    int new_direction = 0,snake_now=0;//随机生成的新方向，重新显示时当前位置的队列编号
    while (1)
    {
        //当页面符合且不处于暂停状态才运行
        if (page_chosen == 1&&!page_stop)
        {
            // 刚切换页面过来，重新显示
            if (page_first == 1)
            {
                rt_kprintf("page:snake\n");
                page_first = 0;
                lcd_fill(0, 0, 240, 240, WHITE);
                snake_address(snake_food[0], snake_food[1], SNAKE_SIZE, GREEN);
                snake_now = snake_tail-1;
                // for (int i = 0; i = snake_len && i < SNAKE_MAX - 1; i++)
                do{
                    snake_now=(snake_now+1)%SNAKE_MAX;
                    snake_address(snake_list[snake_now][0], snake_list[snake_now][1], SNAKE_SIZE, BLACK);
                }while(snake_now!=snake_head);
            }
            //如果长期（约3*SNAKE_MAX）没有按键，则随机改变方向
            if (!snake_pressed)
            {
                // 50%的概率保持当前方向
                if (rand() % 100 < 50)
                {
                    new_direction = rand() % 3;
                    now_direction = (now_direction + 3 + new_direction) % 4; // 防止反向、走回头路
                }
            }
            else
            {
                rt_atomic_add(&snake_pressed, -1);
            }

            // 计算新蛇头位置、并显示坐标
            new_head_x = (snake_list[snake_head][0] + snake_direction[now_direction][0] + SNAKE_MAX) % (SNAKE_MAX);
            new_head_y = (snake_list[snake_head][1] + snake_direction[now_direction][1] + SNAKE_MAX) % (SNAKE_MAX);
            rt_sprintf(tmp, "(%d,%d)", new_head_x, new_head_y);
            // rt_kprintf("head:%d,%d\n", snake_list[snake_head][0], snake_list[snake_head][1]);
            lcd_show_string(20, 20, 16, snake_dirshow[now_direction]);
            lcd_show_string(20 + 16 * 4, 20, 16, tmp);
            snake_address(new_head_x, new_head_y, SNAKE_SIZE, BLACK);

            // 蛇头碰到果子
            if (new_head_x == snake_food[0] && new_head_y == snake_food[1])
            {
                //新生成、显示果子
                snake_food[0] = rand() % SNAKE_MAX;
                snake_food[1] = rand() % SNAKE_MAX;
                snake_address(snake_food[0], snake_food[1], SNAKE_SIZE, GREEN);
                // 长度增加、打印
                snake_len++;
                rt_sprintf(tmp, "%d", snake_len);
                lcd_show_string(100, 105, 32, tmp);
                // 防止蛇咬尾出现bug，限制最长为SNAKE_MAX-1，将蛇尾涂白
                if (snake_len >= SNAKE_MAX)
                {
                    snake_address(snake_list[snake_tail][0], snake_list[snake_tail][1], SNAKE_SIZE, WHITE);
                    snake_tail = (snake_tail + 1) % (SNAKE_MAX);
                }
            }
            else
            {
                //若果子在蛇尾则不涂白
                if (snake_list[snake_tail][0] == snake_food[0] && snake_list[snake_tail][1] == snake_food[1])
                {
                }
                else
                {
                    snake_address(snake_list[snake_tail][0], snake_list[snake_tail][1], SNAKE_SIZE, WHITE);
                }
                snake_tail = (snake_tail + 1) % (SNAKE_MAX);
            }
            //记录新的蛇头
            snake_head = (snake_head + 1) % (SNAKE_MAX);
            snake_list[snake_head][0] = new_head_x;
            snake_list[snake_head][1] = new_head_y;
        }
        rt_thread_mdelay(900);
    }
}
