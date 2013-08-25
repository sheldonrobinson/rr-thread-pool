#include "ThreadPool.h"

#include "MessageQueue.h"
#include "ThreadPosix.h"

#include <iostream>
#include <string>
#include <vector>


// -----------------------------------------------------------------------------

class ThreadPoolPosix
    :
    public IThreadPool,
    private ThreadPosix::IRunner
{
    typedef std::shared_ptr< ThreadPosix > ThreadPtr;
    typedef std::vector< ThreadPtr > Pool;

    Pool m_threads;
    std::unique_ptr< IMessageQueue > m_input_queue;
    std::unique_ptr< IMessageQueue > m_output_queue;
    bool m_cancelled;

public:

    ThreadPoolPosix( std::size_t num_threads,
                     std::size_t task_capacity )
        :
        m_cancelled( false )
    {
        // Creates the message queues (in/out) for the tasks:
        m_input_queue.reset( IMessageQueue::create( task_capacity ) );
        m_output_queue.reset( IMessageQueue::create( ) );

        // Creates the threads:
        m_threads.resize( num_threads );
        for( auto& thread: m_threads )
        {
            thread.reset( new ThreadPosix( *this ) );
        }
    }

    virtual
    ~ThreadPoolPosix()
    {
        join( );
    }

    virtual std::size_t
    push( ITaskPtr task )
    {
        // Precondition verification:
        assert( nullptr != task.get( ) );
        assert( !m_cancelled );

        // Tries to push the task in the form of message to the input queue:
        return m_input_queue->push( task );
    }

    virtual std::size_t
    pop( ITaskPtr& task, bool blocking )
    {
        // Precondition verification:
        assert( !m_cancelled );

        // Fetches the next executed task in the form of message:
        return m_output_queue->pop( task, blocking );
    }

    virtual void
    cancel( )
    {
        m_input_queue->cancel( );
        m_cancelled = true;
    }

    virtual void
    join( )
    {
        // Cancel the input queue in order to terminate all wotkers:
        cancel( );

        // Joins all workers threads:
        for( auto& thread: m_threads )
        {
            thread->join( );
        }

        // Transfers all pending tasks from the input queue to the output one:
        ITaskPtr task;
        while( m_input_queue->pop( task, false ) > 0 )
        {
            m_output_queue->push( task );
        }
    }

private:

    virtual void
    run( ThreadPosix& thread )
    {
        (void) thread;

        assert( nullptr != m_input_queue.get( ) );
        assert( nullptr != m_output_queue.get( ) );

        // For each fetched message:
        ITaskPtr task;
        while( m_input_queue->pop( task, true ) )
        {
            task->execute( );
            m_output_queue->push( task );
        }

        assert( m_input_queue->is_cancelled( ) );
    }

};

// -----------------------------------------------------------------------------

IThreadPool*
IThreadPool::create( std::size_t num_threads,
                     std::size_t task_capacity )
{
    return new ThreadPoolPosix( num_threads, task_capacity );
}

// -----------------------------------------------------------------------------
