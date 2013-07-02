/*
  Copyright (c) 2005-2007 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef BASE64_H__
#define BASE64_H__

#include "macros.h"

#include <string>

namespace gloox
{

  /**
   * @brief An implementation of the Base64 data encoding (RFC 3548)
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.8
   */
  class GLOOX_API Base64
  {

    public:
      /**
       * Base64-encodes the input according to RFC 3548.
       * @param input The data to encode.
       * @return The encoded string.
       */
      static const std::string encode64( const std::string& input );

      /**
       * Base64-decodes the input according to RFC 3548.
       * @param input The encoded data.
       * @return The decoded data.
       */
      static const std::string decode64( const std::string& input );

    private:
      static const std::string alphabet64;
      static const std::string::size_type table64[];
      static const char pad;
      static const std::string::size_type np;
  };

}

#endif // BASE64_H__