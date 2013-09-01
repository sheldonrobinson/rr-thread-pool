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

#include <Mutex.h>

#include <assert.h>
#include <memory>

#ifndef COND_H
#define COND_H

// ------------------------------------------------------------------------

/**
 * @brief A classical condition variable meant to signal events.
 *
 * The class is defined as a pure abstract class with a factory method
 * (see @ref ICond::create) to build platform-specific implementations
 * of the queue while maintaining a platform-agnostic interface.
 *
 * The class is 100% thread safe.
 */
class ICond
{

public:

    /**
     * @brief Factory method to create a condition variable implemented for the
     * current platform.
     */
    static ICond* create( );

    /**
     * @brief Destructor.
     */
    virtual ~ICond( )
    { }

    /**
     * @brief Wait for signals.
     *
     * This method atomically performs this steps:
     * - unlock the mutex.
     * - wait for a signal (see methods @ref signal and @ref broadcast).
     * - lock the mutex.
     *
     * @param mutex The mutex to be unlocked/locked.
     *
     * @warning This method blocks the current thread until a second thread
     * sends a signal, be sure that this second thread is going to do it soon
     * or later to avoid deadlocks.
     */
    virtual void wait( IMutex* mutex ) = 0;

    /**
     * @brief Resumes one single thread that is waiting for the condition.
     */
    virtual void signal( ) = 0;

    /**
     * @brief Resumes all threads that are waiting for the condition.
     */
    virtual void broadcast( ) = 0;

    /**
     * @brief Return the platform dependent handle of the condition variable.
     */
    virtual void* handle( ) = 0;

};

// -----------------------------------------------------------------------------

/**
 * @brief Convenient adapter for class @ref ICond.
 */
class Cond
{

public:

    /**
     * @brief Constructor.
     *
     * If no abstract interface is passed it creates a default instance of it
     * by using the method @ref ICond::create.
     */
    Cond( ICond* icond = nullptr )
        : m_cond( icond )
    {
        if ( m_cond.get() == nullptr )
        {
            m_cond.reset( ICond::create( ) );
        }

        assert( nullptr != m_cond.get( ) );
    }

    /**
     * @copydoc ICond::wait
     */
    void
    wait( Mutex& mutex )
    {
        m_cond->wait( mutex.interface( ) );
    }

    /**
     * @copydoc ICond::signal
     */
    void
    signal( )
    {
        m_cond->signal( );
    }

    /**
     * @copydoc ICond::broadcast
     */
    void
    broadcast( )
    {
        m_cond->broadcast( );
    }

    /**
     * @brief Returns the abstract interface used by the adapter.
     */
    ICond*
    interface( )
    {
        return m_cond.get( );
    }

private:

    std::unique_ptr< ICond > m_cond;

};

// -----------------------------------------------------------------------------

#endif // COND_H
