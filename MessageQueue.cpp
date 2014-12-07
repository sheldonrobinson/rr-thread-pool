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
#include "Mutex.h"
#include "Cond.h"

#include <deque>

// -----------------------------------------------------------------------------

class MessageQueueImpl: public IMessageQueue
{
    typedef Locker<Mutex> Locker;

    std::size_t m_max_capacity;
    volatile bool m_cancelled;

    mutable Mutex m_mutex;
    mutable Cond m_cond;
    std::deque<Message> m_queue;

public:

    MessageQueueImpl(std::size_t max_capacity)
            :
            m_max_capacity(max_capacity),
            m_cancelled(false)
    {
    }

    // -------------------------------------------------------------------------

    virtual
    ~MessageQueueImpl()
    {
    }

    // -------------------------------------------------------------------------

    virtual std::size_t
    pop(Message &message, bool blocking)
    {
        std::size_t ret = 0;

        // Blocking implementation:
        if (blocking)
        {
            Locker locker(m_mutex);

            while (!m_cancelled) // <- while needed because of spurious wake-ups.
            {
                ret = m_queue.size();
                if (ret > 0)
                {
                    message = m_queue.front();
                    m_queue.pop_front();
                    break;
                }

                m_cond.wait(m_mutex); // Performs unlock-wait-lock op.
                if (m_cancelled)
                {
                    break;
                }
            }
        }
        else
        {
            Locker locker(m_mutex);

            ret = m_queue.size();
            if (ret > 0)
            {
                message = m_queue.front();
                m_queue.pop_front();
            }
        }

        return ret;
    }

    // -------------------------------------------------------------------------

    virtual std::size_t
    push(Message message)
    {
        std::size_t ret = 0;
        Locker locker(m_mutex);

        ret = m_queue.size();
        if (ret < m_max_capacity)
        {
            m_queue.push_back(message);

            ret++;
            if (ret == 1)
            {
                m_cond.signal();
            }
        }
        else
        {
            ret = 0; // Failure.
        }

        return ret;
    }

    // -------------------------------------------------------------------------

    virtual void
    cancel()
    {
        Locker locker(m_mutex);
        m_cancelled = true;
        m_cond.broadcast();
    }

    // -------------------------------------------------------------------------

    virtual bool
    is_cancelled() const
    {
        return m_cancelled;
    }

    // -------------------------------------------------------------------------

    virtual
    std::size_t
    size() const
    {
        Locker locker(m_mutex);
        return m_queue.size();
    }

};

// -----------------------------------------------------------------------------

IMessageQueue *
IMessageQueue::create(std::size_t max_capacity)
{
    return new MessageQueueImpl(max_capacity);
}

// -----------------------------------------------------------------------------
