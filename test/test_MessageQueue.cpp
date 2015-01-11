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
#include "Thread.h"
#include "Trace.h"

#include <deque>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

// ------------------------------------------------------------------------....

namespace
{

class TestQueueTask
    : public ITask
{

    int m_id;

    MessageQueueT<std::string> &m_in_queue;
    MessageQueueT<std::string> &m_out_queue;

public:

    TestQueueTask(int id,
                  MessageQueueT<std::string> &in_queue,
                  MessageQueueT<std::string> &out_queue)
            :
            m_id(id),
            m_in_queue(in_queue),
            m_out_queue(out_queue)
    {
    }

    virtual ~TestQueueTask()
    {
    }

    void
    execute()
    {
        trace("Running");

        std::string message;
        while (m_in_queue.pop(message, true))
        {
            trace(message);

            std::stringstream response;
            response << "Response to '" << message << " from '" << m_id << "'";

            while (0 == m_out_queue.push(response.str()))
            {
                trace("Waiting for a free slot into the output queue");
                sched_yield();
            }
        }

        trace("Done.");
    }

};

} // anonymous namespace

// ----------------------------------------------------------------------------

void
test_MessageQueue()
{
    const int NUM_THREADS = 100;
    const int NUM_MESSAGES = 100000;
    const int QUEUE_CAPACITY = 100;

    MessageQueueT<std::string> queue_in(QUEUE_CAPACITY);
    MessageQueueT<std::string> queue_out(QUEUE_CAPACITY);

    {
        std::vector<Thread> threads;

        threads.reserve(NUM_THREADS);
        for (int i = 0; i < NUM_THREADS; ++i)
        {
            Task worker(new TestQueueTask(i + 1,
                                          queue_in,
                                          queue_out));

            Thread new_thread(IThread::create(worker));
            threads.push_back(new_thread);
        }

        int num_messages_in = NUM_MESSAGES;
        int num_messages_out = NUM_MESSAGES;

        while (num_messages_in > 0
                || num_messages_out > 0)
        {

            if (num_messages_in > 0)
            {
                std::stringstream stream;
                stream << "Message " << NUM_MESSAGES - num_messages_in;

                std::size_t num = queue_in.push(stream.str());
                if (num > 0)
                {
                    --num_messages_in;
                    if (num < QUEUE_CAPACITY / 2)
                    {
                        continue;
                    }
                }
                else
                {
                    sched_yield();
                }
            }

            if (num_messages_out > 0)
            {
                std::string message;
                std::size_t num = queue_out.pop(message, num_messages_in > 0);
                if (num > 0)
                {
                    std::stringstream str;
                    str << 0 << ": " << message
                            << " #" << queue_in.size()
                            << ":" << queue_out.size() << std::endl;
                    trace(str.str());
                    --num_messages_out;
                }
                else
                {
                    sched_yield();
                }
            }
        }

        queue_in.cancel();

        for (auto &thread: threads)
        {
            thread->join();
            thread.reset();
        }
    }
}

// ----------------------------------------------------------------------------
