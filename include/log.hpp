#ifndef LOG_H
#define LOG_H

#include <cstdio>
#include <string>
#include <iostream>
#include <stdarg.h>
#include <pthread.h>
#include <memory>
#include <vector>
#include "block_queue.hpp"

class Log {


private:
    std::string dir_name_; // 路径名
    std::string log_name_; // 日志文件名
    int split_lines_; // 每个日志文件的最大行数
    int log_buf_size_; // 日志缓冲区大小
    long long count_; // 日志行数计数
    int today_; // 因为按天分类，记录今天的日期
    // std::shared_ptr<FILE> fp_; // 打开log的文件指针
    FILE* fp_; // 打开log的文件指针
    std::vector<char> buf_; // 日志缓冲区
    std::shared_ptr<block_queue<std::string>> log_queue_; // 阻塞队列
    bool is_async_; // 是否同步标志位
    locker mutex_; // 互斥锁
    int close_log_; // 是否关闭日志

private:
    Log();
    virtual ~Log();
    void async_write_log(); // 异步写日志函数

public:
    static Log* get_instance() {
        static Log instance;
        return &instance;
    }
    static void flush_log_thread() {    // 异步写日志线程函数
        Log::get_instance()->async_write_log();
    }

    bool init(const char* file_name, int close_log, int split_lines = 5000000, int log_buf_size = 8192,  int max_queue_size = 0);
    void write_log(int level, const char* format, ...);
    void flush();

    // 禁止拷贝和赋值
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    // 禁止移动
    Log(Log&&) = delete;
    Log& operator=(Log&&) = delete;

};

#define XXX(level, format, ...) if (close_log_==0) { Log::get_instance()->write_log(level, format, ##__VA_ARGS__); Log::get_instance()->flush(); }

#define LOG_DEBUG(format, ...) XXX(0, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...)  XXX(1, format, ##__VA_ARGS__)        
#define LOG_WARN(format, ...)  XXX(2, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) XXX(3, format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...) XXX(4, format, ##__VA_ARGS__)


#endif // LOG_H