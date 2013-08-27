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

#include <assert.h>
#include <pthread.h>

// ------------------------------------------------------------------------

class MutexPosix
{

    pthread_mutex_t m_mutex;

public:

    MutexPosix( )
    {
        ::pthread_mutex_init( &m_mutex, NULL );
    }

    ~MutexPosix( )
    {
        ::pthread_mutex_destroy( &m_mutex );
    }

    void
    lock( )
    {
        ::pthread_mutex_lock( &m_mutex );
    }

    void
    unlock( )
    {
        ::pthread_mutex_unlock( &m_mutex );
    }

    friend class CondPosix;

};

// ------------------------------------------------------------------------

class CondPosix
{

    pthread_cond_t m_cond;

public:

    CondPosix( )
    {
        ::pthread_cond_init( &m_cond, NULL );
    }

    ~CondPosix( )
    {
        ::pthread_cond_destroy( &m_cond );
    }

    void
    wait( MutexPosix& mutex )
    {
        pthread_cond_wait( &m_cond, &mutex.m_mutex );
    }

    void
    signal( )
    {
        pthread_cond_signal( &m_cond );
    }

    void
    broadcast( )
    {
        pthread_cond_broadcast( &m_cond );
    }

    friend class CondPosix;

};

// ------------------------------------------------------------------------

template< typename L >
class Locker
{

    L& m_target;

public:

    Locker( L& target )
        : m_target( target )
    {
        m_target.lock( );
    }

    ~Locker( )
    {
        m_target.unlock( );
    }

};

// ------------------------------------------------------------------------

class ThreadPosix
{

public:

    class IRunner
    {

    public:

        virtual ~IRunner( ) { }
        virtual void run( ThreadPosix& thread ) = 0;
    };

    ThreadPosix( IRunner& runner )
        :
        m_runner( runner )
    {
        pthread_create( &m_thread, NULL, posix_run, this );
    }

    virtual ~ThreadPosix( )
    {
        join( );
        pthread_detach( m_thread );
    }

    virtual void join( )
    {
        pthread_join( m_thread, NULL );
    }

    virtual void yield( )
    {
        sched_yield( );
    }

private:

    IRunner& m_runner;
    pthread_t m_thread;

    static void *
    posix_run( void * par )
    {
        ThreadPosix *thread = reinterpret_cast< ThreadPosix *>( par );
        assert( thread );

        thread->m_runner.run( *thread );

        return nullptr;
    }


};

// -----------------------------------------------------------------------------
