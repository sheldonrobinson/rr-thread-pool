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

#include "Cond.h"

// ------------------------------------------------------------------------

#include <pthread.h>

class CondPosix
        : public ICond
{

public:

    CondPosix()
    {
        ::pthread_cond_init(&m_cond, nullptr);
    }

    virtual ~CondPosix()
    {
        ::pthread_cond_destroy(&m_cond);
    }

    void
    wait(IMutex *mutex)
    {
        assert(mutex != nullptr);

        pthread_mutex_t *mutex_handle =
                reinterpret_cast< pthread_mutex_t * >(mutex->handle());

        ::pthread_cond_wait(&m_cond, mutex_handle);
    }

    void
    signal()
    {
        pthread_cond_signal(&m_cond);
    }

    void
    broadcast()
    {
        pthread_cond_broadcast(&m_cond);
    }

    virtual void *
    handle()
    {
        return &m_cond;
    }

private:

    pthread_cond_t m_cond;

};

// -----------------------------------------------------------------------------

ICond *
ICond::create()
{
    return new CondPosix();
}

// -----------------------------------------------------------------------------
