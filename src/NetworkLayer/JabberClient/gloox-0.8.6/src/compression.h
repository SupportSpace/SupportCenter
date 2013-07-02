/*
  Copyright (c) 2005-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef COMPRESSION_H__
#define COMPRESSION_H__

#include "gloox.h"

#ifdef WIN32
# include "../config.h.win"
#else
# include "config.h"
#endif

#include <string>

#ifdef HAVE_ZLIB
#include <zlib/zlib.h>
#endif

namespace gloox
{
  /**
   * This is a wrapper around some compression methods.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.8
   */
  class GLOOX_API Compression
  {
    public:
      /**
       * Contructor.
       * @param method The desired compression method.
       */
      Compression( StreamFeature method );

      /**
       * Virtual Destructor.
       */
      virtual ~Compression();

      /**
       * Compresses the given chunk of data.
       * @param data The original (uncompressed) data.
       * @return The compressed data.
       */
      virtual const std::string compress( const std::string& data );

      /**
       * Decompresses the given chunk of data.
       * @param data The compressed data.
       * @return The decompressed data.
       */
      virtual const std::string decompress( const std::string& data );

    protected:
      bool m_valid;
      StreamFeature m_method;
      std::string m_inflateBuffer;
      int m_compCount;
      int m_decompCount;
      int m_dataOutCount;
      int m_dataInCount;

#ifdef HAVE_ZLIB
      z_stream m_zinflate;
      z_stream m_zdeflate;
#endif

  };

}

#endif // COMPRESSION_H__
