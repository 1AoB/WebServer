/*
 * @Author       : 1AoB
 * @Date         : 2023-05-04
 * @copyleft Apache 2.0
 */

#ifndef SQLCONNRAII_H
#define SQLCONNRAII_H
#include "sqlconnpool.h"

/* 资源在对象构造初始化
资源在对象析构时释放*/
class SqlConnRAII
{
public:
    // 传递二级指针是因为要修改指针的指向,所以要传入指针的地址,自然就是二级指针了
    SqlConnRAII(MYSQL **sql, SqlConnPool *connpool)
    {
        assert(connpool);
        *sql = connpool->GetConn();
        sql_ = *sql;
        connpool_ = connpool;
    }

    ~SqlConnRAII()
    {
        if (sql_)
        {
            // 释放就是重新放到池子里面
            connpool_->FreeConn(sql_);
        }
    }

private:
    MYSQL *sql_;
    SqlConnPool *connpool_;
};

#endif // SQLCONNRAII_H