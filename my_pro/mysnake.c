#include <stdio.h>
#include <rtthread.h>
#include <stdbool.h>
#include "my_func.h"
#include "mysnake.h"
#include <time.h>
#include <rtatomic.h>

#define LCD_MAX 240
#define SNAKE_SIZE 20
#define SNAKE_MAX LCD_MAX / SNAKE_SIZE
rt_atomic_t now_direction = 3;
rt_atomic_t snake_pressed = 0;
extern rt_atomic_t page_chosen;
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
    int snake_list[SNAKE_MAX + 1][2] = {0};
    int snake_direction[4][2] = {{0, -1}, {-1, 0}, {0, 1}, {1, 0}}; // 上，左，下，右
    int snake_food[2];
    bool food_flag = false;
    char snake_dirshow[4][7] = {"upup", "left", "down", "rigt"};
    char tmp[10];
    int snake_head = 2, snake_tail = 0; // 蛇头，蛇尾
    snake_list[1][0] = SNAKE_MAX / 2;
    snake_list[1][1] = SNAKE_MAX / 2;
    snake_list[0][0] = snake_list[1][0] - 1;
    snake_list[0][1] = snake_list[1][1];
    snake_list[2][0] = snake_list[1][0] + 1;
    snake_list[2][1] = snake_list[1][1];
    // snake_table[snake_list[0][0]][snake_list[0][1]] = 1;
    // snake_table[snake_list[1][0]][snake_list[1][1]] = 1;
    // snake_table[snake_list[2][0]][snake_list[2][1]] = 1;
    snake_address(snake_list[0][0], snake_list[0][1], SNAKE_SIZE, BLACK);
    snake_address(snake_list[1][0], snake_list[1][1], SNAKE_SIZE, BLACK);
    snake_address(snake_list[2][0], snake_list[2][1], SNAKE_SIZE, BLACK);
    snake_food[0] = rand() % SNAKE_MAX;
    snake_food[1] = rand() % SNAKE_MAX;
    snake_address(snake_food[0], snake_food[1], SNAKE_SIZE, GREEN);
    int new_head_x = 0, new_head_y = 0;
    int new_direction = 0;
    while (1)
    {
        if (page_chosen == 1)
        {
            if (!snake_pressed)
            {
                // 50%的概率保持当前方向，20%的概率随机改变方向
                if (rand() % 100 < 50)
                {
                    new_direction = rand() % 3;
                    now_direction = (now_direction + 3 + new_direction) % 4; // 防止反向,走回头路
                }
            }
            else
            {
                rt_atomic_add(&snake_pressed, -1);
            }

            new_head_x = (snake_list[snake_head][0] + snake_direction[now_direction][0] + SNAKE_MAX) % (SNAKE_MAX);
            new_head_y = (snake_list[snake_head][1] + snake_direction[now_direction][1] + SNAKE_MAX) % (SNAKE_MAX);

            sprintf(tmp, "(%d,%d)", new_head_x, new_head_y);
            // rt_kprintf("head:%d,%d\n", snake_list[snake_head][0], snake_list[snake_head][1]);
            lcd_show_string(20, 20, 16, snake_dirshow[now_direction]);
            lcd_show_string(20 + 16 * 4, 20, 16, tmp);

            snake_address(new_head_x, new_head_y, SNAKE_SIZE, BLACK);
            if (new_head_x == snake_food[0] && new_head_y == snake_food[1])
            {
                snake_food[0] = rand() % SNAKE_MAX;
                snake_food[1] = rand() % SNAKE_MAX;
                snake_address(snake_food[0], snake_food[1], SNAKE_SIZE, GREEN);
                snake_len++;
                sprintf(tmp, "%d", snake_len);
                lcd_show_string(100, 105, 32, tmp);
                // 防止蛇咬尾出现bug
                if (snake_len >= SNAKE_MAX)
                {
                    snake_address(snake_list[snake_tail][0], snake_list[snake_tail][1], SNAKE_SIZE, WHITE);
                    snake_tail = (snake_tail + 1) % (SNAKE_MAX);
                }
            }
            else
            {
                if (snake_list[snake_tail][0] == snake_food[0] && snake_list[snake_tail][1] == snake_food[1])
                {
                }
                else
                {
                    snake_address(snake_list[snake_tail][0], snake_list[snake_tail][1], SNAKE_SIZE, WHITE);
                }
                snake_tail = (snake_tail + 1) % (SNAKE_MAX);
            }
            snake_head = (snake_head + 1) % (SNAKE_MAX);
            snake_list[snake_head][0] = new_head_x;
            snake_list[snake_head][1] = new_head_y;
            rt_thread_mdelay(900);
        }
    }
}
