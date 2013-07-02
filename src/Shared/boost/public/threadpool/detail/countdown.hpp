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

#ifndef THREADPOOL_DETAIL_COUNTDOWN_HPP_INCLUDED
#define THREADPOOL_DETAIL_COUNTDOWN_HPP_INCLUDED

#include <boost/utility.hpp>
#include <boost/thread/mutex.hpp>

#include "locking_ptr.hpp"

namespace boost { namespace threadpool { namespace detail 
{

  // count down
  class countdown
    : private noncopyable 
  {
  private:
      size_t m_counter;
	    mutex m_monitor;
  
  public:
    countdown(size_t const start_value)
      : m_counter(start_value)
    {}

    bool decrease() volatile
    {
      //countdown* self = const_cast<countdown*>(this);
      locking_ptr<countdown> self(*this, m_monitor); 
//      mutex::scoped_lock lock(m_monitor);

      if(self->m_counter > 0)
      {
        --self->m_counter;
      }

      return 0 >= self->m_counter;
    }

    size_t get_counter() const volatile
    {
      return m_counter;
    }
  };



} } } // namespace boost::threadpool::detail

#endif // THREADPOOL_DETAIL_COUNTDOWN_HPP_INCLUDED

