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

public:
	MYSQL *GetConnection();				 //获取数据库连接
	bool ReleaseConnection(MYSQL *conn); //释放连接
	int GetFreeConn();					 //获取连接
	void DestroyPool();					 //销毁所有连接

	//单例模式
	static connection_pool *GetInstance();

	void init(std::string url, std::string User, std::string PassWord, std::string DataBaseName, int Port, int MaxConn, int close_log); 

};


class connectionRAII{

public:
	connectionRAII(MYSQL **con, connection_pool *connPool);
	~connectionRAII();
	
private:
	MYSQL *conRAII;
	connection_pool *poolRAII;
};


#endif // SQL_CONNECTION_POOL_HPP