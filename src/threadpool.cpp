#include "threadpool.hpp"

template <typename T>
threadpool<T>::threadpool(int actor_model, connection_pool* connPool, int thread_num, int max_request)
    : thread_num_(thread_num), max_request_(max_request), threads_(thread_num), conn_pool_(connPool), actor_model_(actor_model)
{
    if (thread_num <= 0 || max_request <= 0) {
        throw std::exception();
    }

    // 创建线程池
    for (int i=0; i<thread_num; ++i) {
        if (pthread_create(&threads_[i], nullptr, worker, this) != 0) {
            throw std::exception();            
        }
        if (pthread_detach(threads_[i]) != 0) {
            throw std::exception();
        }
    }
}


template <typename T>
threadpool<T>::~threadpool() {
}

template <typename T>
bool threadpool<T>::append(T* request, int state) {
    queue_locker_.lock();
    if (work_queue_.size() >= max_requests_) {
        queue_locker_.unlock();
        return false; // 请求队列已满
    }
    request->state_ = state; // 设置请求状态
    work_queue_.push_back(request);
    queue_locker_.unlock();
    queue_sem_.post(); // 通知有新任务到来
    return true; // 成功添加请求
}

template <typename T>
bool threadpool<T>::append_p(T* request) {
    queue_locker_.lock();
    if (work_queue_.size() >= max_requests_) {
        queue_locker_.unlock();
        return false; // 请求队列已满
    }
    work_queue_.push_back(request);
    queue_locker_.unlock();
    queue_sem_.post(); // 通知有新任务到来
    return true; // 成功添加请求
}

template <typename T>
void* threadpool<T>::worker(void* arg) {
    threadpool* pool = static_cast<threadpool*>(arg);
    pool->run(); // 调用线程池的运行函数
    return static_cast<void*>(pool);
}

template <typename T>
void threadpool<T>::run() {
    while (true) {
        queue_sem_.wait(); // 等待有任务到来
        queue_locker_.lock(); // 锁定请求队列
        if (work_queue_.empty()) {
            queue_locker_.unlock();
            continue; // 如果队列为空，继续等待
        }
        T* request = work_queue_.front(); // 获取队列中的第一个请求
        work_queue_.pop_front(); // 从队列中移除该请求
        queue_locker_.unlock(); // 解锁请求队列

        if (!request) {
            continue; // 如果请求为空，跳过处理
        }

        if (actor_model_ == 1) { // 主从模型：表示
            if (request->state_ == 0) {
                if (request->read_once()) {
                    request->improv = 1; // 设置improv标志，表示读操作已完成
                    connectionRAII mysqlcon(&request->mysql, conn_pool_); // RAII管理数据库连接
                    request->process(); // 处理请求
                } else {
                    request->improv = 1;
                    request->timer_flag = 1; // 设置定时器标志，表示读操作失败
                }
            } else { // 写操作
                if (request->write()) {
                    request->improv = 1;
                } else {
                    request->improv = 1;
                    request->timer_flag = 1; // 设置定时器标志，表示写操作失败
                }
            }
        } else { // 其他模型（如线程池模型）
            connectionRAII mysqlcon(&request->mysql, conn_pool_); // RAII管理数据库连接
            request->process(); // 直接处理请求
        }
    }
}