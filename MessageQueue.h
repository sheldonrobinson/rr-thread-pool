#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <limits>
#include <memory>

#include <assert.h>

// ----------------------------------------------------------------------------

/**
 * @brief General pourpose message queue for inter-thread communication.
 *
 * This class uses run-time polimorfism to allow message-driven communication
 * and syncronization between two or more threads.
 *
 * The class is defined as a pure abstract class with a factory method
 * (see @ref IMessageQueue::create) to build platform-specific implementations
 * of the queue while maintaining a platform-agnostic interface.
 */
class IMessageQueue
{

public:

    /**
     * @brief Abstract class meant to be derived by every queued message.
     */
    class IMessage
    {

    public:

        /**
         * @brief Destructor.
         */
        virtual ~IMessage( ) { }
    };

    /**
     * @brief IMessagePtr Shared pointer for messages.
     *
     * Message's ownership is shared between the user and the queue, for this
     * reason methods to push/pop messages requires this typedef.
     */
    typedef std::shared_ptr< IMessage > IMessagePtr;

    /**
     * @brief Factory method to create a message queue implemented for the
     * current platform.
     *
     * @param max_capacity Maximum number of messages that can be queued at
     *        the same time. By default this limit is relaxed as much as
     *        possible.
     *
     * @return The newly created message queue.
     */
    static IMessageQueue* create( std::size_t max_capacity
                                  = std::numeric_limits< std::size_t >::max( ) );

    /**
     * @brief Destructor.
     */
    virtual ~IMessageQueue( )
    { }

    /**
     * @brief Pops one message from the queue.
     *
     * @param[out] message Smart pointer that will be reset with the popped
     *             message in case of success.
     *
     * @param block If set to @a true the method blocks the current thread
     *        indefinitely until a new message is pushed into the queue
     *        by another thread or until the queue is not cancelled.
     *
     * @return
     * - On failure, @a zero (parameter message is not touched in that case).
     * - On success, the number of messages contained by the queue before the
     *   extraction that is at least @a one.
     *
     * @pre
     * - The queue have not been cancelled.
     */
    virtual std::size_t pop( IMessagePtr& message, bool block ) = 0;

    /**
     * @brief Pushes one message into the queue.
     *
     * @param message The message to be inserted.
     *
     * @return
     * - On failure, @a zero. This may happen if the maximum allowed capacity
     *   for the queue have been reached.
     * - On success, the number of messages contained by the queue after the
     *   insertion that is at least @a one.
     *
     * @pre
     * - The parameter message is not null.
     * - The queue have not been cancelled.
     */
    virtual std::size_t push( IMessagePtr message ) = 0;

    /**
     * @brief Cancel the queue functionality indefinitely releasing any blocked
     * thread.
     *
     * The cancelled status is not reversible and is meant mainly as an action to
     * be performed before the queue destruction.
     *
     * @warning Doesn't wait for the peer threads to be released, just broadcast
     * a signal to them.
     */
    virtual void cancel( ) = 0;

    /**
     * @brief Returns @a true if the queue have been cancelled.
     */
    virtual bool is_cancelled( ) const = 0;

    /**
     * @brief Returns the number of messages contained inside the queue.
     */
    virtual std::size_t size( ) const = 0;

    /**
     * @brief Convenient template method to pop messages.
     *
     * Since by design the user should derive messages from the class @ref
     * IMessage this template take care to dynamic cast shared pointers to
     * the user defined derived class.
     *
     * This method is meant to be used when the user uses the queue with one
     * single derived class and hence don't need to manually dynamic cast every
     * popped message from the generic @a IMessage.
     */
    template< typename Derived >
    std::size_t
    pop( std::shared_ptr< Derived > &message, bool blocking )
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

/**
 * @brief General pourpose message queue for inter-thread communication.
 *
 * This template class uses compile-time polimorfism to allow message-driven
 * communication and syncronization between two or more threads.
 *
 * The implementaion of this template is based on @ref IMessageQueue class.
 */
template< typename M >
class MessageQueue
{

public:

    /**
     * @brief Constructor.
     *
     * @param max_capacity Maximum number of messages that can be queued at
     *        the same time. By default this limit is relaxed as much as
     *        possible.
     */
    explicit inline MessageQueue( std::size_t max_capacity
                                  = std::numeric_limits< std::size_t >::max( ) );

    /**
     * @brief Pops one message from the queue.
     *
     * @param[out] message A reference to a message object meant to be set
     *             with the extracted message only in case of success.
     *
     * @param block If set to @a true the method blocks the current thread
     *        indefinitely until a new message is pushed into the queue
     *        by another thread or until the queue is not cancelled.
     *
     * @return
     * - On failure, @a zero (parameter message is not touched in that case).
     * - On success, the number of messages contained by the queue before the
     *   extraction that is at least @a one.
     *
     * @pre
     * - The queue have not been cancelled.
     */
    inline std::size_t pop( M& dst_message, bool block );

    /**
     * @brief Pushes one message into the queue.
     *
     * @param message The message to be inserted.
     *
     * @return
     * - On failure, @a zero. This may happen if the maximum allowed capacity
     *   for the queue have been reached.
     * - On success, the number of messages contained by the queue after the
     *   insertion that is at least @a one.
     *
     * @pre
     * - The queue have not been cancelled.
     */
    inline std::size_t push( const M& message );

    /**
     * @copydoc IMessageQueue::cancel()
     */
    inline void cancel( );

    /**
     * @copydoc IMessageQueue::is_cancelled()
     */
    inline bool is_cancelled( ) const;

    /**
     * @copydoc IMessageQueue::size()
     */
    inline std::size_t size( ) const;

private:

    std::unique_ptr< IMessageQueue > m_impl;

    template < typename P >
    class MessageImpl
        :
        public IMessageQueue::IMessage
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
