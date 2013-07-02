/*
  Copyright (c) 2004-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#ifndef PARSER_H__
#define PARSER_H__

#include "public\gloox.h" 

#include <iksemel\iksemel.h>

#include <string>

namespace gloox
{

  class ClientBase;
  class Stanza;

  /**
   * @brief This class is an abstraction of libiksemel's XML parser.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.4
   */
  class GLOOX_API Parser
  {
    public:
      /**
       * Describes the return values of the parser.
       */
      enum ParserState
      {
        PARSER_OK,                     /**< Everything's alright. */
        PARSER_NOMEM,                  /**< Memory allcation error. */
        PARSER_BADXML                  /**< XML parse error. */
      };

      /**
       * Constructs a new Parser object.
       * @param parent The object to send incoming Tags to.
       */
      Parser( ClientBase *parent );

      /**
       * Virtual destructor.
       */
      virtual ~Parser();

      /**
       * Use this function to feed the parser with more XML.
       * @param data Raw xml to parse.
       * @return The return value indicates success or failure of the parsing.
       */
      ParserState feed( const std::string& data );

    private:
      void streamEvent( Stanza *stanza );

      iksparser *m_parser;
      ClientBase *m_parent;
      Stanza *m_current;
      Stanza *m_root;

      /**
       * Called by iksemel's parser with cdata for the current node.
       * @param parser The current Parser.
       * @param data The cdata.
       * @param len The length of the data.
       */
      friend int cdataHook( Parser *parser, char *data, size_t len );

      /**
       * Called by iksemel's parser for every new element.
       * @param parser The current Parser.
       * @param name The element's name.
       * @param atts The element's list of attributes.
       * @param type The type of the element.
       */
      friend int tagHook( Parser *parser, char *name, char **atts, int type );
  };

  int cdataHook( Parser *parser, char *data, size_t len );

  int tagHook( Parser *parser, char *name, char **atts, int type );

}

#endif // PARSER_H__
