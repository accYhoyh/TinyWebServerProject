#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "locker.h"
#include "sql_connection_pool.h"

template< typename T >
class threadpool
{
public:
    threadpool( int actor_model, connection_pool* conn, int thread_number = 8, int max_request = 10000 );
    ~threadpool();
    bool append( T* request, int state );   // Reactor model append
    bool append_p( T* request );            // Proactor model append
private:
    static void* worker( void* arg );
    void run();
private:
    int m_thread_number;            // thread_number
    int m_max_requests;             // max_request
    pthread_t* m_threads;           // thread pointer
    std::list< T* > m_workerqueue;  // worker request line
    locker m_queuelocker;           // protect worker queue
    sem m_queuestat;                // wheather have work
    connection_pool* m_connPool;    // mysql
    int m_actor_model;              // actor model  0 Reactor   1 Proactor
};

template< typename T >
threadpool<T>::threadpool( int actor_model, connection_pool* conn, int thread_number, int max_requests ):m_actor_model(actor_model), m_connPool(conn), m_thread_number(thread_number), m_max_requests(max_requests), m_threads(NULL)
{
    if( thread_number <= 0 || max_requests <= 0 ) throw std::exception();
    m_threads = new pthread_t[m_thread_number];
    if( !m_threads ) throw std::exception();
    for( int i = 0 ; i < m_thread_number ; ++i )
    {
        if( pthread_create(m_threads+i, NULL, worker, this) != 0 )
        {
            delete [] m_threads;
            throw std::exception();
        }
        if( pthread_detach(m_threads[i]) )
        {
            delete [] m_threads;
            throw std::exception();
        }
    }
}

template< typename T >
threadpool<T>::~threadpool()
{
    delete [] m_threads;
}

template< typename T >
bool threadpool<T>::append( T* request, int state )
{
    m_queuelocker.lock();
    if( m_workerqueue.size() >= m_max_requests )
    {
        m_queuelocker.unlock();
        return false;
    }
    request->m_state = state; // set request (read or wirte)
    m_workerqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

template< typename T >
bool threadpool<T>::append_p( T* request )
{
    m_queuelocker.lock();
    if( m_workerqueue.size() >= m_max_requests )
    {
        m_queuelocker.unlock();
        return false;
    }
    m_workerqueue.push_back(request);
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

template< typename T >
void* threadpool<T>::worker( void* arg )
{
    threadpool* pool = (threadpool*)arg;
    pool->run();
    return pool;
}
template< typename T >
void threadpool<T>::run()
{
    while( true )
    {
        m_queuestat.wait();
        m_queuelocker.lock();
        if( m_workerqueue.size() == 0 )
        {
            m_queuelocker.unlock();
            continue;
        }
        T* request = m_workerqueue.front();
        m_workerqueue.pop_front();
        m_queuelocker.unlock();
        if( !request ) continue;
        
        if( m_actor_model == 1 ) // Reactor
        {
            if( request->m_state == 0 ) // read
            {
                if( request->read_once() )
                {
                    request->improv = 1;
                    connectionRAII( &request->mysql, m_connPool );
                    request->process();
                }
                else // read error
                {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }
            else // write
            {
                if( request->write() )
                {
                    request->improv = 1;
                }
                else
                {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }
        }
        else // Proactor    I/O alread finish
        {
            connectionRAII mysqlcon( &request->mysql, m_connPool );
            request->process();
        }
    }
}

#endif