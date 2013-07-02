/*! \file
* \brief Thread pool core.
*
* This file contains the threadpool's core class: pool<Task, SchedulingPolicy>.
*
* Thread pools are a mechanism for asynchronous and parallel processing 
* within the same process. The pool class provides a convenient way 
* for dispatching asynchronous tasks as functions objects. The scheduling
* of these tasks can be easily controlled by using customized schedulers. 
*
* Copyright (c) 2005-2006 Philipp Henkel
*
* Use, modification, and distribution are  subject to the
* Boost Software License, Version 1.0. (See accompanying  file
* LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
*
* http://threadpool.sourceforge.net
*
*/


#ifndef THREADPOOL_POOL_CORE_HPP_INCLUDED
#define THREADPOOL_POOL_CORE_HPP_INCLUDED


#include "locking_ptr.hpp"
#include "worker_thread.hpp"

//#include "scheduling_policies.hpp"
#include "../task_adaptors.hpp"

#include <boost/thread.hpp>
#include <boost/thread/exceptions.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits.hpp>


/// The namespace threadpool contains a thread pool and related utility classes.
namespace boost { namespace threadpool { namespace detail 
{

  /*! \brief Thread pool. 
  *
  * Thread pools are a mechanism for asynchronous and parallel processing 
  * within the same process. The pool class provides a convenient way 
  * for dispatching asynchronous tasks as functions objects. The scheduling
  * of these tasks can be easily controlled by using customized schedulers. 
  * A task must not throw an exception.
  *
  * A pool_impl is DefaultConstructible and NonCopyable.
  *
  * \param Task A function object which implements the operator 'void operator() (void) const'. The operator () is called by the pool to execute the task. Exceptions are ignored.
  * \param Scheduler A task container which determines how tasks are scheduled. It is guaranteed that this container is accessed only by one thread at a time. The scheduler shall not throw exceptions.
  *
  * \remarks The pool class is thread-safe.
  * 
  * \see Tasks: task_func, prio_task_func
  * \see Scheduling policies: fifo_scheduler, lifo_scheduler, prio_scheduler
  */ 
  template <
    typename Task, 

    template <typename> class SchedulingPolicy,
    template <typename> class SizePolicy,
    template <typename> class SizePolicyController,
    template <typename> class ShutdownPolicy
  > 
  class pool_core
  : public enable_shared_from_this< pool_core<Task, SchedulingPolicy, SizePolicy, SizePolicyController, ShutdownPolicy > > 
  , private noncopyable
  {

  public: // Type definitions
    typedef Task task_type;                                 //!< Indicates the task's type.
    typedef SchedulingPolicy<task_type> scheduler_type;     //!< Indicates the scheduler's type.
    typedef pool_core<Task, 
                      SchedulingPolicy, 
                      SizePolicy,
                      SizePolicyController,
                      ShutdownPolicy > pool_type;           //!< Indicates the thread pool's type.
    typedef SizePolicy<pool_type> size_policy_type;         //!< Indicates the sizer's type.
    //typedef typename size_policy_type::size_controller size_controller_type;

    typedef SizePolicyController<pool_type> size_controller_type;

//    typedef SizePolicy<pool_type>::size_controller size_controller_type;
    typedef ShutdownPolicy<pool_type> shutdown_policy_type;//!< Indicates the shutdown policy's type.  

    // The task is required to be a nullary function.
    BOOST_STATIC_ASSERT(function_traits<task_type()>::arity == 0);

    // The task function's result type is required to be void.
    BOOST_STATIC_ASSERT(is_void<typename result_of<task_type()>::type >::value);


  private:  // Friends 
    friend class detail::worker_thread<pool_type>;
    friend class SizePolicy<pool_type>;
    friend class ShutdownPolicy<pool_type>;

  private: // The following members may be accessed by _multiple_ threads at the same time:
    volatile size_t m_worker_count;	
    volatile size_t m_target_worker_count;	
    volatile size_t m_active_worker_count;
    volatile bool   m_allow_resize;

  private: // The following members are accessed only by _one_ thread at the same time:
    scheduler_type  m_scheduler;
    scoped_ptr<size_policy_type> m_size_policy; // todo better name m_size_policy, is never null
    
  private: // The following members are implemented thread-safe:
    mutable mutex     m_monitor;
    mutable condition m_decrease_active_worker_event;	
    mutable condition m_task_or_decrease_worker_event;

  public:
    /// Constructor.
    pool_core()
      : m_worker_count(0) 
      , m_target_worker_count(0)
      , m_active_worker_count(0)
      , m_allow_resize(true)
    {
      pool_type volatile & self_ref = *this;
      m_size_policy.reset(new size_policy_type(self_ref));

      m_scheduler.clear();
    }


    /// Destructor.
    virtual ~pool_core()
    {
    }

    /*! Gets the size controller which manages the number of threads in the pool. 
    * \return The size controller.
    * \see SizePolicy
    */
    size_controller_type size_controller()
    {
      return size_controller_type(*m_size_policy, enable_shared_from_this<pool_core>::shared_from_this());
    }

    /*! Gets the number of threads in the pool.
    * \return The number of threads.
    */
    size_t size()	const volatile
    {
      return m_worker_count;
    }

// TODO is only called once
    void shutdown()
    {
      ShutdownPolicy<pool_type>::shutdown(*this);
    }

    /*! Schedules a task for asynchronous execution. The task will be executed once only.
    * \param task The task function object. It should not throw execeptions.
    * \return true, if the task could be scheduled and false otherwise. 
    */  
    bool schedule(task_type const & task) volatile
    {	
      locking_ptr<pool_type> lockedThis(*this, m_monitor); 
      
      if(lockedThis->m_scheduler.push(task))
      {
        lockedThis->m_task_or_decrease_worker_event.notify_one();
        return true;
      }
      else
      {
        return false;
      }
    }	


    /*! Returns the number of tasks which are currently executed.
    * \return The number of active tasks. 
    */  
    size_t active() const volatile
    {
      return m_active_worker_count;
    }


    /*! Returns the number of tasks which are ready for execution.    
    * \return The number of pending tasks. 
    */  
    size_t pending() const volatile
    {
      locking_ptr<const pool_type> lockedThis(*this, m_monitor);
      return lockedThis->m_scheduler.size();
    }


    /*! Removes all pending tasks from the pool's scheduler.
    */  
    void clear() volatile
    { 
      locking_ptr<pool_type> lockedThis(*this, m_monitor);
      lockedThis->m_scheduler.clear();
    }    


    /*! Indicates that there are no tasks pending. 
    * \return true if there are no tasks ready for execution.	
    * \remarks This function is more efficient that the check 'pending() == 0'.
    */   
    bool empty() const volatile
    {
      locking_ptr<const pool_type> lockedThis(*this, m_monitor);
      return lockedThis->m_scheduler.empty();
    }	


    /*! The current thread of execution is blocked until the sum of all active
    *  and pending tasks is equal or less than a given threshold. 
    * \param task_threshold The maximum number of tasks in pool and scheduler.
    */     
    void wait(size_t const task_threshold = 0) const volatile
    {
      const pool_type* self = const_cast<const pool_type*>(this);
      mutex::scoped_lock lock(self->m_monitor);

      if(0 == task_threshold)
      {
        while(0 != self->m_active_worker_count || !self->m_scheduler.empty())
        { 
          self->m_decrease_active_worker_event.wait(lock);
        }
      }
      else
      {
        while(task_threshold < self->m_active_worker_count + self->m_scheduler.size())
        { 
          self->m_decrease_active_worker_event.wait(lock);
        }
      }
    }	

    /*! The current thread of execution is blocked until the timestamp is met
    * or the sum of all active and pending tasks is equal or less 
    * than a given threshold.  
    * \param timestamp The time when function returns at the latest.
    * \param task_threshold The maximum number of tasks in pool and scheduler.
    * \return true if the task sum is equal or less than the threshold, false otherwise.
    */       
    bool wait(xtime const & timestamp, size_t const task_threshold = 0) const volatile
    {
      const pool_type* self = const_cast<const pool_type*>(this);
      mutex::scoped_lock lock(self->m_monitor);

      if(0 == task_threshold)
      {
        while(0 != self->m_active_worker_count || !self->m_scheduler.empty())
        { 
          if(!self->m_decrease_active_worker_event.timed_wait(lock, timestamp)) return false;
        }
      }
      else
      {
        while(task_threshold < self->m_active_worker_count + self->m_scheduler.size())
        { 
          if(!self->m_decrease_active_worker_event.timed_wait(lock, timestamp)) return false;
        }
      }

      return true;
    }


  private:	

    /*! Changes the number of worker threads in the pool. The resizing 
    *  is handled by the SizePolicy.
    * \param threads The new number of worker threads.
    * \return true, if pool will be resized and false if not. 
    */
    virtual bool resize(size_t const worker_count) volatile
    {
      locking_ptr<pool_type> lockedThis(*this, m_monitor); 


      if(m_allow_resize)
      {
        m_target_worker_count = worker_count;
      }
      else
      { // allow shutdown
        if(0 == worker_count)
        {
          m_target_worker_count = 0;
        }
        else
        {
          return false;
        }
      }



      if(m_worker_count <= m_target_worker_count)
      { // increase worker count
        while(m_worker_count < m_target_worker_count)
        {
          try
          {
            worker_thread<pool_type>::create_and_attach(lockedThis->shared_from_this());
            m_worker_count++;
            m_active_worker_count++;	
          }
          catch(thread_resource_error)
          {
            return false;
          }
        }
      }
      else
      { // decrease worker count
        lockedThis->m_task_or_decrease_worker_event.notify_all();
      }

      return true;
    }


    /*! Changes the number of worker threads in the pool.
    * \param threads The new number of worker threads.
    * \return true, if pool was resized and false if not. 
    */
  /*  bool force_resize(size_t const worker_count)
    {
        bool result = true;

        m_target_worker_count = worker_count;

        if(m_worker_count <= m_target_worker_count)
        { // increase worker count
          while(m_worker_count < m_target_worker_count)
          {
            try
            {
              worker_thread<pool_type>::create_and_attach(shared_from_this());
              m_worker_count++;
              m_active_worker_count++;	
            }
            catch(thread_resource_error)
            {
              result = false;
              break;
            }
          }
        }
        else
        { // decrease worker count
          //        lockedThis->m_decrease_active_worker_event.notify_all();
          m_task_or_decrease_worker_event.notify_all();
        }

        return result;
    }
*/


    // worker handling
    bool terminate_worker()
    {
      if(m_worker_count > m_target_worker_count)
      {	
        m_worker_count--;
        m_active_worker_count--;
        m_decrease_active_worker_event.notify_all();		
        return true;	// terminate worker
      }
      else
      {
        return false;
      }
    }

    // worker died with unhandled exception
    void worker_died() volatile
    {
      locking_ptr<pool_type> lockedThis(*this, m_monitor);
      m_worker_count--;
      m_active_worker_count--;
      lockedThis->m_decrease_active_worker_event.notify_all();		
      lockedThis->m_size_policy->worker_died_unexpectedly(m_worker_count);
    }


    class exception_guard
    {
      bool m_enabled;
      volatile pool_type* m_pool;

    public:
      exception_guard(volatile pool_type& pool)
      : m_enabled(true)
      , m_pool(&pool)
      {
      }

      ~exception_guard()
      {
        if(m_enabled)
        {
          assert(m_pool);
          m_pool->worker_died();
        }
      }

      void disable()
      {
        m_enabled = false;
      }

    };

    bool execute_task() volatile
    {
      std::auto_ptr<exception_guard> guard(new exception_guard(*this));

      function0<void> task;

      { // fetch task
        pool_type* lockedThis = const_cast<pool_type*>(this);
        mutex::scoped_lock lock(lockedThis->m_monitor);

        // decrease number of threads if necessary
        if(lockedThis->terminate_worker())
        {	
          guard->disable();
          return false;	// terminate worker
        }


        // wait for tasks
        while(lockedThis->m_scheduler.empty())
        {	
          // decrease number of workers if necessary
          if(lockedThis->terminate_worker())
          {	
            guard->disable();
            return false;	// terminate worker
          }
          else
          {
            m_active_worker_count--;
            lockedThis->m_decrease_active_worker_event.notify_all();	
            lockedThis->m_task_or_decrease_worker_event.wait(lock);
            m_active_worker_count++;
          }
        }

        task = lockedThis->m_scheduler.top();
        lockedThis->m_scheduler.pop();
      }

      // call task function
      if(task)
      {
        task();
      }
 
      guard->disable();
      return true;
    }
  };




} } } // namespace boost::threadpool::detail

#endif // THREADPOOL_POOL_CORE_HPP_INCLUDED
