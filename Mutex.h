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

#include <assert.h>
#include <memory>

#ifndef MUTEX_H
#define MUTEX_H

// ------------------------------------------------------------------------

/**
 * @brief A classical mutex meant to protect critical sections.
 *
 * The class is defined as a pure abstract class with a factory method
 * (see @ref IMutex::create) to build platform-specific implementations
 * of the queue while maintaining a platform-agnostic interface.
 *
 * The class is 100% thread safe.
 */
class IMutex
{

public:

    /**
     * @brief Factory method to create a mutex implemented for the current
     * platform.
     */
    static IMutex* create( );

    /**
     * @brief Destructor.
     */
    virtual ~IMutex( )
    { }

    /**
     * @brief Locks the mutex.
     */
    virtual void lock( ) = 0;

    /**
     * @brief Unlocks the mutex.
     */
    virtual void unlock( ) = 0;

    /**
     * @brief Return the platform dependent handle of the mutex.
     */
    virtual void* handle( ) = 0;

};

// -----------------------------------------------------------------------------

/**
 * @brief Convenient adapter for class @ref IMutex.
 */
class Mutex
{

public:

    /**
     * @brief Constructor.
     *
     * If no abstract interface is passed it creates a default instance of it
     * by using the method @ref IMutex::create.
     */
    Mutex( IMutex* imutex = nullptr )
        : m_mutex( imutex )
    {
        if ( m_mutex.get() == nullptr )
        {
            m_mutex.reset( IMutex::create( ) );
        }

        assert( nullptr != m_mutex.get( ) );
    }

    /**
     * @copydoc IMutex::lock()
     */
    void
    lock( )
    {
        m_mutex->lock( );
    }

    /**
     * @copydoc IMutex::unlock()
     */
    void
    unlock( )
    {
        m_mutex->unlock( );
    }

    /**
     * @brief Returns the abstract interface used by the adapter.
     */
    IMutex*
    interface( )
    {
        return m_mutex.get( );
    }

private:

    std::shared_ptr< IMutex > m_mutex;

    friend class CondPosix;
    friend class Cond;

};

// -----------------------------------------------------------------------------

template< typename Lockable = Mutex >
class Locker
{

public:

    Locker( Lockable& target )
        : m_target( target )
    {
        m_target.lock( );
    }

    ~Locker( )
    {
        m_target.unlock( );
    }

private:

    Lockable& m_target;

};

// -----------------------------------------------------------------------------

#endif // MUTEX_H
