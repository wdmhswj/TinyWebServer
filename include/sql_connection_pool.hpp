#ifndef SQL_CONNECTION_POOL_HPP
#define SQL_CONNECTION_POOL_HPP

#include <mysql/mysql.h>
#include <string>
#include <list>
#include "locker.hpp"
#include "log.hpp"

class connection_pool {
public:

private:
    int max_conn_;   // 最大连接数
    int cur_conn_;   // 当前连接数
    int free_conn_;  // 空闲连接数
    locker lock_;    // 互斥锁
    std::list<MYSQL*> conn_pool_;    // 连接池
    sem reserve_;    // 信号量

public:
    std::string url;    // 主机地址
    std::string port;   // 数据库端口号
    std::string user;   // 登录数据库用户名
    std::string password; // 登录数据库密码
    std::string database_name; // 使用数据库名
    int close_log_; // 日志开关


};

#endif // SQL_CONNECTION_POOL_HPP