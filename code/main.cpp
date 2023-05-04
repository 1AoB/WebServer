/*
 * @Author       : 1AoB
 * @Date         : 2023-05-04
 * @copyleft Apache 2.0
 */
#include <unistd.h>
#include "server/webserver.h"

int main()
{
    /* 守护进程 后台运行 */
    // daemon(1, 0);

    // http://192.168.8.128:8989
    WebServer server(
        8989 /*1316*/, 3, 60000, false,    /* 端口 ET模式 timeoutMs 优雅退出  */
        3306, "root", "root", "webserver", /* Mysql配置 */
        12, 6, true, 1, 1024);             /* 连接池数量 线程池数量 日志开关 日志等级 日志异步队列容量 */
    server.Start();                        // 开启服务端
}
