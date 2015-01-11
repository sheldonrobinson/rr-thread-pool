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


#include "Thread.h"
#include "test_Utils.h"

#include "Cond.h"
#include "Mutex.h"
#include "Trace.h"

#include <iostream>
#include <vector>

// -----------------------------------------------------------------------------

namespace {

class TestBaseTask
        :
                public ITask
{

    int m_id;
    Mutex &m_mutex;
    int &m_instance_counter;
    int &m_execution_counter;

public:

    TestBaseTask(int id,
                 Mutex &mutex,
                 int &instance_counter,
                 int &execution_counter)
            :
            m_id(id),
            m_mutex(mutex),
            m_instance_counter(instance_counter),
            m_execution_counter(execution_counter)
    {
        trace(m_id, "created");

        Locker<Mutex> lock(m_mutex);
        ++m_instance_counter;
    }

    virtual void
    execute()
    {
        trace(m_id, "executed");

        Locker<Mutex> lock(m_mutex);
        ++m_execution_counter;
    }

    virtual
    ~TestBaseTask()
    {
        trace(m_id, "destroyed");

        Locker<Mutex> lock(m_mutex);
        --m_instance_counter;
    }

};

// -----------------------------------------------------------------------------

void
test_base()
{
    const int NUM_THREADS = 100;

    Mutex mutex;
    int instance_counter = 0;
    int execution_counter = 0;

    {
        std::vector<Thread> threads;
        threads.reserve(NUM_THREADS);
        for (int i = 0; i < NUM_THREADS; ++i)
        {
            Task new_task = std::make_shared<TestBaseTask>(
                    i + 1, mutex, instance_counter, execution_counter);
            TEST_CHECK(instance_counter >= 1);

            Thread new_thread(IThread::create(new_task));
            threads.push_back(new_thread);
        }

        for (auto &thread: threads)
        {
            thread->join();
        }

        TEST_CHECK(NUM_THREADS == execution_counter);
    }

    TEST_CHECK(0 == instance_counter);
}

// -----------------------------------------------------------------------------

class TestJoinTask
        :
                public ITask
{

    int m_id;
    Mutex &m_mutex;
    Cond &m_cond_wait;
    Cond *m_cond_signal;
    int &m_instance_counter;
    int &m_execution_counter;

public:

    TestJoinTask(int id,
                 Mutex &mutex,
                 Cond &cond_wait,
                 Cond *cond_signal,
                 int &instance_counter,
                 int &execution_counter)
            :
            m_id(id),
            m_mutex(mutex),
            m_cond_wait(cond_wait),
            m_cond_signal(cond_signal),
            m_instance_counter(instance_counter),
            m_execution_counter(execution_counter)
    {
        trace(m_id, "created");

        Locker<Mutex> lock(m_mutex);
        ++m_instance_counter;
    }

    virtual void
    execute()
    {
        trace(m_id, "executing");

        Thread self = IThread::self();

        if (m_cond_signal)
        {
            self->yield();
            self->yield();
            self->yield();
            self->yield();
            self->yield();
            self->yield();
            self->yield();
            self->yield();
            self->yield();
            m_cond_signal->signal();
        }

        {
            Locker<Mutex> lock(m_mutex);
            m_cond_wait.wait(m_mutex);

            ++m_execution_counter;
        }

        self->yield();

        trace(m_id, "executing");
    }

    virtual
    ~TestJoinTask()
    {
        trace(m_id, "destroyed");

        Locker<Mutex> lock(m_mutex);
        --m_instance_counter;
    }

};

// -----------------------------------------------------------------------------

void
test_join()
{
    const int NUM_THREADS = 100;

    Mutex mutex;
    Cond cond_task, cond_init;
    int instance_counter = 0;
    int execution_counter = 0;

    {
        std::vector<Thread> threads;
        threads.reserve(NUM_THREADS);
        for (int i = 0; i < NUM_THREADS; ++i)
        {
            Cond *cond_signal = 0;
            if (i + 1 == NUM_THREADS)
            {
                cond_signal = &cond_init;
            }

            Task new_task = std::make_shared<TestJoinTask>(
                    i + 1,
                    mutex,
                    cond_task,
                    cond_signal,
                    instance_counter,
                    execution_counter);
            TEST_CHECK(instance_counter >= 1);

            Thread new_thread(IThread::create(new_task));
            threads.push_back(new_thread);
        }
        TEST_CHECK(0 == execution_counter);
        TEST_CHECK(NUM_THREADS == instance_counter);

        {
            Locker<Mutex> locker(mutex);
            cond_init.wait(mutex);
            cond_task.broadcast();
        }

        for (int i = 0; i < NUM_THREADS; ++i)
        {
            trace(i + 1, "joining it");
            threads[i]->join();
            trace(i + 1, "joined");
        }
        trace("Done.");
        TEST_CHECK(NUM_THREADS == execution_counter);
    }

    TEST_CHECK(0 == instance_counter);
}

} // anonymous namespace

// -----------------------------------------------------------------------------

void
test_Thread()
{
    test_base();
    test_join();
}

// -----------------------------------------------------------------------------
