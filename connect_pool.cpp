//
// Created by 27229 on 24-11-4.
//
#include "connect_pool.h"
#include "public.h"

//连接池的构造
connect_pool::connect_pool(){
    //加载配置文件，如果加载失败，则直接返回
    if(!loadConfigfile())
    {
        return;
    }
    //根据配置文件中的初始大小创建连接
    for(int i=0; i<_initSize; i++){
        connection *p = new connection();
        //使用配置信息连接数据库
        p->connect(_ip, _port, _username, _password, _dbname);
        //将创建的连接放入连接队列中
        _connectionQue.push(p);
        //增加连接计数
        _connectionCnt++;
    }

    //启动一个新的线程，作为连接的生产者
    thread produce(std::bind(&connect_pool::prodiceConnectionTask, this));
    //将生产者线程分离，使其在后台独立运行
    produce.detach();
    //启动一个新的线程，扫描多余的空闲连接，超过maxIdleTime时间的空闲连接，进行多于的连接回收
    thread scanner(std::bind(&connect_pool::scannerConnectionTask, this));
    //将扫描器线程分离，使其在后台独立运行
    scanner.detach();
};

//线程安全的懒汉单例函数接口
connect_pool* connect_pool::getconnect_pool()
{
    //静态变量，保证只被初始化一次
    static connect_pool pool;
    //返回连接池实例的指针
    return &pool;
};

//加载配置文件，初始化数据库连接参数
bool connect_pool::loadConfigfile(){
    //使用 fopen 打开名为 mysql.ini 的文件，模式为只读 ("r")，返回的文件指针存储在 pf 中。
    FILE *pf = fopen("mysql.ini","r");
    //如果文件不存在，记录日志并返回false
    if(pf == nullptr)
    {
        LOG("mysql.ini file is not exit!");
        return false;
    }
    //读取配置文件中的每一行，直到文件结束
    while(!feof(pf)) //函数feof(pf)会检查文件指针pf是否已到达文件末尾
    {
        char line[1024] = {0};
        fgets(line,1023,pf);//使用 fgets 从文件中读取最多 1023 个字符，并将它们存储到 line 中。
        // fgets 会在读取到换行符或文件结束时停止。就是读每一行内容保存到line中。
        string str = line;//将line内容转变为字符串
        //查找配置项的键值对
        int idx= str.find('=',0); //从第0个字符开始搜索"="，找到后返回该位置的索引。
        //如果找不到等号，跳过当前行
        if(idx == -1)
        {
            continue;
        }
        int endidx = str.find('\n', idx); //从当前位置开始搜索换行符，找到后返回该位置的索引。
        string key = str.substr(0,idx); //即搜索"="前面的字符串，即配置项的键，例如用户名、密码等。
        string value = str.substr(idx+1, endidx -idx -1); //即搜索"="后面的字符串，即配置项的值
        //根据配置项的键设置相应的值
        if (key == "ip")
        {
            _ip = value;
        }
        else if (key == "port")
        {
            _port = atoi(value.c_str()); //将字符串转换为整数
        }
        else if (key == "username")
        {
            _username = value;
        }
        else if (key == "password")
        {
            _password = value;
        }
        else if (key == "dbname")
        {
            _dbname = value;
        }
        else if (key == "initSize")
        {
            _initSize = atoi(value.c_str());
        }
        else if (key == "maxSize")
        {
            _maxSize = atoi(value.c_str());
        }
        else if (key == "maxIdleTime")
        {
            _maxIdleTime = atoi(value.c_str());
        }
        else if (key == "connectionTimeOut")
        {
            _connectionTimeout = atoi(value.c_str());
        }
    }
    //配置文件加载完成，返回true
    return true;
}

//运行在独立的线程中，专门负责生产新连接
void connect_pool::prodiceConnectionTask(){
    //无限循环，持续生产连接
    for(;;)
    {
        unique_lock<mutex> lock(_queueMutex);//互斥锁
        //如果连接队列不为空，等待消费者使用连接
        while(!_connectionQue.empty()){
            cv.wait(lock);//等待被唤醒
        }

        //如果连接数量未达到上限，创建新的连接
        if(_connectionCnt < _maxSize){
            connection* p = new connection();
            p->connect( _ip,  _port, _username, _password, _dbname);
            p->refreshsAliveTime();
            _connectionQue.push(p);
            _connectionCnt++;
        }
        //通知所有等待的线程，连接已经生产
        cv.notify_all();
    }
}

//给外部提供接口，从连接池中获取一个可用的空闲连接
shared_ptr<connection> connect_pool::getconnection()
{
    unique_lock<mutex> lock(_queueMutex);
    //如果连接队列为空，等待生产者生产连接
    while(_connectionQue.empty())
    {
        if( cv_status::timeout == cv.wait_for(lock, chrono::microseconds (_connectionTimeout))){
            if(_connectionQue.empty())
            {
                LOG("获取空闲连接超时了。。。获取连接失败！");
                return nullptr;
            }
        }
    }
    //使用自定义的释放策略创建智能指针
    shared_ptr<connection> sp(_connectionQue.front(), [&](connection *pcon)//lambda表达式，定义了一个自定义的释放策略
    {
        unique_lock<mutex> lock(_queueMutex);//会自动在作用域结束时解锁，并且支持显式解锁和重新加锁，此时是智能指针的析构，
        //多个线程可能在不同的时机销毁 shared_ptr，所以需要加锁
        pcon->refreshsAliveTime();
        _connectionQue.push(pcon);
    });
    _connectionQue.pop();//弹出一个连接
    cv.notify_all();
    return sp;
}

//扫描超过maxIdleTime时间的空闲连接，进行多余的连接回收
void connect_pool::scannerConnectionTask()
{
    for(;;)
    {
        this_thread::sleep_for(chrono::seconds(_maxIdleTime)); //线程休眠一段时间，每过这个时间，扫描回收一次连接池
        unique_lock<mutex> lock(_queueMutex);
        //如果当前连接数大于初始大小，回收多余的连接
        while( _connectionCnt > _initSize)
        {
            connection *p = _connectionQue.front();
            //如果连接的空闲时间超过最大空闲时间，回收该连接
            if(p->getAliveeTime() >= (_maxIdleTime * 1000))
            {
                _connectionQue.pop();
                _connectionCnt--;
                delete p;
            }
            else
            {
                break;
            }
        }
    }
}




