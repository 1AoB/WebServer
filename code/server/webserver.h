/*
 * @Author       : 1AoB
 * @Date         : 2023-05-04
 * @copyleft Apache 2.0
 */
#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unordered_map>
#include <fcntl.h>  // fcntl()
#include <unistd.h> // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "epoller.h"
#include "../log/log.h"
#include "../timer/heaptimer.h"
#include "../pool/sqlconnpool.h"
#include "../pool/threadpool.h"
#include "../pool/sqlconnRAII.h"
#include "../http/httpconn.h"

class WebServer
{
public:
    WebServer(
        int port, int trigMode, int timeoutMS, bool OptLinger,
        int sqlPort, const char *sqlUser, const char *sqlPwd, // 数据库的端口,数据库用户名,数据库密码
        const char *dbName, int connPoolNum, int threadNum,   // 数据库名称,数据库连接池的数量,线程池线程的数量
        bool openLog, int logLevel, int logQueSize);          // 打开日志,日志级别,日志异步队列的大小

    ~WebServer();
    void Start();

private:
    bool InitSocket_();
    void InitEventMode_(int trigMode);
    void AddClient_(int fd, sockaddr_in addr);

    void DealListen_();
    void DealWrite_(HttpConn *client);
    void DealRead_(HttpConn *client);

    void SendError_(int fd, const char *info);
    void ExtentTime_(HttpConn *client);
    void CloseConn_(HttpConn *client);

    void OnRead_(HttpConn *client);
    void OnWrite_(HttpConn *client);
    void OnProcess(HttpConn *client);

    static const int MAX_FD = 65536; // 最大的文件描述符

    static int SetFdNonblock(int fd); // 设置文件描述符非阻塞

    int port_;        // 端口
    bool openLinger_; // 会否打开优雅关闭
    int timeoutMS_;   /* 超时时间：毫秒MS */
    bool isClose_;    // 是否关闭
    int listenFd_;    // 监听的文件描述符
    char *srcDir_;    // 资源目录

    uint32_t listenEvent_; // 监听的文件描述符的事件
    uint32_t connEvent_;   // 连接到的文件描述符的事件

    std::unique_ptr<HeapTimer> timer_;        // 定时器
    std::unique_ptr<ThreadPool> threadpool_;  // 线程池
    std::unique_ptr<Epoller> epoller_;        // epoll对象
    std::unordered_map<int, HttpConn> users_; // key:fd , value:保存的是客户端连接的信息
};

#endif // WEBSERVER_H