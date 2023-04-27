#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>

int main() {

    // 创建socket
    int lfd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in saddr;
    saddr.sin_port = htons(9999);
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = INADDR_ANY;
    //其值为 0x 00000000（或者使用宏定义 htonl(INADDR_ANY)）。当程序将 socket 绑定到 INADDR_ANY 上时，该 socket 可以接受任何连接请求，无论是来自本地主机还是来自网络上的任何其他主机。

    // 绑定
    bind(lfd, (struct sockaddr *)&saddr, sizeof(saddr));

    // 监听
    listen(lfd, 8);

    /***epoll代码***/
    // 调用epoll_create()创建一个epoll实例
    int epfd = epoll_create(100);//给一个大于0的值即可 

    /***epev是一个临时变量,它是epoll_ctl的最后一个参数,这里我们表示检测可读事件***/
    // 将监听的文件描述符相关的检测信息添加到epoll实例中
    struct epoll_event epev;//struct epoll_event 是一个结构体类型，用于表示一个事件
    epev.events = EPOLLIN;//events 成员表示需要监听的事件类型,EPOLLIN 表示对应的文件描述符可以进行读取操作
    epev.data.fd = lfd;//data.fd 成员表示需要监听的文件描述符,int fd：一个整数，可以用于存储文件描述符。
    /*以上两行代码的结果:当 lfd 上有可读事件时，epoll 会检测到并通知程序进行相应的处理。*/
    /**********************************************************/

    //将一个监听文件描述符相关的检测信息添加到 epoll 实例中
    /*epfd：表示 epoll 实例的文件描述符；
    EPOLL_CTL_ADD：表示需要对 epoll 实例中的文件描述符进行添加操作；
    lfd：表示需要添加到 epoll 实例中的文件描述符(epoll_ctl函数要操作的文件描述符)；
    &epev：表示需要添加的文件描述符相关的检测信息。*/
    epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &epev);//EPOLL_CTL_ADD: 添加   它的值为1

    struct epoll_event epevs[1024];//接收检测后的数据

    while(1) {
        //epoll_wait用于等待多个文件描述符上的事件发生,它会返回有多少个文件描述符发生改变,返回将改变的文件描述符从0~ret-1放入到epevs中
        //struct epoll_event epevs是一个结构体类型，用于表示一个事件。
        //在 Linux 系统中，它通常被用作 epoll_wait() 函数的第二个参数，以便在等待期间返回发生的事件。
        int ret = epoll_wait(epfd, epevs, 1024, -1);//1024是epevs数组大小
        if(ret == -1) {
            perror("epoll_wait");
            exit(-1);
        }

        printf("ret = %d\n", ret);//有几个发生了改变

        for(int i = 0; i < ret; i++) {

            //当前遍历到的fd
            int curfd = epevs[i].data.fd;

            if(curfd == lfd) {//当前遍历到的描述符是否是监听的描述符
                // 监听的文件描述符有数据达到，有客户端连接
                struct sockaddr_in cliaddr;
                int len = sizeof(cliaddr);
                int cfd = accept(lfd, (struct sockaddr *)&cliaddr, &len);//accept接收->得到客户端与服务端用于通信的描述符


                epev.events = EPOLLIN ;//EPOLLIN 表示对应的文件描述符可以进行读取操作
                epev.data.fd = cfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &epev);//将用于通信的描述符添加(EPOLL_CTL_ADD)到epoll实例epfd中
            } else {
                if(epevs[i].events & EPOLLOUT) {//如果有写事件到达.就continue,因为我们没有监听可写事件
                    continue;
                }   
                // 有数据到达，需要通信
                char buf[1024] = {0};
                int len = read(curfd, buf, sizeof(buf));
                if(len == -1) {//有错误
                    perror("read");
                    exit(-1);
                } else if(len == 0) {//对方已关闭连接
                    printf("client closed...\n");
                    epoll_ctl(epfd, EPOLL_CTL_DEL, curfd, NULL);//从红黑树中 将当前遍历到的描述符curfd 删除(EPOLL_CTL_DEL)
                    close(curfd);
                } else if(len > 0) {//读到当前的数据
                    printf("read buf = %s\n", buf);
                    write(curfd, buf, strlen(buf) + 1);//读出当前遍历到的描述符curfd 读到的数据
                }

            }

        }
    }

    close(lfd);//关闭监听的描述符
    close(epfd);//关闭epoll的描述符
    return 0;
}
