/*
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

#include "Locker.h"

#include <assert.h>
#include <memory>

#ifndef MUTEX_H
#define MUTEX_H

// ------------------------------------------------------------------------

/**
 * @brief The mutex class is a synchronization primitive that can be used to
 * protect shared data from being simultaneously accessed by multiple threads.
 *
 * @ingroup threading-base
 */
class IMutex
{

public:

    /**
     * @brief Creates one new mutex.
     *
     * A mutual exclusion object (mutex) is an object that can be owned by one
     * single thread at the same time. Threads can use mutexes to performs
     * operation on block of shared variables atomically.
     */
    static IMutex* create( );

    /**
     * @brief Destructor.
     */
    virtual ~IMutex( )
    { }

    /**
     * @brief Locks the mutex.
     *
     * The calling thread try to get the exclusive ownership over the mutex or
     * wait until it manage to
     */
    virtual void lock( ) = 0;

    /**
     * @brief Unlocks the mutex.
     */
    virtual void unlock( ) = 0;

    /**
     * @brief Returns the platform dependent handle assiciated to this object.
     */
    virtual void* handle( ) = 0;

};

// -----------------------------------------------------------------------------

/**
 * @brief Convenient adapter for class @ref IMutex.
 *
 * @see @ref RAII "Resource Acquisition Is Initialization"
 *
 * @code
   Mutex my_mutex;
   ...
   // Exception safe critical section:
   {
        Mutex::Locker( my_mutex );
        ...
   }
   ...
   @endcode
 *
 * @ingroup threading-base raii
 */
class Mutex
{

public:

    /**
     * @brief Convenient typedef for a @ref Locker that locks a @ref Mutex.
     *
     * @ingroup threading-base raii
     */
    typedef Locker< Mutex > Locker;

    /**
     * @brief Default constructor.
     *
     * Builds a mutex calling the method @ref IMutex::create() and hosting the
     * returned abstract interface.
     */
    Mutex( )
        : m_mutex( IMutex::create( ) )
    { }

    /**
     * @brief Creates a mutex adapter from out of an abstract interface.
     *
     * @param imutex Object implementing the abstract interface @ref IMutex.
     *
     * @pre
     * -# Parameter @a imutex is not null.
     */
    Mutex( IMutex* imutex )
        : m_mutex( imutex )
    {
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

#endif // MUTEX_H
