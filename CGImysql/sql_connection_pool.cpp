#include "sql_connection_pool.h"

using namespace std;

connection_pool::connection_pool(){
    m_CurConn = 0;
    m_FreeConn = 0;
}

connection_pool* connection_pool::GetInstance(){
    static connection_pool connPool;
    return &connPool;
}

void connection_pool::init( string url, string User, string Password, string DataBaseName, int port, int MaxConn, int close_log )
{
    // save data
    m_url = url;
    m_User = User;
    m_Password = Password;
    m_DatabaseName = DataBaseName;
    m_Port = to_string(port);
    m_MaxConn = MaxConn;
    m_close_log = close_log;
    for( int i = 0 ; i < MaxConn ; ++i )
    {
        MYSQL* con = NULL;
        con = mysql_init( con ); // get mysql handle

        if( con == NULL ) exit(1);
        // connect mysql
        con = mysql_real_connect( con, m_url.c_str(), m_User.c_str(), m_Password.c_str(), m_DatabaseName.c_str(), port, NULL, 0 );

        if( con == NULL ) exit(1);
        connList.push_back(con);
        ++m_FreeConn;
    }
    reserve = sem( m_FreeConn );
}

// find a free connection and update freeconn/curcon
MYSQL* connection_pool::GetConnection()
{
    MYSQL* con = NULL;
    if( connList.size() == 0 ) return NULL;
    reserve.wait();
    lock.lock();
    con = connList.front();
    connList.pop_front();
    ++m_CurConn;
    --m_FreeConn;
    lock.unlock();
    return con;
}

bool connection_pool::ReleaseConnection( MYSQL* conn )
{
    if( conn == NULL ) return false;
    
    lock.lock();
    connList.push_back(conn);
    ++m_FreeConn;
    --m_CurConn;
    lock.unlock();
    reserve.post();
    return true;
}

void connection_pool::DestroyPool()
{
    lock.lock();
	if (connList.size() > 0)
	{
        for( auto t : connList ) mysql_close(t);
		m_CurConn = 0;
		m_FreeConn = 0;
		connList.clear();
	}

	lock.unlock();
}

// current Free connection
int connection_pool::GetFreeConn()
{
    return this->m_FreeConn;
}

connection_pool::~connection_pool()
{
    DestroyPool();
}

// ------RAII-------- //
connectionRAII::connectionRAII( MYSQL** con, connection_pool* connPool ){
    *con = connPool->GetConnection();
    conRAII = *con;
    poolRAII = connPool;
}

connectionRAII::~connectionRAII()
{
    poolRAII->ReleaseConnection( conRAII );
}