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

#include "Task.h"

#include <assert.h>
#include <memory>

#ifndef THREAD_H
#define THREAD_H

// ------------------------------------------------------------------------

class IThread;
typedef std::shared_ptr< IThread > Thread;

/**
 * @brief A class to create and control threads.
 *
 * The class is defined as a pure abstract class with a factory method
 * (see @ref ICond::create) to build platform-specific implementations
 * of the queue while maintaining a platform-agnostic interface.
 *
 * The class is 100% thread safe.
 */
class IThread
{

public:

    /**
     * @brief Factory method to create a thread implemented for the
     * current platform.
     */
    static Thread create( Task task );

    /**
     * @brief Returns an handle to control the current thread.
     */
    static Thread self( );

   /**
     * @brief Destructor.
     *
     * @warning If the thread is still running waits for its termination (see
     * method @ref join).
     */
    virtual ~IThread( )
    { }

    /**
     * @brief Checks if the thread is still running.
     */
    virtual bool is_running( ) const = 0;

    /**
     * @brief Cancel and wait for thread termination.
     *
     * @warning This method blocks the caller thread until the thread controlled
     * by the current object is not finished, be sure that this is going to
     * happen to avoid deadlocks.
     */
    virtual void join( ) = 0;

    /**
     * @brief Passes the execution of the caller thread to another thread.
     *
     * @note This method is affecting the caller thread instead of the thread
     * controlled by the object. Have not been declared static in order to
     * use the same platform implementation of the current instance.
     */
    virtual void yield( ) const = 0;

    /**
     * @brief Returns the platform specific handle.
     */
    virtual void* handle( ) = 0;

};

// -----------------------------------------------------------------------------

#endif // THREAD_H
