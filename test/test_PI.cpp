/**
Copyright (c) 2015, Riccardo Ressi
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

#include "test_Utils.h"

#include "Thread.h"
#include "ThreadPool.h"
#include "Trace.h"

#include <atomic>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <random>

// -----------------------------------------------------------------------------

namespace {


class TestTask
        :
                public ITask
{

    const double m_x;
    const double m_y;
    bool m_run;
    bool m_result;

public:

    TestTask(double x, double y)
            :
            m_x(x),
            m_y(y),
            m_run(false),
            m_result(false)
    {
    }

    virtual
    ~TestTask()
    {
    }

    virtual void
    execute()
    {
        m_result = (m_x * m_x + m_y * m_y <= 1.0);
        m_run = true;
    }

    bool isRun() const { return m_run; }
    bool result() const { return m_result; }

};

} // anonymous namespace

// -----------------------------------------------------------------------------

void
test_PI(int NUM_THREADS)
{
    // const int NUM_THREADS = 4;
    const int NUM_TASKS = 100000;
    const int QUEUE_CAPACITY = NUM_TASKS;

    auto mainThread = IThread::self();
    std::size_t numPositive = 0;

    std::clock_t begin = std::clock();
    {
        std::unique_ptr <IThreadPool> pool(
                IThreadPool::create(NUM_THREADS, QUEUE_CAPACITY));

        std::default_random_engine pick;
        std::uniform_real_distribution<double> zeroToOne(0.0, 1.0);

        // Generates the tasks:
        for (size_t i = 0; i < NUM_TASKS; ++i)
        {
            double x = zeroToOne(pick);
            double y = zeroToOne(pick);

            auto task = std::make_shared<TestTask>(x, y);
            auto res = pool->push(task);
            TEST_CHECK(res != 0);
        }

        // Collects their results:
        for (size_t i = 0; i < NUM_TASKS; ++i)
        {
            std::shared_ptr<TestTask> task;
            auto res = pool->popT(task, true);
            TEST_CHECK(res != 0);

            TEST_CHECK(bool(task));
            TEST_CHECK(task->isRun());
            if (task->result())
            {
                ++numPositive;
            }
        }

        pool->join();
    }
    std::clock_t end = clock();

    {
        std::stringstream message;
        message << "[" << NUM_THREADS << "]";
        trace(message);

        TEST_CHECK(numPositive < NUM_TASKS);
        double pi = 4.0 * double(numPositive) / double(NUM_TASKS);
        message << "PI: " << pi;
        trace(message);

        double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
        message << "Duration: " << elapsed_secs;
        trace(message);
        trace(message);
    }

}

void test_PI()
{
    for (auto i = 1; i <= 16; ++i)
    {
        test_PI(i);
    }
}

// -----------------------------------------------------------------------------
