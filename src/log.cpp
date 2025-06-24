#include "log.hpp"
#include "string.h"

Log::Log() {
    count_ = 0; // 初始化日志行数计数
    is_async_ = false; // 默认不使用异步日志
    close_log_ = 0; // 默认不关闭日志
}
Log::~Log() {
    if (fp_) {
        // fclose(fp_.get()); // 关闭文件指针
        fclose(fp_); // 关闭文件指针
        fp_ = nullptr; // 设置为空
    }
}
void Log::async_write_log() {
    std::string single_log;
    while (log_queue_->pop(single_log)) { // 从阻塞队列中取出日志
        mutex_.lock(); // 加锁
        // fputs(single_log.c_str(), fp_.get()); // 写入日志到文件
        fputs(single_log.c_str(), fp_); // 写入日志到文件
        mutex_.unlock(); // 解锁
    }
}

bool Log::init(const char* file_name, int close_log, int split_lines, int log_buf_size, int max_queue_size) {
    // 如果设置了max_queue_size，则使用异步日志
    if (max_queue_size > 0) {
        is_async_ = true;
        log_queue_ = std::make_shared<block_queue<std::string>>(max_queue_size);
    } else {
        is_async_ = false;
    }

    close_log_ = close_log; // 设置是否关闭日志
    log_buf_size_ = log_buf_size; // 设置日志缓冲区大小
    buf_.resize(log_buf_size, '\0'); // 初始化日志缓冲区
    split_lines_ = split_lines; // 设置每个日志文件的最大行数

    time_t t = time(nullptr);
    struct tm* sys_tm = localtime(&t); // 获取当前时间
    struct tm my_tm = *sys_tm; // 复制当前时间

    const char* p = strrchr(file_name, '/'); // 查找最后一个斜杠
    if (p == nullptr) {
        dir_name_ = "./"; // 如果没有斜杠，则默认目录为当前目录
        log_name_ = file_name; // 日志文件名为file_name
    } else {
        dir_name_.assign(file_name, p - file_name + 1); // 目录名为斜杠前的部分
        log_name_ = p + 1; // 日志文件名为斜杠后的部分
    }

    char log_full_name[256] = {0}; // 日志文件全名
    snprintf(log_full_name, sizeof(log_full_name) - 1, "%s%d_%02d_%02d_%s", dir_name_.c_str(), my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, log_name_.c_str());
    today_ = my_tm.tm_mday; // 记录今天的日期
    // fp_ = std::shared_ptr<FILE>(fopen(log_full_name, "a")); // 打开日志文件
    fp_ = fopen(log_full_name, "a"); // 打开日志文件
    if (!fp_) {
        std::cerr << "Error opening log file: " << log_full_name << std::endl;
        return false; // 打开日志文件失败
    }
    return true; // 初始化成功
}

void Log::write_log(int level, const char* format, ...) {
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr); // 获取当前时间
    time_t t = now.tv_sec; // 秒
    struct tm* sys_tm = localtime(&t); // 转换为本地时间
    struct tm my_tm = *sys_tm; // 复制本地时间

    char s[16] = {0}; // 时间字符串
    switch (level) {
        case 0: strcpy(s, "[debug]:"); break;
        case 1: strcpy(s, "[info]:"); break;
        case 2: strcpy(s, "[warn]:"); break;
        case 3: strcpy(s, "[error]:"); break;
        default: strcpy(s, "[info]:"); break; // 默认级别为info
    }

    mutex_.lock(); // 加锁
    ++count_; // 增加日志行数计数
    if (today_ != my_tm.tm_mday || count_ % split_lines_ == 0) {    // 如果日期变化或行数达到上限，则重新打开日志文件
        char new_log[256] = {0};
        // fflush(fp_.get()); // 刷新文件缓冲区
        // fclose(fp_.get()); // 关闭当前日志文件
        fflush(fp_); // 刷新文件缓冲区
        fclose(fp_); // 关闭当前日志文件
        char tail[16] = {0}; // 日志文件尾部
        snprintf(tail, sizeof(tail)-1, "%d_%02d_%02d_", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);

        if (today_ != my_tm.tm_mday) {
            snprintf(new_log, sizeof(new_log)-1, "%s%s%s", dir_name_.c_str(), tail, log_name_.c_str()); // 新日志文件名
            today_ = my_tm.tm_mday; // 更新今天的日期
        } else {
            snprintf(new_log, sizeof(new_log)-1, "%s%s%s.%lld", dir_name_.c_str(), tail, log_name_.c_str(), count_ / split_lines_); // 日志文件名加上行数
        }
        // fp_ = std::shared_ptr<FILE>(fopen(new_log, "a")); // 打开新的日志文件
        fp_ = fopen(new_log, "a"); // 打开新的日志文件

    }

    mutex_.unlock(); // 解锁

    va_list valst; // 可变参数列表
    va_start(valst, format); // 初始化可变参数列表

    std::string log_str;

    mutex_.lock(); // 加锁

    int n = snprintf(buf_.data(), 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, s);
    int m = vsnprintf(buf_.data() + n, log_buf_size_ - 1 - n, format, valst); // 格式化日志内容
    buf_[n + m] = '\n'; // 添加换行符
    buf_[n + m + 1] = '\0'; // 添加字符串结束符
    log_str = std::string(buf_.data()); // 转换为字符串
    mutex_.unlock(); // 解锁

    if(is_async_ && !log_queue_->full()) {
        log_queue_->push(log_str); // 如果是异步日志，则将日志内容放入阻塞队列
    } else {
        mutex_.lock(); // 如果不是异步日志，则直接写入文件
        // fputs(log_str.c_str(), fp_.get()); // 写入日志到文件
        fputs(log_str.c_str(), fp_); // 写入日志到文件
        mutex_.unlock(); // 解锁
    }

    va_end(valst); // 结束可变参数列表
}
void Log::flush() {
    mutex_.lock();
    if (fp_) {
        // fflush(fp_.get()); // 刷新文件缓冲区
        fflush(fp_); // 刷新文件缓冲区
    }
    mutex_.unlock(); // 解锁
}