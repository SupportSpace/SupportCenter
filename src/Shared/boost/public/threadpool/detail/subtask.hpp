/*! \file
* \brief TODO.
*
* TODO. 
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


#ifndef THREADPOOL_DETAIL_SUBTASK_HPP_INCLUDED
#define THREADPOOL_DETAIL_SUBTASK_HPP_INCLUDED

//#include "pool.hpp"

#include "sequential_task.hpp"
#include "countdown.hpp"


#include <boost/smart_ptr.hpp>



namespace boost { namespace threadpool { namespace detail 
{

  /// subtask_interface
  template <typename Pool>
  class subtask_interface
  {
    typedef typename Pool::task_type task_type;
    typedef Pool pool_type;

  public:
    virtual const task_type& get_function() const = 0;
    virtual const std::list<shared_ptr<subtask_interface<pool_type> > >& get_parallel() const = 0;
    virtual const shared_ptr<subtask_interface<pool_type> > get_sequential() const = 0;
	
  protected:
    virtual ~subtask_interface(){}
  };



  
  // merged_task
  //template <typename Task = function0<void> >
  template <typename Pool>
  class merged_task
  { 
  public:
    typedef Pool pool_type;

  protected:
    typedef typename Pool::task_type task_type;
    typedef subtask_interface<Pool> subtask_type;

    shared_ptr<pool_type> m_pool;  

    task_type m_function;

    std::list<shared_ptr<merged_task<pool_type> > > m_parallel;
    shared_ptr<merged_task<pool_type> > m_sequential;
    function0<void> m_notify_finished;
    shared_ptr<countdown> m_countdown;


  public:
    static shared_ptr<merged_task> create_merged_task(shared_ptr<pool_type> const & pool, function0<void> const & notify_finished, const task_type& function, const std::list<shared_ptr<subtask_type> >& parallel, shared_ptr<subtask_type> const & sequential)
    {
      // Two phase construction:

      // Construct new merged_task (with own deleter because destructor is private)
      shared_ptr<merged_task> px(new merged_task(pool, notify_finished), scalar_deleter<merged_task>());

      // Complete construction
      construct(px, function, parallel, sequential);

      return px;
    }


    bool schedule()
    {
      bool res = true;

      // Schedule own task function
      if(m_function)
      {
        res = res && m_pool->schedule(m_function);
      }

      // Schedule all parallel tasks
      for(typename std::list<shared_ptr<merged_task> >::iterator it = m_parallel.begin();
        it != m_parallel.end();
        ++it)
      {
        res = res && (**it).schedule();
      }

      return res;
    }





  protected:
    merged_task(shared_ptr<pool_type> const & pool, function0<void> notify_finished)
      : m_pool(pool)
      , m_notify_finished(notify_finished)
    {
    }

    ~merged_task()
    {
    }

    template<typename T>
    struct scalar_deleter
    {
      typedef void result_type; //!< Indicates the functor's result type.

      void operator()(T* p)
      {
        delete p;
      }
    };  

    void finished_parallel()
    {
      // Check if all parallel functions are terminated
      if(m_countdown)
      {
        if(!m_countdown->decrease())
        {
          // count down is not finished; exit function
          return;
        }
      }

      // Schedule sequential task or notify parent that *this is finished
      if(m_sequential)
      {
        m_sequential->schedule();
      }
      else
      {
        if(m_notify_finished)
        {
          m_notify_finished();
        }
      }
    }


    void finished_sequential()
    {
      // Notify parent that *this is finished
      if(m_notify_finished)
      {
        m_notify_finished();
      }
    }




    static void construct(shared_ptr<merged_task> const & self, task_type const function, std::list<shared_ptr<subtask_type> > const & parallel,  shared_ptr<subtask_type> const & sequential)
    {
      self->m_function = sequential_task<task_type>(function, bind(&merged_task::finished_parallel, self));

      if(!parallel.empty() && (sequential || self->m_notify_finished) )
      {
        self->m_countdown.reset(new countdown(parallel.size() +  1));
      }

      if(!parallel.empty())
      {
        // Process all parallel tasks
        for(typename std::list<shared_ptr<subtask_type> >::const_iterator it = parallel.begin();
          it != parallel.end();
          ++it)
        {
          shared_ptr<merged_task> task = create_merged_task(self->m_pool, bind(&merged_task::finished_parallel, self), (**it).get_function(), (**it).get_parallel(), (**it).get_sequential());
          self->m_parallel.push_back(task);
        }
      }

      if(sequential)
      {
        self->m_sequential = create_merged_task(self->m_pool, bind(&merged_task::finished_sequential, self), sequential->get_function(), sequential->get_parallel(), sequential->get_sequential());
      }
    }
  };


} } } // namespace boost::threadpool::detail

#endif // THREADPOOL_DETAIL_SUBTASK_HPP_INCLUDED


