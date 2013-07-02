/*! \file
* \brief Thread pool worker.
*
* The worker thread instance is attached to a pool 
* and executes tasks of this pool. 
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

#ifndef THREADPOOL_DETAIL_WORKER_THREAD_HPP_INCLUDED
#define THREADPOOL_DETAIL_WORKER_THREAD_HPP_INCLUDED


#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/exceptions.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>


namespace boost { namespace threadpool { namespace detail 
{

  /*! \brief Thread pool worker. 
  *
  * A worker_thread represents a thread of execution. The worker is attached to a 
  * thread pool and processes tasks of that pool. The lifetime of the worker and its 
  * internal boost::thread is managed automatically.
  *
  * This class is a helper class and cannot be constructed or accessed directly.
  * 
  * \see pool_core
  */ 
  template <typename Pool>
  class worker_thread
  : private noncopyable
  {
  public:
    typedef Pool pool_type;         //!< Indicates the pool's type.

  private:
    shared_ptr<pool_type>      m_pool;     //!< Weak pointer to the pool which owns the worker.
    shared_ptr<boost::thread>  m_thread;   //!< Pointer to the thread which executes the run loop.

    
    /*! Constructs a new worker. 
    * \param pool Pointer to it's parent pool.
    * \see function create_and_attach
    */
    worker_thread(shared_ptr<pool_type> const & pool)
    : m_pool(pool)
    {
      assert(pool);
    }

  public:
    /*! Destructor.
    */
    ~worker_thread()
    {
    }

  public:
    /*! Executes pool's tasks sequentially.
    */
    void run() const 
    { 
      while(m_pool->execute_task()) {}
    }


  public:
    /*! Constructs a new worker thread and attachs it to the pool.
    * \param pool Pointer to the pool.
    */
    static void create_and_attach(shared_ptr<pool_type> const & pool)
    {
      shared_ptr< worker_thread<pool_type> > worker(new worker_thread<pool_type>(pool));
      if(worker)
      {
        worker->m_thread.reset(new boost::thread(bind(&worker_thread<pool_type>::run, worker)));
      }
    }

  };


} } } // namespace boost::threadpool::detail

#endif // THREADPOOL_DETAIL_WORKER_THREAD_HPP_INCLUDED

