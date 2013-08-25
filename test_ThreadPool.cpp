#include "ThreadPool.h"

#include "MessageQueue.h"
#include "ThreadPosix.h"

#include <iostream>
#include <string>
#include <vector>

// -----------------------------------------------------------------------------

namespace
{

class TestTask
    :
    public IThreadPool::ITask
{

    int m_id;
    MutexPosix& m_mutex;
    int& m_instance_counter;
    int& m_execution_counter;
    int m_step;

public:

    TestTask( int id,
              MutexPosix& mutex,
              int& instance_counter,
              int& execution_counter )
        :
        m_id( id ),
        m_mutex( mutex ),
        m_instance_counter( instance_counter ),
        m_execution_counter( execution_counter ),
        m_step( 0 )
    {
        {
            Locker< MutexPosix > locker( m_mutex );
            ++m_instance_counter;
        }

        trace( "created" );
    }

    virtual ~TestTask( )
    {
        {
            Locker< MutexPosix > locker( m_mutex );
            assert( m_instance_counter > 0 );
            --m_instance_counter;
        }

        trace( "destroyed" );

        assert( m_step == 1 );
        ++m_step;
    }

    virtual void execute( )
    {
        {
            Locker< MutexPosix > locker( m_mutex );
            ++m_execution_counter;
        }

        trace( "executed" );

        assert( m_step == 0 );
        ++m_step;
    }

    void
    trace( std::string message )
    {
        if ( m_id % 100000 == 0 )
        {
            Locker< MutexPosix > locker( m_mutex );
            std::cerr << m_id << ": " << message << std::endl;
        }
    }

};

} // anonymous namespace

// -----------------------------------------------------------------------------

void
test_ThreadPool( )
{
    const int NUM_THREADS = 16;
    const int NUM_TASKS = 1000000;
    const int QUEUE_CAPACITY = 100;

    std::unique_ptr< IThreadPool > pool(
                IThreadPool::create( NUM_THREADS, QUEUE_CAPACITY ) );

    MutexPosix mutex;
    int num_tasks_in = NUM_TASKS;
    int num_tasks_out = NUM_TASKS;
    int instance_counter = 0;
    int execution_counter = 0;

    while( num_tasks_in > 0
           || num_tasks_out > 0 )
    {

        if ( num_tasks_in > 0 )
        {
            int id = NUM_TASKS - num_tasks_in;

            IThreadPool::ITaskPtr task( new TestTask( id,
                                                      mutex,
                                                      instance_counter,
                                                      execution_counter ) );

            std::size_t num = pool->push( task );
            if ( num > 0 )
            {
                --num_tasks_in;
            }
            else
            {
                sched_yield( );
            }
        }

        if ( num_tasks_out > 0 )
        {
            std::shared_ptr< TestTask > task;
            std::size_t num = pool->pop( task, num_tasks_in > 0 );
            if( num > 0 )
            {
                task->trace( "collected" );
                --num_tasks_out;
            }
            else
            {
                sched_yield( );
            }
        }

    }

    pool->join( );

    assert( 0 == instance_counter );
    assert( NUM_TASKS == execution_counter );
}

// -----------------------------------------------------------------------------

