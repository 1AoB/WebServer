/*
 * @Author       : 1AoB
 * @Date         : 2023-05-04
 * @copyleft Apache 2.0
 */
#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h> //epoll_ctl()
#include <fcntl.h>     // fcntl()
#include <unistd.h>    // close()
#include <assert.h>    // close()
#include <vector>
#include <errno.h>

class Epoller
{
public:
    // 最大检测的事件
    explicit Epoller(int maxEvent = 1024);

    ~Epoller();
    // 增加检测的事件
    bool AddFd(int fd, uint32_t events);
    // 修改检测的事件
    bool ModFd(int fd, uint32_t events);
    // 删除检测的事件
    bool DelFd(int fd);
    // 调用内核,让内核帮我们检测
    int Wait(int timeoutMs = -1);
    // 获取事假的fd
    int GetEventFd(size_t i) const;
    // 获取事假的event
    uint32_t GetEvents(size_t i) const;

private:
    int epollFd_; // epoll_create();创建一个epoll对象,返回值就是epollFd

    std::vector<struct epoll_event> events_; // 检测到的事件的集合
};

#endif // EPOLLER_H