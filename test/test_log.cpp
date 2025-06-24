#include "log.hpp"
#include <unistd.h> // for sleep


// 创建包含日志开关的类
class ClassLog {
private:
    int close_log_=0; // 日志开关
public:
    void test_basic_logging() {
        // 初始化日志，异步模式（max_queue_size > 0）
        Log::get_instance()->init("testlog", 0, 1000, 8192, 100);

        // LOG_DEBUG("Debug message: %d", 1);
        // LOG_INFO("Info message: %s", "info test");
        // LOG_WARN("Warning message");
        // LOG_ERROR("Error message %.2f", 3.14);
        // LOG_FATAL("Fatal error");
        for (int i = 0; i < 10; ++i) {
            LOG_INFO("Sync log test %d", i);
        }

        // 给异步线程一些时间来写入日志
        sleep(1);
    }

    void test_sync_logging() {
        // 同步模式（max_queue_size == 0）
        Log::get_instance()->init("sync_testlog", 0, 1000, 8192, 0);

        for (int i = 0; i < 10; ++i) {
            LOG_INFO("Sync log test %d", i);
        }
    }

    void test_disable_logging() {
        // 关闭日志（close_log=1）
        Log::get_instance()->init("disabledlog", 1, 1000, 8192,0);

        LOG_INFO("This should not appear in the log.");
    }
};



int main() {
    ClassLog log_test;
    log_test.test_basic_logging();
    log_test.test_sync_logging();
    log_test.test_disable_logging();
    return 0;
}
