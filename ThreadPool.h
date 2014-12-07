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

#ifndef TTHREADPOOL_H
#define TTHREADPOOL_H

#include "MessageQueue.h"
#include "Task.h"

#include <cstddef>
#include <limits>
#include <memory>

// ----------------------------------------------------------------------------

class IThreadPool;
typedef std::shared_ptr<IThreadPool> ThreadPool;

/**
 * @brief General purpose thread pool for inter-thread communication.
 *
 * The class is defined as a pure abstract class with a factory method
 * (see @ref IMessageQueue::create) to build platform-specific implementations
 * of the queue while maintaining a platform-agnostic interface.
 *
 * The class is 100% thread safe.
 */
class IThreadPool
{

public:

    /**
     * @brief Factory method to create a thread pool implemented for the current
     * platform.
     *
     * @param num_threads The number of threads the pool should use
     *        concurrently.
     *
     * @param task_capacity Maximum number of tasks that can be queued at the
     *        same time before their execution. By default this limit is
     *        relaxed as much as possible.
     *
     * @return The newly created thread pool.
     */
    static IThreadPool *create(std::size_t num_threads,
                               std::size_t task_capacity
                               = std::numeric_limits<std::size_t>::max());
    /**
     * @brief Destructor.
     */
    virtual ~IThreadPool()
    { }

    /**
     * @brief Pushes one task into the pool.
     *
     * @param task The task to be inserted.
     *
     * @return
     * - On success, the number of tasks pending to be executed after the
     *   insertion, that is at least @a one.
     * - On failure, @a zero. This may happen if the maximum allowed capacity
     *   for pending tasks have been reached.
     *
     * @pre
     * - The parameter task is not null.
     * - The pool have not been cancelled.
     */
    virtual std::size_t push(Task task) = 0;

    /**
     * @brief Pops one executed/cancelled task from the pool.
     *
     * @param[out] task Smart pointer that will be reset with the popped
     *             task in case of success.
     *
     * @param block If set to @a true the method blocks the current thread
     *        indefinitely until a new task have been executed or the pool
     *        have been cancelled.
     *
     * @return
     * - On success, the number of tasks already completed not yet popped
     *   before the extraction, that is at least @a one.
     * - On failure, @a zero (parameter task is not touched in that case).
     *
     * @pre
     * - The pool have not been cancelled.
     */
    virtual std::size_t pop(Task &task, bool blocking) = 0;

    /**
     * @brief Cancel the pool functionality indefinitely releasing any thread.
     *
     * Also cancel any task that have not yet executed. Those task are queued
     * on the list of executed one and can be popped (see method @ref pop).
     *
     * The cancelled status is not reversible and is meant mainly as an action
     * to be performed before the pool destruction.
     *
     * @warning Doesn't wait for the peer threads to be released, just broadcast
     * a signal to them.
     */
    virtual void cancel() = 0;

    /**
     * @brief Cancel and wait for the termination of pool's threads.
     *
     * This method calls the method @ref cancel and than waits indefinitely for
     * the pool's threads.
     *
     * @pre
     * - The behaviour of this method if called during the task execution is
     *   undefined.
     */
    virtual void join() = 0;

    /**
     * @brief Convenient template method to pop executed tasks.
     *
     * Since by design the user should derive messages from the class @ref
     * ITask this template take care to dynamic cast shared pointers to
     * the user defined derived class.
     *
     * This method is meant to be used when the user uses the pool with one
     * single derived class and hence don't need to manually dynamic cast every
     * popped message from the generic @a ITask.
     *
     * @copydetails pop(ITaskPtr& task, bool blocking)
     */
    template<typename Derived>
    std::size_t pop(std::shared_ptr<Derived> &task, bool blocking)
    {
        Task task_abstract;
        std::size_t ret = pop(task_abstract, blocking);
        if (ret > 0)
        {
            assert(task_abstract.get() != nullptr);
            task = std::dynamic_pointer_cast<Derived>(task_abstract);
            assert(task.get() == task_abstract.get());
        }

        return ret;
    }

};

#endif // TTHREADPOOL_H
