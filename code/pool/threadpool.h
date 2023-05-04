/*
 * 线程池
 * @Date         : 2023-05-04
 * @copyleft Apache 2.0
 */

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>
class ThreadPool // 线程池类
{
public:
    // explicit防止构造函数隐式转换
    //  构造函数,创建8个线程
    explicit ThreadPool(size_t threadCount = 8) : pool_(std::make_shared<Pool>())
    {
        assert(threadCount > 0); // 断言,测试用的
        for (size_t i = 0; i < threadCount; i++)
        {
            std::thread([pool = pool_] // 创建std::thread
                        {
                    std::unique_lock<std::mutex> locker(pool->mtx);//锁
                    while(true) {//while死循环
                        if(!pool->tasks.empty()) {//任务对列不为空
                            auto task = std::move(pool->tasks.front());//从任务对列取第一个任务
                            pool->tasks.pop();//任务出列
                            
                            locker.unlock();//解锁
                            task();//执行的任务函数
                            locker.lock();//加锁
                        } 
                        else if(pool->isClosed) break;
                        else pool->cond.wait(locker);//没有任务会阻塞在这
                    } })
                .detach(); // 设置线程分离
        }
    }

    ThreadPool() = default;

    ThreadPool(ThreadPool &&) = default;

    ~ThreadPool()
    {
        if (static_cast<bool>(pool_))
        {
            {
                std::lock_guard<std::mutex> locker(pool_->mtx);
                pool_->isClosed = true; // 表示要关闭所有线程
            }
            pool_->cond.notify_all(); // 唤醒所有线程
        }
    }

    template <class F>     // F是任务的类型
    void AddTask(F &&task) // 添加一个任务
    {
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->tasks.emplace(std::forward<F>(task)); // 把任务添加到任务对列
        }
        pool_->cond.notify_one(); // 唤醒一个线程去消费
    }

private:
    // 结构体,池子
    struct Pool
    {
        std::mutex mtx;                          // 互斥锁
        std::condition_variable cond;            // 条件变量
        bool isClosed;                           // 是否关闭
        std::queue<std::function<void()>> tasks; // 对列,保存的是任务
    };
    std::shared_ptr<Pool> pool_; // 池子
};

#endif // THREADPOOL_H