#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <stdlib.h>
#include <drv_lcd.h>
#include <ntp.h>

#define LCD_MAX 240
void lcd_black(int x, int y)
{
    lcd_address_set(x, y, x, y);
    lcd_write_half_word(BLACK);
}

void lcd_white(int x, int y)
{
    lcd_address_set(x, y, x, y);
    lcd_write_half_word(WHITE);
}
int roundxy[4][2] = {
    {0, 0},
    {0, LCD_MAX},
    {LCD_MAX, 0},
    {LCD_MAX, LCD_MAX},
};
int xymove[4][2] = {
    {1, 1},
    {1, -1},
    {-1, 1},
    {-1, -1},
};
void mytime()
{
    rt_thread_mdelay(10000);
    time_t cur_time;

    cur_time = ntp_get_time(RT_NULL);
    if (cur_time)
    {
        rt_kprintf("NTP Server Time: %s", ctime((const time_t *)&cur_time));
    }
    lcd_show_string(20, 2, 16, ctime((const time_t *)&cur_time));
}

void xy_round(int x, int y, int x2, int y2, int r, int ii)
{
    // rt_kprintf("x:%d,y:%d,x2:%d,y2:%d,r:%d\n", x, y, x2, y2, r);
    for (int i = x; i != x2; i += xymove[ii][0])
    {
        for (int j = y; j != y2; j += xymove[ii][1])
        {
            int newi = x2 - i;
            int newj = y2 - j;
            // rt_kprintf("(%d,%d,%d)",(newi * newi + newj * newj), newi, newj);
            if ((newi * newi + newj * newj) > (r * r))
            {
                // rt_kprintf("x:%d,y:%d\n", i, j);
                lcd_black(i, j);
            }
        }
    }
}
void my_round(int r)
{
    // 这个范围涂黑
    lcd_fill(0, 0, roundxy[2][0], roundxy[2][1], BLACK);
    lcd_write_half_word(BLACK);

    for (int i = 0; i < 4; i++)
    {
        xy_round(roundxy[i][0], roundxy[i][1], roundxy[i][0] + r * xymove[i][0], roundxy[i][1] + r * xymove[i][1], r, i);
    }
}
void xy_sink()
{
    for (int i = 0; i < 240; i++)
    {
        for (int j = 0; j <= 240; j++)
        {
            lcd_black(j, 240 - i);
            rt_thread_mdelay(1);
        }
        // rt_kprintf("(%d,...) Blacked\n", i);
    }
}
void snake_address(int x, int y, int r, const rt_uint16_t da)
{
    int f = 0; // 使蛇身成节
    if (r > 5)
    {
        f = 1;
    }
    for (int i = x * r + f; i < x * r + r - f; i++)
    {
        for (int j = y * r + f; j < y * r + r - f; j++)
        {
            lcd_address_set(i, j, i, j);
            lcd_write_half_word(da);
        }
    }
}
// bt 命令行
int color_cmd(int argc, char **argv)
{
    snake_address(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), BLACK);
    return 1;
}
MSH_CMD_EXPORT_ALIAS(color_cmd, snk, bt 命令行);