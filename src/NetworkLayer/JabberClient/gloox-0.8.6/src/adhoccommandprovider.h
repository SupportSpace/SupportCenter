/*
  Copyright (c) 2004-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef ADHOCCOMMANDPROVIDER_H__
#define ADHOCCOMMANDPROVIDER_H__

#include "tag.h"

#include <list>
#include <map>
#include <string>

namespace gloox
{

  /**
   * @brief A virtual interface for an Ad-hoc Command Provider according to JEP-0050.
   *
   * Derived classes can be registered as Command Providers with the Adhoc object.
   * @author Jakob Schroeter <js@camaya.net>
   */
  class GLOOX_API AdhocCommandProvider
  {
    public:
      /**
       * Virtual destructor.
       */
      virtual ~AdhocCommandProvider() {};

      /**
       * This function is called when an Ad-hoc Command needs to be handled.
       * The callee is responsible for the whole command execution, i.e. session
       * handling etc.
       * @param command The name of the command to be executed.
       * @param tag The complete command tag.
       */
      virtual void handleAdhocCommand( const std::string& command, Tag *tag ) = 0;

  };

}

#endif // ADHOCCOMMANDPROVIDER_H__
