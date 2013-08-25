
#include "MessageQueue.h"

#include <deque>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#include <assert.h>

#include <ThreadPosix.h>

// ------------------------------------------------------------------------....

namespace
{

class TestQueueThread
    : public ThreadPosix::IRunner
{

    int m_id;

    MutexPosix& m_mutex;
    MessageQueue< std::string >& m_in_queue;
    MessageQueue< std::string >& m_out_queue;

    std::unique_ptr< ThreadPosix > m_thread;

public:

    TestQueueThread( int id,
                     MutexPosix& mutex,
                     MessageQueue< std::string >& in_queue,
                     MessageQueue< std::string >& out_queue )
        :
          m_id( id ),
          m_mutex( mutex ),
          m_in_queue( in_queue ),
          m_out_queue( out_queue )
    {
        m_thread.reset( new ThreadPosix( *this ) );
    }

    virtual ~TestQueueThread( )
    { }

    virtual void
    run( ThreadPosix& thread )
    {
        (void) thread;

        trace( "Running" );

        std::string message;
        while( m_in_queue.pop( message, true ) )
        {
            trace( message );

            std::stringstream response;
            response << "Response to '" << message << " from '" << m_id << "'";

            while( 0 == m_out_queue.push( response.str( ) ) )
            {
                trace( "Waiting for a free slot into the output queue" );
                sched_yield( );
            }
        }

        trace( "Done." );
    }

    void
    trace( std::string message )
    {
        Locker< MutexPosix > locker( m_mutex );
        std::cerr << m_id << ": " << message << std::endl;
    }

};

} // anonymous namespace

// ----------------------------------------------------------------------------

void
test_MessageQueue( )
{
    const int NUM_THREADS = 10;
    const int NUM_MESSAGES = 1000;
    const int QUEUE_CAPACITY = 100;

    MutexPosix mutex;
    MessageQueue< std::string > queue_in( QUEUE_CAPACITY );
    MessageQueue< std::string > queue_out( QUEUE_CAPACITY );

    {
        std::vector< std::shared_ptr< TestQueueThread > > threads;
        threads.reserve( NUM_THREADS );
        for( int i = 0; i < NUM_THREADS; ++i )
        {
            threads.push_back( std::make_shared< TestQueueThread >( i + 1, mutex, queue_in, queue_out ) );
        }

        int num_messages_in = NUM_MESSAGES;
        int num_messages_out = NUM_MESSAGES;

        while( num_messages_in > 0
               || num_messages_out > 0 )
        {

            if ( num_messages_in > 0 )
            {
                std::stringstream stream;
                stream << "Message " << NUM_MESSAGES - num_messages_in;

                std::size_t num = queue_in.push( stream.str( ) );
                if ( num > 0 )
                {
                    --num_messages_in;
                    if( num < QUEUE_CAPACITY / 2 )
                    {
                        continue;
                    }
                }
                else
                {
                    sched_yield( );
                }
            }

            if ( num_messages_out > 0 )
            {
                std::string message;
                std::size_t num = queue_out.pop( message, num_messages_in > 0 );
                if( num > 0 )
                {
                    Locker< MutexPosix > locker( mutex );
                    std::cerr << 0 << ": " << message
                              << " #" << queue_in.size( )
                              << ":" << queue_out.size( ) << std::endl;
                    --num_messages_out;
                }
                else
                {
                    sched_yield( );
                }
            }
        }

        queue_in.cancel( );
    }
}

// ----------------------------------------------------------------------------
