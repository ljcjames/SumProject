#include <drv_lcd.h>
#include <rtthread.h>
#include <stdio.h>
#include <malloc.h>
#define PAGE_MAX 4
#define LCD_MAX 240
#define EVENT_MQTT_ENABLE (1<<3)
// struct rt_event my_event;


void mytime();
void greattime();
void xy_round(int x, int y, int x2, int y2, int r, int ii);
void my_round(int r);
void xy_sink();
void lcd_black(int x, int y);
void lcd_white(int x, int y);
void snake_address(int x, int y, int r, const rt_uint16_t da);


// typedef int QDataType;

// typedef struct QListNode
// {
//     QDataType data;
//     struct QListNode* next;

// } QueueNode;
