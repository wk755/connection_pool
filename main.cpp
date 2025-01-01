#include <iostream>
#include <fstream>
#include "connection.h"
#include "connect_pool.h"
using namespace std;

int main() {



    /*connection conn;
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name,age,sex) values('%s', %d, '%s')", "zhang san", 20,"male" );
    conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
    conn.update(sql);*/
    clock_t begin = clock();
    // 静态函数必须使用类名访问
    thread t1([] (){
        connect_pool* cp = connect_pool::getconnect_pool();
        for(int i=0; i<250; ++i)
        {
            char sql[1024] = {0};
            sprintf(sql, "insert into user(name,age,sex) "
                         "values('%s', %d, '%s')", "zhang san", 20,"male" );
            shared_ptr<connection> sp = cp->getconnection();
            sp->update(sql);
        }
    });
    thread t2([] (){
        connect_pool* cp = connect_pool::getconnect_pool();
        for(int i=0; i<250; ++i)
        {
            shared_ptr<connection> sp = cp->getconnection();
            char sql[1024] = {0};
            sprintf(sql, "insert into user(name,age,sex) "
                         "values('%s', %d, '%s')", "zhang san", 20,"male" );
            sp->update(sql);
        }

    });
    thread t3([] (){
        connect_pool* cp = connect_pool::getconnect_pool();
        for(int i=0; i<250; ++i)
        {
            shared_ptr<connection> sp = cp->getconnection();
            char sql[1024] = {0};
            sprintf(sql, "insert into user(name,age,sex) "
                         "values('%s', %d, '%s')", "zhang san", 20,"male" );
            sp->update(sql);
        }

    });
    thread t4([] (){
        connect_pool* cp = connect_pool::getconnect_pool();
        for(int i=0; i<250; ++i)
        {
            shared_ptr<connection> sp = cp->getconnection();
            char sql[1024] = {0};
            sprintf(sql, "insert into user(name,age,sex) "
                         "values('%s', %d, '%s')", "zhang san", 20,"male" );
            sp->update(sql);
        }

    });
    /*for(int i=0; i<5000; ++i)
    {
        /*connection conn;
        char sql[1024] = {0};
        sprintf(sql, "insert into user(name,age,sex) "
                     "values('%s', %d, '%s')", "zhang san", 20,"male" );
        conn.connect("127.0.0.1", 3306, "root", "123456", "chat");
        conn.update(sql);*/
        /*shared_ptr<connection> sp = cp->getconnection();
        char sql[1024] = {0};
        sprintf(sql, "insert into user(name,age,sex) "
                     "values('%s', %d, '%s')", "zhang san", 20,"male" );
        sp->update(sql);*/
    t1.join();
    t2.join();
    t3.join();
    t4.join();

    clock_t end = clock();
    cout << (end-begin) << "ms" << endl;
    return 0;
}
