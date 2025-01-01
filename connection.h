//
// Created by 27229 on 24-11-4.
//

#ifndef CONNECTION_POOL_CONNECTION_H
#define CONNECTION_POOL_CONNECTION_H
#include "iostream"
#include <mysql.h>
using namespace std;

class connection
{
public:
    // 初始化数据库连接
    connection();
    // 释放数据库连接资源
    ~connection();
    // 连接数据库
    bool connect(string ip,
                 unsigned short port,
                 string user, string password,
                 string dbname);
    // 更新操作 insert、delete、update
    bool update(string sql);
    // 查询操作 select
    MYSQL_RES* query(string sql);
    //刷新一下连接的起始的空闲时间点
    void refreshsAliveTime(){ _alivetime = clock();}
    //返回存活的时间
    clock_t  getAliveeTime() const {return clock() - _alivetime;}
private:
    MYSQL *_conn; // 表示和MySQL Server的一条连接
    clock_t _alivetime; //记录进入空闲状态后的存活时间
};

#endif //CONNECTION_POOL_CONNECTION_H
