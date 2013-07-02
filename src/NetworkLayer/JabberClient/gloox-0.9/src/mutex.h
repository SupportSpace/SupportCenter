/*
  Copyright (c) 2007 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef MUTEX_H__
#define MUTEX_H__

#include "macros.h"

namespace gloox
{

  class MutexImpl;

  /**
   * @brief A simple implementation of mutex as a wrapper around a pthread mutex
   * or a win32 critical section.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.9
   */
  class GLOOX_API Mutex
  {
    public:
      /**
       * Contructs a new simple mutex.
       */
      Mutex();

      /**
       * Destructor
       */
      ~Mutex();

      /**
       * Locks the mutex.
       */
      void lock();

      /**
       * Releases the mutex.
       */
      void unlock();

    private:
      Mutex& operator=( const Mutex& );
      MutexImpl* m_mutex;

  };
}

#endif // MUTEX_H__
