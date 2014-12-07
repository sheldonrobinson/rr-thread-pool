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

#include "ThreadPool.h"

#include "MessageQueue.h"
#include "Thread.h"

#include <iostream>
#include <string>
#include <vector>

// -----------------------------------------------------------------------------

class ThreadPoolWorker
        :
                public ITask
{

    IMessageQueue &m_input_queue;
    IMessageQueue &m_output_queue;

public:

    ThreadPoolWorker(IMessageQueue &input_queue,
                     IMessageQueue &output_queue)
            : m_input_queue(input_queue),
              m_output_queue(output_queue)
    {
    }

    virtual
    ~ThreadPoolWorker()
    {
    }

    virtual void
    execute()
    {
        // For each fetched message:
        Task task;
        while (m_input_queue.pop(task, true))
        {
            task->execute();
            m_output_queue.push(task);
        }

        assert(m_input_queue.is_cancelled());
    }

};

// -----------------------------------------------------------------------------

class ThreadPoolPosix
        :
                public IThreadPool
{

    std::vector<Thread> m_threads;
    std::unique_ptr<IMessageQueue> m_input_queue;
    std::unique_ptr<IMessageQueue> m_output_queue;
    volatile bool m_cancelled;

public:

    ThreadPoolPosix(std::size_t num_threads,
                    std::size_t task_capacity)
            :
            m_cancelled(false)
    {
        // Creates the message queues (in/out) for the tasks:
        m_input_queue.reset(IMessageQueue::create(task_capacity));
        m_output_queue.reset(IMessageQueue::create());

        // Creates the threads:
        m_threads.reserve(num_threads);
        for (std::size_t i = 0; i < num_threads; ++i)
        {
            Task worker(new ThreadPoolWorker(*m_input_queue,
                                             *m_output_queue));

            Thread thread_worker(IThread::create(worker));
            m_threads.push_back(thread_worker);
        }
    }

    virtual
    ~ThreadPoolPosix()
    {
        join( );
    }

    virtual std::size_t
    push(Task task)
    {
        // Precondition verification:
        assert(nullptr != task.get());
        assert(!m_cancelled);

        // Tries to push the task in the form of message to the input queue:
        return m_input_queue->push(task);
    }

    virtual std::size_t
    pop(Task &task, bool blocking)
    {
        // Precondition verification:
        assert(!m_cancelled);

        // Fetches the next executed task in the form of message:
        return m_output_queue->pop(task, blocking);
    }

    virtual void
    cancel()
    {
        m_input_queue->cancel();
        m_cancelled = true;
    }

    virtual void
    join()
    {
        // Cancel the input queue in order to terminate all workers:
        cancel();

        // Joins all workers threads:
        for (auto &thread: m_threads)
        {
            thread->join();
        }

        // Transfers all pending tasks from the input queue to the output one:
        Task task;
        while (m_input_queue->pop(task, false) > 0)
        {
            m_output_queue->push(task);
        }
    }

};

// -----------------------------------------------------------------------------

IThreadPool *
IThreadPool::create(std::size_t num_threads,
                    std::size_t task_capacity)
{
    return new ThreadPoolPosix(num_threads, task_capacity);
}

// -----------------------------------------------------------------------------
