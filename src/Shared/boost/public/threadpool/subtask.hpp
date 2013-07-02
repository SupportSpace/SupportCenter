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

#ifndef THREADPOOL_SUBTASK_HPP_INCLUDED
#define THREADPOOL_SUBTASK_HPP_INCLUDED

#include "pool.hpp"
#include "detail/subtask.hpp"

#include <boost/utility.hpp>
#include <boost/thread/mutex.hpp>


namespace boost { namespace threadpool
{
  /*! \brief Experimental. Do not use in production code. TODO. 
  *
  * TODO Combining sequential and parallel execution.
  *
  * \see TODO
  *
  */ 
  template<typename Pool>
  class subtask 
    : public detail::subtask_interface<Pool>
  {
  public:
    typedef typename Pool::task_type task_type;
    typedef Pool pool_type;

  private:
    task_type m_function;                          //!< The task's function.
    shared_ptr<subtask<pool_type> > m_sequential;              //!< TODO
    std::list<shared_ptr<detail::subtask_interface<pool_type> > > m_parallel;  //!< TODO

  public:

    /*! Constructor.
    * \param function The task's function object.
    */
    subtask(task_type const & function) 
    : m_function(function)
    {
    }

    /*! Constructor.
    */
    subtask() 
    {
    }

// FEATURE mit task_funcs!!!
// linked_task& operator|= (const task_func& rhs)
    

    // parallel
    subtask<pool_type>& operator|= (subtask<pool_type> const & rhs)
    {
      // Create copy and append list of parallel tasks
      shared_ptr<subtask<pool_type> > task(new subtask<pool_type>(rhs));
      m_parallel.push_back(task);
      return *this;
    }

    // parallel
    subtask<pool_type> operator| (subtask<pool_type> const & rhs)
    {
      subtask<pool_type> self = *this;
      return self |= rhs;
    }

    // sequential
    subtask<pool_type>& operator&= (subtask<pool_type> const & rhs)
    {
      //
      // Copies task and appends it to sequential task chain

      // Create copy
      shared_ptr<subtask<pool_type> > sequential(new subtask<pool_type>(rhs));

      // Append new linked task to sequential task chain 
      if(!m_sequential)
      { // New linked task shall be executed directly after *this
        m_sequential = sequential;
      }
      else
      { // Find last sequential task and append linked task
        shared_ptr<subtask<pool_type> > last_seq = m_sequential;

        while(last_seq->m_sequential)
        {
          last_seq = last_seq->m_sequential;
        }

        last_seq->m_sequential = sequential;
      }

      return *this;
    }

    // sequential
    subtask operator& (subtask<pool_type> const & rhs)
    {
      subtask<pool_type> self = *this;
      return self &= rhs;
    }


    bool schedule(shared_ptr<pool_type> const & pool) const
    {
      if(pool)
      { // Build bridge_task tree and schedule the tree for execution
        shared_ptr<detail::merged_task<pool_type> > task 
          = detail::merged_task<pool_type>::create_merged_task(
          pool, function0<void>(), m_function, m_parallel, m_sequential);
        if(task)
        {
          return task->schedule();
        }
      }

      return false;
    }

  private:
    virtual const task_type& get_function() const
    {
      return m_function;  
    }

    virtual const std::list<shared_ptr<detail::subtask_interface<Pool> > >& get_parallel() const
    {
      return m_parallel;
    }

    virtual const shared_ptr<detail::subtask_interface<Pool> > get_sequential() const
    {
      return m_sequential;
    }


  };


  typedef subtask<fifo_pool> task; // fifo_pool is required for correct execution order

  /*! Schedules a subtask for asynchronous execution. The task will be executed once only.
  * TODO convenient scheduling method
  * \param task A subtask which is composed of an arbitrary number of subtasks.
  */  
  template<class Pool>
  bool schedule(shared_ptr<Pool> const & pool, subtask<Pool> const & task)
  {	
    return task.schedule(pool);
  }	

} } // namespace boost::threadpool

#endif // THREADPOOL_SUBTASK_HPP_INCLUDED

