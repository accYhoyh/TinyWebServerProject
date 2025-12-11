#ifndef _SQL_CONNECTION_POOL_H_
#define _SQL_CONNECTION_POOL_H_

#include <stdio.h>
#include <list>
#include <mysql/mysql.h>
#include <error.h>
#include <string.h>
#include <iostream>
#include <string>
#include "locker.h"

using namespace std;

class connection_pool{
private:
    connection_pool();
    ~connection_pool();

public:
    MYSQL *GetConnection();                 // get mysql connection
    bool ReleaseConnection(MYSQL* conn);    // release mysql connection
    int GetFreeConn();                      // get free connection count -> server can use
    void DestroyPool();                     // destroy all connection

    // singleton pattern
    static connection_pool* GetInstance();

    void init( string url, string User, string Password, string DataBaseName, int port, int MaxConn, int close_log );

private:
    int m_MaxConn;  // max connection
    int m_CurConn;  // current used connection
    int m_FreeConn; // current free connection
    locker lock;
    list< MYSQL* > connList;    // connection_pool
    sem reserve;    // reserve -> m_FreeConn  <---> thread syn

public:
    string m_url;           // host address
    string m_Port;          // mysql port(default 3306)
    string m_User;          // log in mysql username
    string m_Password;      // log in mysql password
    string m_DatabaseName;  // the used database name
    int m_close_log;        // log open
};

class connectionRAII{
private:
    MYSQL* conRAII;             // get one mysql connection
    connection_pool* poolRAII;  // get the mysql_pool(can return the mysql connection)
public:
    connectionRAII( MYSQL** con, connection_pool* connPool ); // double ** becase need to change the MYSQL*
    ~connectionRAII();
};


#endif