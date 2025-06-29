#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <pthread.h>
#include <iostream>
#include "sql_connection_pool.hpp"



connection_pool::connection_pool()
{
	cur_conn_ = 0;
	free_conn_ = 0;
}

connection_pool *connection_pool::GetInstance()
{
	static connection_pool connPool;
	return &connPool;
}

//构造初始化
void connection_pool::init(std::string url, std::string User, std::string PassWord, std::string DBName, int Port, int MaxConn, int close_log)
{
	url = url;
	port = Port;
	user = User;
	password = PassWord;
	database_name = DBName;
	close_log_ = close_log;

	for (int i = 0; i < MaxConn; i++)
	{
		MYSQL *con = NULL;
		con = mysql_init(con);

		if (con == NULL)
		{
			LOG_ERROR("MySQL Error");
			exit(1);
		}
		con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, NULL, 0);

		if (con == NULL)
		{
			LOG_ERROR("MySQL Error");
			exit(1);
		}
		conn_pool_.push_back(con);
		++free_conn_;
	}

	reserve_ = sem(free_conn_);

	max_conn_ = free_conn_;
}


//当有请求时，从数据库连接池中返回一个可用连接，更新使用和空闲连接数
MYSQL *connection_pool::GetConnection()
{
	MYSQL *con = NULL;

	if (0 == conn_pool_.size())
		return NULL;

	reserve_.wait();
	
	lock_.lock();

	con = conn_pool_.front();
	conn_pool_.pop_front();

	--free_conn_;
	++cur_conn_;

	lock_.unlock();
	return con;
}

//释放当前使用的连接
bool connection_pool::ReleaseConnection(MYSQL *con)
{
	if (NULL == con)
		return false;

	lock_.lock();

	conn_pool_.push_back(con);
	++free_conn_;
	--cur_conn_;

	lock_.unlock();

	reserve_.post();
	return true;
}

//销毁数据库连接池
void connection_pool::DestroyPool()
{

	lock_.lock();
	if (conn_pool_.size() > 0)
	{
		std::list<MYSQL *>::iterator it;
		for (it = conn_pool_.begin(); it != conn_pool_.end(); ++it)
		{
			MYSQL *con = *it;
			mysql_close(con);
		}
		cur_conn_ = 0;
		free_conn_ = 0;
		conn_pool_.clear();
	}

	lock_.unlock();
}

//当前空闲的连接数
int connection_pool::GetFreeConn()
{
	return this->free_conn_;
}

connection_pool::~connection_pool()
{
	DestroyPool();
}

connectionRAII::connectionRAII(MYSQL **SQL, connection_pool *connPool){
	*SQL = connPool->GetConnection();
	
	conRAII = *SQL;
	poolRAII = connPool;
}

connectionRAII::~connectionRAII(){
	poolRAII->ReleaseConnection(conRAII);
}