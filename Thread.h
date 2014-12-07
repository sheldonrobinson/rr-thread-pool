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

#include "Task.h"

#include <assert.h>
#include <memory>

#ifndef THREAD_H
#define THREAD_H

// ------------------------------------------------------------------------

class IThread;

/**
 * @brief Shared pointer for abstract interface @ref IThread.
 *
 * @see @ref RAII "Resource Acquisition Is Initialization"
 *
 * @ingroup threading-base raii
 */
typedef std::shared_ptr< IThread > Thread;

/**
 * @brief The class thread represents a single thread of execution.
 *
 * Threads allow multiple pieces of code to run asynchronously and
 * simultaneously.
 *
 * @code
   class MyTask:
        public ITask
   {
       void execute( ) { ... }
   }

   int main( )
   {
       Task task = std::make_shared< MyTask >( );
       Thread my_thread = IThread::create( task );
       ...
       my_thread->join( );
   }
   @endcode
 *
 * @ingroup threading-base
 */
class IThread
{

public:

    /**
     * @brief Creates one new thread to execute one single task.
     *
     * @param task Shared pinter to a @ref ITask to be executed by the thread.
     * The created thread also takes ownership of the task during its
     * execution.
     *
     * @return A shared pointer to an object to check and control the created
     * thread. The created thread is also taking ownership to the returned
     * object.
     */
    static Thread create( Task task );

    /**
     * @brief Returns an handle to control the current thread.
     */
    static Thread self( );

   /**
     * @brief Joins and destroys the object.
     *
     * Before destroying the object calls the method @ref join to ensure the
     * handled thread is terminated.
     *
     * @pre
     * -# The object cannot be destroyed by its own thread.
     */
    virtual ~IThread( )
    { }

    /**
     * @brief Checks if the thread is still running.
     */
    virtual bool is_running( ) const = 0;

    /**
     * @brief Cancels and waits for thread termination.
     *
     * @pre
     * -# This method cannot be from the whole thread.
     */
    virtual void join( ) = 0;

    /**
     * @brief if supported by the platform implementation, passes the execution
     * of the thread to another thread.
     *
     * @pre
     * -# This method can be called only from the whole thread.
     */
    virtual void yield( ) const = 0;

    /**
     * @brief Returns the platform dependent handle associated to this object.
     */
    virtual void* handle( ) = 0;

};

// -----------------------------------------------------------------------------

#endif // THREAD_H
