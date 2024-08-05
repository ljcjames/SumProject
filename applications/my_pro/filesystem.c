//记得在menuconfig中开启支持旧版本功能（Support legacy version）
#include <board.h>
#include <rtthread.h>
#include <drv_gpio.h>
#include <dfs_posix.h>//需要添加软件包进这里

//定义要写入的内容
char String[] = "Hello, RT-Thread.Welcom to RSOC!\n temp: 123, humi: 789";

//定义接受文件内容的缓冲区
char buffer[100] = {};

void FileSystem_Test(void *parameter)
{
    //文件描述符
    int fd;

    //用只写方式打开文件,如果没有该文件,则创建一个文件
    fd = open("/fal/FileTest.txt", O_WRONLY | O_CREAT);

    //如果打开成功
    if (fd >= 0)
    {
        //写入文件
        write(fd, String, sizeof(String));

        rt_kprintf("Write done.\n");

        //关闭文件
        close(fd);
    }
    else
    {
        rt_kprintf("File Open Fail.\n");
    }

    //用只读方式打开文件
    fd = open("/fal/FileTest.txt", O_RDONLY);

    if (fd>= 0)
    {
        //读取文件内容
        rt_uint32_t size = read(fd, buffer, sizeof(buffer));
    
        if (size < 0)
        {
            rt_kprintf("Read File Fail.\n");
            return ;
        }

        //输出文件内容
        rt_kprintf("Read from file test.txt : %s \n", buffer);

        //关闭文件
        close(fd);
    }
    else
    {
        rt_kprintf("File Open Fail.\n");
    }
}
//导出命令
MSH_CMD_EXPORT(FileSystem_Test, FileSystem_Test);

static void readdir_sample(void)
{
    DIR *dirp;
    struct dirent *d;

    /* 打开 / dir_test 目录 */
    dirp = opendir("/fal");
    if (dirp == RT_NULL)
    {
        rt_kprintf("open directory error!\n");
    }
    else
    {
        /* 读取目录 */
        while ((d = readdir(dirp)) != RT_NULL)
        {
            rt_kprintf("found %s\n", d->d_name);
        }

        /* 关闭目录 */
        closedir(dirp);
    }
}
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(readdir_sample, readdir sample);

/*
#define WIFI_CS GET_PIN(F, 10)
void WIFI_CS_PULL_DOWM(void)
{
    rt_pin_mode(WIFI_CS, PIN_MODE_OUTPUT);
    rt_pin_write(WIFI_CS, PIN_LOW);
}
INIT_BOARD_EXPORT(WIFI_CS GET_PIN);
*/
