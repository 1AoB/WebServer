/*
 * @Author       : 1AoB
 * @Date         : 2023-05-04
 * @copyleft Apache 2.0
 */

#include "epoller.h"

/*
epollFd_(epoll_create(512)),创建epoll对象,存储在epollFd_
events_(maxEvent),events是检测到的事件,maxEvent可以检测的事件的大小
*/
Epoller::Epoller(int maxEvent) : epollFd_(epoll_create(512)), events_(maxEvent)
{
    assert(epollFd_ >= 0 && events_.size() > 0); // 使用了 C++ 的 assert() 断言宏来检查 epollFd_ 和 events_ 变量的值是否满足一定条件
}

Epoller::~Epoller()
{
    close(epollFd_);
}

bool Epoller::AddFd(int fd, uint32_t events)
{
    if (fd < 0)
        return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev); // epoll_ctl
}

bool Epoller::ModFd(int fd, uint32_t events)
{
    if (fd < 0)
        return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &ev); // epoll_ctl
}

bool Epoller::DelFd(int fd)
{
    if (fd < 0)
        return false;
    epoll_event ev = {0};
    return 0 == epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &ev); // epoll_ctl
}

int Epoller::Wait(int timeoutMs)
{
    // epoll_wait
    return epoll_wait(epollFd_, &events_[0], static_cast<int>(events_.size()), timeoutMs);
}

int Epoller::GetEventFd(size_t i) const
{
    assert(i < events_.size() && i >= 0);
    return events_[i].data.fd; // 根据索引i,获取fd
}

uint32_t Epoller::GetEvents(size_t i) const
{
    assert(i < events_.size() && i >= 0);
    return events_[i].events; //  根据索引i,获取event
}