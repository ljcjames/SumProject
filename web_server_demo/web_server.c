// #include <rtthread.h>
#include <httpd.h>
#ifdef LWIP_HTTPD_SSI 
extern void httpd_ssi_init(void);
#endif

#ifdef LWIP_HTTPD_CGI 
extern void httpd_cgi_init(void);
#endif
void webserver_start(void)
{
    rt_kprintf("\n\n\tNow, Initializing The WEB File System...\n");
     /* Httpd Init */
     httpd_init();
     /* 配置 SSI 处理程序 */
     httpd_ssi_init();
     /* 配置 CGI 处理器 */
     httpd_cgi_init();
    rt_kprintf("\tNow, Starting The WEB Server Thread...\n");
}
MSH_CMD_EXPORT(webserver_start, start web server);
