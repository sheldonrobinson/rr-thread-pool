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

#include <Mutex.h>

#include <assert.h>
#include <memory>

#ifndef COND_H
#define COND_H

// Opaque interfaces:
class IMutex;

// Opaque classes:
class Mutex;

// ------------------------------------------------------------------------

/**
 * @brief A condition variable to allow synchronization between threads.
 *
 * Condition variables are synchronization primitives to allow threads to
 * wait until a particular condition occurs.
 *
 * @ingroup threading-base
 */
class ICond
{

public:

    /**
     * @brief Creates one new condition variable.
     */
    static ICond *create();

    /**
     * @brief Destructor.
     */
    virtual ~ICond()
    {
    }

    /**
     * @brief The calling thread will wait until the condition variable is
     * signaled by another thread.
     *
     * This method atomically performs this steps:
     * - unlocks the passed mutex.
     * - wait for a signal from another thread (see methods @ref signal and @ref
     *   broadcast).
     * - locks again the mutex.
     *
     * @param mutex The mutex to be unlocked/locked.
     *
     * @pre
     * -# The passed mutex is currently locked by the calling thread.
     *
     * @post
     * -# The passed mutex is locked back by the calling thread.
     */
    virtual void wait(IMutex *mutex) = 0;

    /**
     * @brief Resumes at least one single thread that is waiting for the
     * condition.
     *
     * Calling the method without no threads waiting for signals have no effect.
     */
    virtual void signal() = 0;

    /**
     * @brief Resumes all threads that are waiting for the condition.
     *
     * Calling the method without no threads waiting for signals have no effect.
     */
    virtual void broadcast() = 0;

    /**
     * @brief Returns the platform dependent handle associated to this object.
     */
    virtual void *handle() = 0;

};

// -----------------------------------------------------------------------------

/**
 * @brief Convenient adapter for class @ref ICond.
 *
 * @note
 * This class is useful to write code that follow the design pattern
 *
 * @see @ref RAII "Resource Acquisition Is Initialization"
 *
 * @ingroup threading-base
 */
class Cond
{

public:

    /**
     * @brief Default constructor.
     *
     * Builds a condition variable by calling the method @ref ICond::create()
     * and then hosts the returned abstract interface to destroy it during the
     * object destruction.
     */
    Cond()
            : m_cond(ICond::create())
    {
    }

    /**
     * @brief Creates a cond adapter from out of an abstract interface.
     *
     * @param icond Object implementing the abstract interface @ref ICond.
     *
     * @pre
     * -# Parameter @a icond is not null.
     */
    Cond(ICond *icond)
            : m_cond(icond)
    {
        assert(nullptr != m_cond.get());
    }

    /**
     * @copydoc ICond::wait
     */
    void wait(Mutex &mutex)
    {
        m_cond->wait(mutex.interface());
    }

    /**
     * @copydoc ICond::signal
     */
    void signal()
    {
        m_cond->signal();
    }

    /**
     * @copydoc ICond::broadcast
     */
    void broadcast()
    {
        m_cond->broadcast();
    }

    /**
     * @brief Returns the abstract interface used by the adapter.
     */
    ICond *interface()
    {
        return m_cond.get();
    }

private:

    std::unique_ptr<ICond> m_cond;

};

// -----------------------------------------------------------------------------

#endif // COND_H
