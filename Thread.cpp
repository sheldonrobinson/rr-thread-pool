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

#include "Thread.h"

#include "Cond.h"
#include "Mutex.h"
#include "Trace.h"

#include <pthread.h>
#include <sched.h>
#include <sys/types.h>

// -----------------------------------------------------------------------------

class ThreadPosix;
typedef std::shared_ptr< ThreadPosix > ThreadPosixPtr;

struct ThreadInfo
{
    ThreadInfo( Thread thread )
        : m_thread( thread )
    { }

    Thread m_thread;
};

class ThreadRegister
{
     pthread_key_t m_key;

public:

     ThreadRegister( )
     {
         ::pthread_key_create( &m_key, destroy_object );
     }

     ~ThreadRegister( )
     {
         ::pthread_key_delete( m_key );
     }

     void register_object( Thread thread );

     Thread current( );

     static void destroy_object( void *opaque )
     {
         ThreadInfo* info = reinterpret_cast< ThreadInfo* >( opaque );
         assert( info != nullptr );
         delete info;
     }

};

static ThreadRegister thread_register;

// -----------------------------------------------------------------------------

class ThreadPosix
    : public IThread
{
    friend class ThreadRegister;

    struct InitData
    {
        Thread m_self;
        Task m_task;
        volatile bool& m_running;

        Cond m_cond;
        Mutex m_mutex;

        InitData( Thread self, Task task, volatile bool& running )
            : m_self( self ),
              m_task( task ),
              m_running( running )
        { }

    };

    pthread_t m_thread;
    volatile bool m_running;

public:

    ThreadPosix( bool fetch_self )
        : m_running( false )
    {
        if ( fetch_self )
        {
            m_thread = ::pthread_self( );
            m_running = true;
        }
    }

    void
    init( ThreadPosixPtr& self, Task task )
    {
        assert( self.get() == this );

        pthread_attr_t attr;
        ::pthread_attr_init( &attr );
        ::pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE );

        {
            InitData init_data( self, task, m_running );
            Locker< Mutex > lock( init_data.m_mutex );

            ::pthread_create( &m_thread, &attr, run_thread, &init_data );

            init_data.m_cond.wait( init_data.m_mutex );
        }

        ::pthread_attr_destroy( &attr );
    }

    virtual
    ~ThreadPosix( )
    {
        if( m_thread != ::pthread_self( ) )
        {
            join( );
        }
    }

    virtual bool
    is_running( ) const
    {
        return m_running;
    }

    virtual void
    join( )
    {
        assert( m_thread != ::pthread_self( ) );
        ::pthread_join( m_thread, nullptr );
    }

    virtual void yield( ) const
    {
        assert( m_thread == ::pthread_self( ) );
        ::sched_yield( );
    }

    virtual void*
    handle( )
    {
        return reinterpret_cast< void *>( m_thread );
    }

private:

    static void*
    run_thread( void* par )
    {
        {
            Thread self;
            Task task;
            volatile bool* running_flag = 0;

            {
                // This is needed to keep the thread and task objects alive during
                // the entire execution:
                InitData& init_data = *( reinterpret_cast< InitData* >( par ) );
                Locker< Mutex > locker( init_data.m_mutex );
                self = init_data.m_self;
                task = init_data.m_task;
                running_flag = &( init_data.m_running );
                ( *running_flag ) = true;

                thread_register.register_object( self );
                init_data.m_cond.signal( );
            }

            task->execute( );

            ( *running_flag ) = false;
        }

        // trace( "Done!" );

        // pthread_exit( nullptr );
        return nullptr;
    }

};

// -----------------------------------------------------------------------------

Thread
IThread::create( Task task )
{
    ThreadPosixPtr new_thread( std::make_shared< ThreadPosix >( false ) );
    assert( new_thread.get( ) != nullptr );

    new_thread->init( new_thread, task );

    return new_thread;
}

// -----------------------------------------------------------------------------

Thread
IThread::self( )
{
    return thread_register.current( );
}

// -----------------------------------------------------------------------------

void
ThreadRegister::register_object( Thread thread )
{
    ::pthread_setspecific( m_key, new ThreadInfo( thread ) );
}

// -----------------------------------------------------------------------------

Thread
ThreadRegister::current( )
{
    void* opaque = pthread_getspecific( m_key );
    if ( opaque != nullptr )
    {
        ThreadInfo* info = reinterpret_cast< ThreadInfo* >( opaque );
        assert( info != nullptr );
        return info->m_thread;
    }

    Thread thread( new ThreadPosix( true ) );
    ::pthread_setspecific( m_key, new ThreadInfo( thread ) );
    assert( pthread_getspecific( m_key ) != nullptr );

    return thread;
}

// -----------------------------------------------------------------------------
