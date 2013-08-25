#ifndef TTHREADPOOL_H
#define TTHREADPOOL_H

#include "MessageQueue.h"

class IThreadPool
{

public:

    class ITask
        : public IMessageQueue::IMessage
    {

    public:
        virtual ~ITask() { }
        virtual void execute( ) = 0;
    };

    typedef std::shared_ptr< ITask > ITaskPtr;

public:

    static IThreadPool* create( std::size_t num_threads,
                                std::size_t task_capacity );
    virtual ~IThreadPool() { }

    virtual std::size_t push( ITaskPtr task ) = 0;
    virtual std::size_t pop( ITaskPtr& task, bool blocking ) = 0;

    virtual void cancel( ) = 0;
    virtual void join( ) = 0;

    template< typename Derived >
    std::size_t pop( std::shared_ptr< Derived > &task, bool blocking )
    {
        ITaskPtr task_tmp;
        std::size_t ret = pop( task_tmp, blocking );
        if ( ret > 0 )
        {
            assert( task_tmp.get() != nullptr );
            task = std::dynamic_pointer_cast< Derived >( task_tmp );
            assert( task.get() != nullptr );
        }

        return ret;
    }

};

#endif // TTHREADPOOL_H
