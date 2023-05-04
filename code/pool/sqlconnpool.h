/*
 * @Author       : 1AoB
 * @Date         : 2023-05-04
 * @copyleft Apache 2.0
 */
#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include "../log/log.h"

class SqlConnPool
{
public:
    static SqlConnPool *Instance();

    MYSQL *GetConn();           // 获取一个连接
    void FreeConn(MYSQL *conn); // 释放连接,重新放到池子里面
    int GetFreeConnCount();     // 获取空闲的数量

    // 主机名 端口
    // 用户名 密码
    // 数据库名 数据库大小
    void Init(const char *host, int port,
              const char *user, const char *pwd,
              const char *dbName, int connSize);
    void ClosePool();

private:
    SqlConnPool();
    ~SqlConnPool();

    int MAX_CONN_;  // 最大的连接数
    int useCount_;  // 当前的用户数
    int freeCount_; // 空闲的用户数

    std::queue<MYSQL *> connQue_; // 队列(MYSQL*)
    std::mutex mtx_;              // 互斥锁
    sem_t semId_;                 // 信号量
};

#endif // SQLCONNPOOL_H