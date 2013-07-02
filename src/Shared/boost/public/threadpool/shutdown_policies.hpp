/*! \file
* \brief Shutdown policies.
*
* This file contains shutdown policies for thread_pool. 
* A shutdown policy controls the pool's behavior from the time
* when the pool is not referenced any longer.
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


#ifndef THREADPOOL_SHUTDOWN_POLICIES_HPP_INCLUDED
#define THREADPOOL_SHUTDOWN_POLICIES_HPP_INCLUDED



/// The namespace threadpool contains a thread pool and related utility classes.
namespace boost { namespace threadpool
{


  /*! \brief ShutdownPolicy which waits for the completion of all tasks.
  *
  * \param Pool The pool's core type.
  */ 
  template<typename Pool>
  class wait_for_all_tasks
  {
  public:
    static void shutdown(Pool& pool)
    {
      pool.wait();
      pool.m_allow_resize = false;
      pool.resize(0);
    }
  };


  /*! \brief ShutdownPolicy which waits for the completion of all active tasks.
  *
  * \param Pool The pool's core type.
  */ 
  template<typename Pool>
  class wait_for_active_tasks
  {
  public:
    static void shutdown(Pool& pool)
    {
      pool.clear();
      pool.wait();
      pool.m_allow_resize = false;
      pool.resize(0);
    }
  };


  /*! \brief ShutdownPolicy which does not wait for any tasks.
  *
  * This policy does not wait for any tasks. Nevertheless all active tasks will be processed completely.
  *
  * \param Pool The pool's core type.
  */ 
  template<typename Pool>
  class immediately
  {
  public:
    static void shutdown(Pool& pool)
    {
      pool.clear();
      pool.m_allow_resize = false;
      pool.resize(0);
    }
  };

} } // namespace boost::threadpool

#endif // THREADPOOL_SHUTDOWN_POLICIES_HPP_INCLUDED
