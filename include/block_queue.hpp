#ifndef BOCK_QUEUE_HPP
#define BOCK_QUEUE_HPP

#include <pthread.h>
#include <exception>
#include <sys/time.h>
#include <stdlib.h>

#include "locker.hpp"


template <typename T>
class block_queue {
private:
    locker mutex_; // 互斥锁
    cond cond_; // 条件变量

    T* array_; // 队列数组
    int size_; // 队列大小
    int capacity_; // 队列容量
    int front_; // 队首索引
    int back_; // 指向队尾元素的下一个索引

public:
    block_queue(int capacity = 1000) : size_(0), capacity_(capacity), front_(0), back_(0) {
        if (capacity <= 0) {
            throw std::exception();
        }
        array_ = new T[capacity];
    }

    ~block_queue() {
        mutex_.lock();
        delete[] array_;
        mutex_.unlock();
    }

    void clear() {
        mutex_.lock();
        size_ = 0;
        front_ = 0;
        back_ = 0;
        // cond_.notify_all();
        mutex_.unlock();
    }

    bool full() {
        mutex_.lock();
        if (size_ >= capacity_) {
            mutex_.unlock();
            return true; // 队列已满
        }
        mutex_.unlock();
        return false; // 队列未满
    }

    bool empty() {
        mutex_.lock();
        if (size_ <= 0) {
            mutex_.unlock();
            return true; // 队列为空
        }
        mutex_.unlock();
        return false; // 队列非空
    }

    bool front(T& value) {
        mutex_.lock();
        if (size_ <= 0) {
            mutex_.unlock();
            return false; // 队列为空
        }
        value = array_[front_];
        mutex_.unlock();
        return true; // 成功获取队首元素
    }

    bool back(T& value) {
        mutex_.lock();
        if (size_ <= 0) {
            mutex_.unlock();
            return false; // 队列为空
        }
        value = array_[back_];
        mutex_.unlock();
        return true; // 成功获取队尾元素
    }


    bool push(const T& item) {
        mutex_.lock();
        while (size_ >= capacity_) {
            cond_.broadcast();
            mutex_.unlock();
            return false; // 队列已满，无法添加新元素
        }
        
        array_[back_] = item;
        back_ = (back_ + 1) % capacity_;
        ++size_;
        cond_.broadcast();
        mutex_.unlock();
        return true;
    }

    bool pop(T& item) {
        mutex_.lock();
        while (size_ <= 0) {
            cond_.broadcast();
            mutex_.unlock();
            return false; // 队列为空，无法弹出元素
        }
        item = array_[front_];
        front_ = (front_ + 1) % capacity_;
        
        --size_;
        cond_.broadcast();
        mutex_.unlock();
        return true;
    }

    // 增加超时处理
    bool pop(T& item, int timeout) {
        struct timespec t;
        struct timeval now;
        gettimeofday(&now, nullptr);
        t.tv_sec = now.tv_sec + timeout / 1000; // 秒
        t.tv_nsec = (now.tv_usec + (timeout % 1000) * 1000) * 1000; // 纳秒

        mutex_.lock();
        if (size_ <= 0) {
            if (!cond_.timewait(mutex_.get(), t)) {
                mutex_.unlock();
                return false; // 超时或错误
            }
        }

        if (size_ <= 0) {
            mutex_.unlock();
            return false; // 队列仍然为空
        }
        
        item = array_[front_];
        front_ = (front_ + 1) % capacity_;
        
        --size_;
        cond_.broadcast();
        mutex_.unlock();
        return true;
    }


    int size() {
        int current_size = 0;
        mutex_.lock();
        current_size = size_;
        mutex_.unlock();
        return current_size;
    }

    int capacity() {
        int current_capacity = 0;
        mutex_.lock();
        current_capacity = capacity_;
        mutex_.unlock();
        return current_capacity;
    }
};

#endif // BOCK_QUEUE_HPP