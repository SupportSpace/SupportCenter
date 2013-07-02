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

#ifndef THREADPOOL_DETAIL_SEQUENTIAL_TASK_HPP_INCLUDED
#define THREADPOOL_DETAIL_SEQUENTIAL_TASK_HPP_INCLUDED

#include <boost/function.hpp>

namespace boost { namespace threadpool { namespace detail 
{

  template <typename Task = function0<void> >
  class sequential_task
  {
  public:
   typedef Task task_type;                   //!< Provides the tasks' type.
   typedef void result_type; //!< Indicates the functor's result type.

   private: // Members
      task_type m_first_task;   //!< The first task's function.
      task_type m_second_task;  //!< The second task's function.

	  
  public:
    sequential_task(const task_type& first_task, const task_type& second_task)
      : m_first_task(first_task)
      , m_second_task(second_task)
    {
    }


    void operator() (void) const
    {
      if(m_first_task)
      {
        m_first_task();
      }
      if(m_second_task)
      {
        m_second_task();
      }
    }


  };


} } } // namespace boost::threadpool::detail

#endif // THREADPOOL_DETAIL_SEQUENTIAL_TASK_HPP_INCLUDED


