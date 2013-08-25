#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <limits>
#include <memory>

#include <assert.h>

// ----------------------------------------------------------------------------

class IMessageQueue
{

public:

    class IMessage
    {
    public:
        virtual ~IMessage( ) { }
    };

    typedef std::shared_ptr< IMessage > IMessagePtr;

    static IMessageQueue* create( std::size_t max_capacity
                                  = std::numeric_limits< std::size_t >::max( ) );
    virtual ~IMessageQueue( ) { }

    virtual std::size_t pop( IMessagePtr& message, bool block ) = 0;
    virtual std::size_t push( IMessagePtr message ) = 0;
    virtual void cancel( ) = 0;

    virtual bool is_cancelled( ) const = 0;
    virtual std::size_t size( ) const = 0;

    template< typename Derived >
    std::size_t pop( std::shared_ptr< Derived > &message, bool blocking )
    {
        IMessagePtr message_tmp;
        std::size_t ret = pop( message_tmp, blocking );
        if ( ret > 0 )
        {
            assert( message_tmp.get() != nullptr );
            message = std::dynamic_pointer_cast< Derived >( message_tmp );
            assert( message.get() != nullptr );
        }

        return ret;
    }

};


// ----------------------------------------------------------------------------

template< typename M >
class MessageQueue
{

public:

    explicit inline MessageQueue( std::size_t max_capacity
                                  = std::numeric_limits< std::size_t >::max( ) );

    inline std::size_t pop( M& dst_message, bool block );
    inline std::size_t push( const M& message );
    inline void cancel( );

    inline bool is_cancelled( ) const;
    inline std::size_t size( ) const;

private:

    std::unique_ptr< IMessageQueue > m_impl;

    template < typename P >
    class MessageImpl
        : public IMessageQueue::IMessage
    {

    public:

        P m_payload;

        MessageImpl( const P& payload )
            : m_payload( payload )
        { }

        virtual ~MessageImpl( )
        { }
    };

};

// ----------------------------------------------------------------------------

template< typename M >
MessageQueue< M >::MessageQueue( std::size_t max_capacity )
{
    m_impl.reset( IMessageQueue::create( max_capacity ) );
}

// ----------------------------------------------------------------------------

template< typename M >
std::size_t
MessageQueue< M >::pop( M& dst_message, bool blocking )
{
    IMessageQueue::IMessagePtr new_message;
    std::size_t ret = m_impl->pop( new_message, blocking );

    if ( ret > 0 )
    {
        assert( nullptr != new_message.get( ) );

        MessageImpl< M >* tmp = dynamic_cast< MessageImpl< M >* >( new_message.get( ) );
        assert( tmp != nullptr );

        dst_message = tmp->m_payload;
    }

    return ret;
}

// ----------------------------------------------------------------------------

template< typename M >
std::size_t
MessageQueue< M >::push( const M& message )
{
    IMessageQueue::IMessagePtr new_message( new MessageImpl< M >( message ) );

    return m_impl->push( new_message );
}

// ----------------------------------------------------------------------------

template< typename M >
void
MessageQueue< M >::cancel( )
{
    m_impl->cancel( );
}

// ----------------------------------------------------------------------------

template< typename M >
bool
MessageQueue< M >::is_cancelled( ) const
{
    return m_impl->is_cancelled( );
}

// ----------------------------------------------------------------------------

template< typename M >
std::size_t
MessageQueue< M >::size( ) const
{
    return m_impl->size( );
}

// ----------------------------------------------------------------------------

#endif // MESSAGEQUEUE_H
