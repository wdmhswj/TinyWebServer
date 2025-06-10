#ifndef THEAD_POOL_HPP
#define THEAD_POOL_HPP

#include <pthread.h>
#include <exception>
#include <list>
#include "locker.hpp"
#include "sql_connection_pool.hpp"

template <typename T>
class threadpool {
public:
    threadpool(int actor_model, connection_pool* connPool, int thread_num=8, int max_request=10000);
    ~threadpool();
    bool append(T* request, int state); // 添加任务到请求队列, state用于模型切换
    bool append_p(T* request);
private:
    static void* worker(void* arg); // 线程工作函数，处理请求
    void run(); // 线程池运行函数

private:
    // 线程池中的线程数量
    int thread_num_;
    // 请求队列中允许的最大请求数
    int max_requests_;
    // 线程池数组
    pthread_t* threads_;
    // 请求队列
    std::list<T*> work_queue_;
    // 保护请求队列的互斥锁
    locker queue_locker_;
    // 信号量，是否有任务需要处理
    sem queue_sem_;
    // 数据库连接池
    connection_pool* conn_pool_;
    // 模型切换
    int actor_model_;

};

#endif // THEAD_POOL_HPP