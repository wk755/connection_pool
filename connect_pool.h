//
// Created by 27229 on 24-11-4.
//

#ifndef CONNECTION_POOL_CONNECT_POOL_H
#define CONNECTION_POOL_CONNECT_POOL_H

#include <string>
#include <queue>
#include <memory>
#include "mutex"
#include "functional"
#include "connection.h"
#include "atomic"
#include "thread"
#include "condition_variable"

using namespace std;

class connect_pool {
public:
    //获取连接池对象实例
    static connect_pool* getconnect_pool();
    //给外部提供接口，从连接池中获取一个可用的空闲连接
    shared_ptr<connection> getconnection();
private:
    connect_pool(); //单例 构造函数私有化

    bool loadConfigfile();

    //运行在独立的线程中，专门负责生产新连接
    void prodiceConnectionTask();

    //启动一个新的线程，扫描多余的空闲连接，超过maxIdleTime时间的空闲连接，进行多于的连接回收
    void scannerConnectionTask();

    string _ip; //mysql的ip地址
    unsigned short _port; //mysql的端口号
    string _username; //登录用户名
    string _password; //登陆密码
    string _dbname; //数据库名
    int _initSize; //连接池初始连接量
    int _maxSize; // 连接池最大连接量
    int _maxIdleTime; //连接池最大空闲时间
    int _connectionTimeout; //连接池获取连接的超时时间
    queue<connection*> _connectionQue; //存储mysql的队列
    mutex _queueMutex; //维护连接队列的线程安全互斥锁
    atomic_int _connectionCnt; //记录连接所创建的connection连接的总数量
    condition_variable cv; //设置条件变量，用于连接生产线程和连接消费线程的通信
};


#endif //CONNECTION_POOL_CONNECT_POOL_H
