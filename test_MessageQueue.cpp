/**
Copyright (c) 2013, Riccardo Ressi
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list
of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

Neither the name of Riccardo Ressi nor the names of its contributors may be
used to endorse or promote products derived from this software without specific
prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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
