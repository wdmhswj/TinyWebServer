#ifndef HTTP_CONN_HPP
#define HTTP_CONN_HPP

#include <netinet/in.h>
#include <sys/stat.h>
#include <map>


#include "locker.hpp"
#include "sql_connection_pool.hpp"
#include "log.hpp"

class http_conn {
public:
    // 常量
    static const int FILE_NAME_LEN = 200;   // 文件名最大长度
    static const int READ_BUFFER_SIZE = 2048;   // 读缓冲区大小
    static const int WRITE_BUFFER_SIZE = 1024;  // 写缓冲区大小

    // HTTP请求方法枚举
    enum METHOD
    {
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };

    // 主状态机当前所处的解析状态
    enum CHECK_STATE {
        CHECK_STATE_REQUEST_LINE,   // 请求行
        CHECK_STATE_HEADER, // 请求头
        CHECK_STATE_CONTENT // 请求体
    };

    enum HTTP_CODE
    {
        NO_REQUEST,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };

    enum LINE_STATUS
    {
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };

    http_conn() {}
    ~http_conn() {}

    void init(int sockfd, const sockaddr_in &addr, char *, int, int, std::string user, std::string passwd, std::string sqlname);
    void close_conn(bool real_close = true);
    void process();
    bool read_once();
    bool write();
    sockaddr_in *get_address()
    {
        return &address_;
    }
    void initmysql_result(connection_pool *connPool);
    int timer_flag;
    int improv; 

private:
    void init();
    HTTP_CODE process_read();
    bool process_write(HTTP_CODE ret);
    HTTP_CODE parse_request_line(char *text);
    HTTP_CODE parse_headers(char *text);
    HTTP_CODE parse_content(char *text);
    HTTP_CODE do_request();
    char *get_line() { return read_buf_ + start_line_; };
    LINE_STATUS parse_line();
    void unmap();
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_length);
    bool add_content_type();
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();

public:
    static int epollfd_;
    static int user_count_;
    MYSQL *mysql_;
    int state_;  //读为0, 写为1


private:
    int sockfd_; // 该http连接的socket
    sockaddr_in address_;    // 客户端地址
    char read_buf_[READ_BUFFER_SIZE];   // 读缓存区
    long read_idx_; // 已读入的数据的最后一个字节的下一个位置
    long checked_idx_;  // 正在解析的字符位置
    int start_line_;    // 当前行在缓冲区的起始位置
    char write_buf_[WRITE_BUFFER_SIZE]; // 写缓冲区
    int write_idx_; // 
    CHECK_STATE check_state_;   // 主状态机当前状态
    METHOD method_; // 请求方法

    char real_file_[FILE_NAME_LEN];  // 客户请求的资源完整路径
    char* url_;   // 请求目标文件的URL
    char* version_;   // HTTP协议版本
    char* host_;  // 主机名
    long content_length_;    // HTTP请求体长度
    bool linger_;   // 是否保持连接

    char* file_address_;          // 文件映射后在内存中的起始地址
    struct stat file_stat_;       // 目标文件的状态（是否存在、是否可读等）
    struct iovec iv_[2];          // writev结构体
    int iv_count_;                // 被写内存块数量

    int cgi_;            // 是否启用CGI（处理POST请求）
    std::string string_;     // 存储POST请求体数据（如用户名、密码）
    int bytes_to_send;  // 剩余待发送字节数
    int bytes_have_send;// 已发送字节数

    std::string doc_root_;     // 网站根目录

    locker lock_;
    std::map<std::string, std::string> users_; // 从数据库读取的用户信息

    int TRIGMode_;     // 触发模式（ET还是LT）
    int close_log_;    // 是否关闭日志

    std::string sql_user_;     // 数据库用户名
    std::string sql_passwd_;   // 数据库密码
    std::string sql_name_;     // 数据库名

};

#endif