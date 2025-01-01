//
// Created by 27229 on 24-11-4.
//
/*
这段代码封装了对 MySQL 数据库的常用操作，包括：
初始化和释放数据库连接。
连接数据库。
执行更新操作（INSERT、DELETE、UPDATE）。
执行查询操作（SELECT）并返回查询结果。
 */
#include "connection.h"
#include <mysql.h>
#include <string>
#include "public.h"
using namespace std;
// 数据库操作类

// 初始化数据库连接
connection::connection()
{
    _conn = mysql_init(nullptr);
}
// 释放数据库连接资源
connection::~connection()
{
    if (_conn != nullptr)
        mysql_close(_conn);
}
// 连接数据库
bool connection::connect(string ip, unsigned short port, string user, string password,
             string dbname)
{
    MYSQL *p = mysql_real_connect(_conn, ip.c_str(), user.c_str(),
                                  password.c_str(), dbname.c_str(), port, nullptr, 0);
    return p != nullptr;
}
// 更新操作 insert、delete、update
bool connection::update(string sql)
{
    if (mysql_query(_conn, sql.c_str()))// 执行sql语句
    {
        LOG("更新失败:" + sql);
        return false;
    }
    return true;
}
// 查询操作 select
MYSQL_RES* connection::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG("查询失败:" + sql);
        return nullptr;
    }
    return mysql_use_result(_conn);
}

