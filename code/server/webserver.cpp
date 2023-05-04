/*
 * @Author       : 1AoB
 * @Date         : 2023-05-04
 * @copyleft Apache 2.0
 */

#include "webserver.h"

using namespace std;

WebServer::WebServer(
    int port, int trigMode, int timeoutMS, bool OptLinger,
    int sqlPort, const char *sqlUser, const char *sqlPwd,
    const char *dbName, int connPoolNum, int threadNum,
    bool openLog, int logLevel, int logQueSize) : port_(port), openLinger_(OptLinger), timeoutMS_(timeoutMS), isClose_(false),
                                                  timer_(new HeapTimer()), threadpool_(new ThreadPool(threadNum)), epoller_(new Epoller())
{
    srcDir_ = getcwd(nullptr, 256); // 获取当前的工作路径==>相当于pwd
    assert(srcDir_);
    strncat(srcDir_, "/resources/", 16); // 拼接路径src+/resources/
    HttpConn::userCount = 0;
    HttpConn::srcDir = srcDir_;
    SqlConnPool::Instance()->Init("localhost", sqlPort, sqlUser, sqlPwd, dbName, connPoolNum);

    // 初始化事假的模式
    InitEventMode_(trigMode);

    if (!InitSocket_())
    {
        isClose_ = true; // 初始化失败,关闭服务器
    }

    /*日志相关*/
    if (openLog)
    {
        Log::Instance()->init(logLevel, "./log", ".log", logQueSize);
        if (isClose_)
        {
            LOG_ERROR("========== Server init error!==========");
        }
        else
        {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", port_, OptLinger ? "true" : "false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                     (listenEvent_ & EPOLLET ? "ET" : "LT"),
                     (connEvent_ & EPOLLET ? "ET" : "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("srcDir: %s", HttpConn::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
        }
    }
}

WebServer::~WebServer()
{
    close(listenFd_);
    isClose_ = true;
    free(srcDir_);
    SqlConnPool::Instance()->ClosePool();
}

// 设置监听的文件描述符合通信的文件描述翻符的模式
void WebServer::InitEventMode_(int trigMode)
{
    listenEvent_ = EPOLLRDHUP; // 通过套接字连接的远程主机已经关闭了连接,这个事件只能在使用 epoll 操作的 ET 模式下被触发。
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode)
    {
    case 0:
        break;
    case 1:
        connEvent_ |= EPOLLET; // 边沿触发(ET):当有数据到来,你这次没读完,下次也不再提醒你了(只有当你的数据到来时,才会再次提醒你读)
        break;
    case 2:
        listenEvent_ |= EPOLLET;
        break;
    case 3:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    default:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    }
    HttpConn::isET = (connEvent_ & EPOLLET); // 查看是否是epollet
}

// 开启服务器
void WebServer::Start()
{
    int timeMS = -1; /* epoll wait timeout == -1 无事件将阻塞 */
    if (!isClose_)   // 如果服务器没有关闭
    {
        // 打印日志
        LOG_INFO("========== Server start ==========");
    }
    while (!isClose_) // 只要服务器不关闭,就一直运行
    {
        if (timeoutMS_ > 0)
        {
            timeMS = timer_->GetNextTick();
        }

        int eventCnt = epoller_->Wait(timeMS); // 检测到有多少个事件
        for (int i = 0; i < eventCnt; i++)
        {
            /* 处理事件 */
            int fd = epoller_->GetEventFd(i);         // 获取fd
            uint32_t events = epoller_->GetEvents(i); // 获取事件
            if (fd == listenFd_)
            {
                DealListen_(); // 处理监听的文件描述符
            }
            else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                assert(users_.count(fd) > 0);
                CloseConn_(&users_[fd]); // 出现错误,关闭连接
            }
            else if (events & EPOLLIN) // 处理(往里)读的事件
            {
                assert(users_.count(fd) > 0);
                DealRead_(&users_[fd]);
            }
            else if (events & EPOLLOUT) // 处理(往外)写的事件
            {
                assert(users_.count(fd) > 0);
                DealWrite_(&users_[fd]);
            }
            else
            {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}

void WebServer::SendError_(int fd, const char *info)
{
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if (ret < 0)
    {
        LOG_WARN("send error to client[%d] error!", fd);
    }
    close(fd); // 关闭客户端
}

void WebServer::CloseConn_(HttpConn *client)
{
    assert(client);
    LOG_INFO("Client[%d] quit!", client->GetFd());
    epoller_->DelFd(client->GetFd());
    client->Close();
}

void WebServer::AddClient_(int fd, sockaddr_in addr) // 添加客户端
{
    assert(fd > 0);
    users_[fd].init(fd, addr);
    if (timeoutMS_ > 0)
    {
        timer_->add(fd, timeoutMS_, std::bind(&WebServer::CloseConn_, this, &users_[fd]));
    }
    epoller_->AddFd(fd, EPOLLIN | connEvent_);
    SetFdNonblock(fd);
    LOG_INFO("Client[%d] in!", users_[fd].GetFd());
}
// 处理监听的事件
void WebServer::DealListen_()
{
    struct sockaddr_in addr; // 保存连接的客户端的信息
    socklen_t len = sizeof(addr);
    do
    {
        int fd = accept(listenFd_, (struct sockaddr *)&addr, &len);
        if (fd <= 0) // 出错
        {
            return;
        }
        else if (HttpConn::userCount >= MAX_FD) // 连接成功,但是超出了最大的处理事件
        {
            SendError_(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        AddClient_(fd, addr); // 添加客户端
    } while (listenEvent_ & EPOLLET);
}

// 处理读事件
void WebServer::DealRead_(HttpConn *client)
{
    assert(client);
    ExtentTime_(client);

    // 线程池添加任务,让线程池里面的空闲线程去读,然后再处理=====>所以是Reactor模式
    threadpool_->AddTask(std::bind(&WebServer::OnRead_, this, client));
}

void WebServer::DealWrite_(HttpConn *client)
{
    assert(client);
    ExtentTime_(client);
    threadpool_->AddTask(std::bind(&WebServer::OnWrite_, this, client));
}

void WebServer::ExtentTime_(HttpConn *client)
{
    assert(client);
    if (timeoutMS_ > 0)
    {
        timer_->adjust(client->GetFd(), timeoutMS_);
    }
}

// 客户端只读事件
void WebServer::OnRead_(HttpConn *client) // 这方法在子线程中执行
{
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno); // 读取客户端中的数据
    if (ret <= 0 && readErrno != EAGAIN)
    {
        CloseConn_(client);
        return;
    }
    // 业务逻辑的处理
    OnProcess(client);
}
// 业务逻辑的处理
void WebServer::OnProcess(HttpConn *client)
{
    if (client->process())
    {
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT); // 监听可写
    }
    else
    {
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLIN);
    }
}

void WebServer::OnWrite_(HttpConn *client)
{
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    if (client->ToWriteBytes() == 0)
    {
        /* 传输完成 */
        if (client->IsKeepAlive())
        {
            OnProcess(client);
            return;
        }
    }
    else if (ret < 0)
    {
        if (writeErrno == EAGAIN)
        {
            /* 继续传输 */
            epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
            return;
        }
    }
    CloseConn_(client);
}

/* listenFd初始化 */
bool WebServer::InitSocket_()
{
    int ret;
    struct sockaddr_in addr;           // socket通信的地址,套接字的地址
    if (port_ > 65535 || port_ < 1024) // 如果端口号不在这个范围内
    {
        LOG_ERROR("Port:%d error!", port_); // 打印端口错误的日志
        return false;
    }
    addr.sin_family = AF_INET;                // ipv4
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // ip:主机字节序转网络字节序
    addr.sin_port = htons(port_);             // prot:主机字节序转网络字节序
    struct linger optLinger = {0};
    if (openLinger_) // Linger表示是否优雅的关闭
    {
        /* 优雅关闭: 直到所剩数据发送完毕或超时 */
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }

    listenFd_ = socket(AF_INET, SOCK_STREAM, 0); // 用于监听的文件描述符
    if (listenFd_ < 0)
    {
        LOG_ERROR("Create socket error!", port_);
        return false;
    }

    // SO_LINGER=>是否优雅的关闭
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optLinger, sizeof(optLinger));
    if (ret < 0)
    {
        close(listenFd_);
        LOG_ERROR("Init linger error!", port_);
        return false;
    }

    // setsockopt端口复用
    int optval = 1;
    /* 端口复用 */
    /* 只有最后一个套接字会正常接收数据。 */
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
    if (ret == -1)
    {
        LOG_ERROR("set socket setsockopt error !");
        close(listenFd_);
        return false;
    }

    // 绑定
    ret = bind(listenFd_, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0)
    {
        LOG_ERROR("Bind Port:%d error!", port_);
        close(listenFd_);
        return false;
    }

    // 监听有没有客户端到达
    ret = listen(listenFd_, 6);
    if (ret < 0)
    {
        LOG_ERROR("Listen port:%d error!", port_);
        close(listenFd_);
        return false;
    }
    // 监听的文件描述符listenFd_   监听的时间listenEvent_ | EPOLLIN
    ret = epoller_->AddFd(listenFd_, listenEvent_ | EPOLLIN); // 只是加到要检测的事件中,还没让epoller开始检测,因为服务器此时还没开启
    if (ret == 0)
    {
        LOG_ERROR("Add listen error!");
        close(listenFd_);
        return false;
    }
    // 设置监听文件描述符listenFd_非阻塞
    SetFdNonblock(listenFd_);
    LOG_INFO("Server port:%d", port_);
    return true;
}

// 设置文件描述符非阻塞
int WebServer::SetFdNonblock(int fd)
{
    assert(fd > 0);

    // fcntl函数:复制文件描述符,设置/获取文件的状态标志
    //  int a = fcntl(fd, F_SETFL,_):fcntl(要操作的文件描述符,F_SETFL设置文件描述符状态的标志,_)
    //  int b = fcntl(fd, F_GETFD, 0) | O_NONBLOCK):fcntl(要操作的文件描述符,F_GETFD获取文件描述符原先的标志,0) | O_NONBLOCK设置文件描述符非阻塞
    //  将b设置 给a,再返回
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
    // F_GETFL:获取文件描述符的状态标志
    // F_SETFD:提取文件描述符
}
