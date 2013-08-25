
#include "MessageQueue.h"

#include <deque>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

#include <assert.h>

#include <ThreadPosix.h>

// -----------------------------------------------------------------------------

class MessageQueuePosix: public IMessageQueue
{
    typedef std::deque< IMessagePtr > Queue;
    typedef Locker< MutexPosix > Locker;

    std::size_t m_max_capacity;
    volatile bool m_cancelled;

    mutable MutexPosix m_mutex;
    mutable CondPosix m_cond;
    Queue m_queue;

public:

    MessageQueuePosix( std::size_t max_capacity )
        :
        m_max_capacity( max_capacity ),
        m_cancelled( false )
    { }

    // -------------------------------------------------------------------------

    virtual
    ~MessageQueuePosix( )
    { }

    // -------------------------------------------------------------------------

    virtual std::size_t
    pop( IMessagePtr& message, bool blocking )
    {
        std::size_t ret = 0;

        // Non-blocking implementation:
        if ( blocking )
        {
            Locker locker( m_mutex );

            while ( !m_cancelled ) // <- while needed because of spourious wake-ups.
            {
                ret = m_queue.size( );
                if ( ret > 0 )
                {
                    message = m_queue.front( );
                    m_queue.pop_front( );
                    break;
                }

                m_cond.wait( m_mutex );
                if ( m_cancelled )
                {
                    break;
                }
            }
        }
        else
        {
            Locker locker( m_mutex );

            ret = m_queue.size( );
            if ( ret > 0 )
            {
                message = m_queue.front( );
                m_queue.pop_front( );
            }

            return ret;
        }

        return ret;
    }

    // -------------------------------------------------------------------------

    virtual std::size_t
    push( IMessagePtr message )
    {
        std::size_t ret = 0;
        Locker locker( m_mutex );

        ret = m_queue.size( );
        if ( ret < m_max_capacity )
        {
            m_queue.push_back( message );

            ret++;
            if ( ret == 1 )
            {
                m_cond.signal( );
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
    cancel( )
    {
        Locker locker( m_mutex );
        m_cancelled = true;
        m_cond.broadcast( );
    }

    // -------------------------------------------------------------------------

    virtual bool
    is_cancelled( ) const
    {
        return m_cancelled;
    }

    // -------------------------------------------------------------------------

    virtual
    std::size_t
    size( ) const
    {
        Locker locker( m_mutex );
        return m_queue.size( );
    }

};

// -----------------------------------------------------------------------------

IMessageQueue*
IMessageQueue::create( std::size_t max_capacity )
{
    return new MessageQueuePosix( max_capacity );
}

// -----------------------------------------------------------------------------
