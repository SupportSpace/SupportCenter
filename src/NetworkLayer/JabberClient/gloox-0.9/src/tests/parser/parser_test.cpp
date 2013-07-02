#include "../../parser.h"
#include "../../taghandler.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

class ParserTest : private TagHandler
{
  public:
    ParserTest() : m_tag( 0 ) {}
    virtual ~ParserTest() {}

    virtual void handleTag( Tag *tag )
    {
      m_tag = tag->clone();
    }

    int run()
    {
      int fail = 0;
      std::string name;
      std::string data;
      bool tfail = false;
      Parser *p = new Parser( this );


      // -------
      name = "simple";
      data = "<tag/>";
      p->feed( data );
      if( m_tag == 0 || m_tag->name() != "tag" )
      {
        ++fail;
        printf( "test '%s: %s' failed\n", name.c_str(), data.c_str() );
      }
      delete m_tag;
      m_tag = 0;

      // -------
      name = "simple child";
      data = "<tag1><child/></tag1>";
      p->feed( data );
      if( m_tag == 0 || m_tag->name() != "tag1" || !m_tag->hasChild( "child" ) )
      {
        ++fail;
        printf( "test '%s: %s' failed\n", name.c_str(), data.c_str() );
      }
      delete m_tag;
      m_tag = 0;

      // -------
      name = "attribute";
      data = "<tag2 attr='val'><child/></tag2>";
      p->feed( data );
      if( m_tag == 0 ||
            m_tag->name() != "tag2" ||
            !m_tag->hasAttribute( "attr", "val" ) ||
            !m_tag->hasChild( "child" ) )
      {
        ++fail;
        printf( "test '%s: %s' failed\n", name.c_str(), data.c_str() );
      }
      delete m_tag;
      m_tag = 0;

      // -------
      name = "attribute in child";
      data = "<tag3><child attr='val'/></tag3>";
      p->feed( data );
      if( m_tag == 0 ||
            m_tag->name() != "tag3" ||
            !m_tag->hasChild( "child" ) )
      {
        tfail = true;
      }
      else
      {
        Tag *c = m_tag->findChild( "child" );
        if( !c->hasAttribute( "attr", "val" ) )
        {
          tfail = true;
        }
      }
      if( tfail )
      {
        ++fail;
        printf( "test '%s: %s' failed\n", name.c_str(), data.c_str() );
        tfail = false;
      }
      delete m_tag;
      m_tag = 0;

      // -------
      name = "cdata";
      data = "<tag4>cdata</tag4>";
      p->feed( data );
      if( m_tag == 0 ||
            m_tag->name() != "tag4" ||
            m_tag->cdata() != "cdata" )
      {
        ++fail;
        printf( "test '%s: %s' failed\n", name.c_str(), data.c_str() );
      }
      delete m_tag;
      m_tag = 0;

      // -------
      name = "tag w/ whitespace 1";
      data = "< tag4 />";
      p->feed( data );
      if( m_tag == 0 ||
            m_tag->name() != "tag4" )
      {
        ++fail;
        printf( "test '%s: %s' failed\n", name.c_str(), data.c_str() );
      }
      delete m_tag;
      m_tag = 0;

      // -------
      name = "tag w/ whitespace 2";
      data = "< tag4/ >";
      p->feed( data );
      if( m_tag == 0 ||
            m_tag->name() != "tag4" )
      {
        ++fail;
        printf( "test '%s: %s' failed\n", name.c_str(), data.c_str() );
      }
      delete m_tag;
      m_tag = 0;

        // -------
      name = "tag w/ whitespace 3";
      data = "< tag4 / >";
      p->feed( data );
      if( m_tag == 0 ||
            m_tag->name() != "tag4" )
      {
        ++fail;
        printf( "test '%s: %s' failed\n", name.c_str(), data.c_str() );
      }
      delete m_tag;
      m_tag = 0;

    //   // -------
    //   name = "tag w/ cdata and child";
    //   data = "< tag4 > cdata < tag/ ></tag4>";
    //   if( c->setTest( p, data ) != Parser::PARSER_BADXML );
    //   {
    //     s = c->getLastResult();
    //     printf( "xml: %s\n", m_tag->xml().c_str() );
    //     ++fail;
    //     printf( "test '%s: %s' failed\n", name.c_str(), data.c_str() );
    //   }
    //   delete m_tag;
    //   m_tag = 0;


      // -------
      name = "simple child + white\tspace";
      data = "<tag1 ><child\t/ >< /  \ttag1>";
      p->feed( data );
      if( m_tag == 0 ||
            m_tag->name() != "tag1" ||
            !m_tag->hasChild( "child" ) )
      {
        ++fail;
        printf( "test '%s: %s' failed\n", name.c_str(), data.c_str() );
      }
      delete m_tag;
      m_tag = 0;

      //-------
      name = "stream start";
      data = "<stream:stream version='1.0' to='example.org' xmlns='jabber:client' id='abcdef'>";
      p->feed( data );
      if( m_tag == 0 ||
            m_tag->name() != "stream:stream" ||
            !m_tag->hasAttribute( "version", "1.0" ) ||
            !m_tag->hasAttribute( "id", "abcdef" ) ||
            !m_tag->hasAttribute( "to", "example.org" ) ||
            !m_tag->hasAttribute( "xmlns", "jabber:client" ) )
      {
        ++fail;
        printf( "test '%s: %s' failed\n", name.c_str(), data.c_str() );
      }
    //   printf( "stanza: %s\n", m_tag->xml().c_str() );
      delete m_tag;
      m_tag = 0;


      // -------
      name = "prolog";
      data = "<?xml version='1.0'?>";
      p->feed( data );
      if( ( m_tag != 0 )/* ||
            m_tag->name() != "xml" ||
            !m_tag->hasAttribute( "version", "1.0" )*/ )
      {
        ++fail;
        printf( "test '%s: %s' failed\n", name.c_str(), data.c_str() );
      }
    //   printf( "stanza: %s\n", m_tag->xml().c_str() );
      delete m_tag;
      m_tag = 0;


      // -------
      name = "deeply nested";
      data = "<tag1 attr11='val11' attr12='val12'><tag2 attr21='val21' attr22='val22'/><tag3 attr31='val31'><tag4>cdata1</tag4><tag4>cdata2</tag4></tag3></tag1>";
      p->feed( data );
      if( m_tag == 0 ||
            m_tag->name() != "tag1" ||
            !m_tag->hasAttribute( "attr11", "val11" ) ||
            !m_tag->hasAttribute( "attr12", "val12" ) ||
            !m_tag->hasChild( "tag2" ) ||
            !m_tag->hasChild( "tag3" ) )
      {
        printf( "fail1\n" );
        tfail = true;
      }
      else
      {
        Tag *c = m_tag->findChild( "tag2" );
        if( !c->hasAttribute( "attr21", "val21" ) ||
            !c->hasAttribute( "attr22", "val22" ) )
        {
          printf( "fail2\n" );
          tfail = true;
        }
        c = m_tag->findChild( "tag3" );
        if( !c->hasAttribute( "attr31", "val31" ) ||
            !c->hasChild( "tag4" ) ||
            !c->hasChild( "tag4" ) )
        {
          printf( "fail3\n" );
          tfail = true;
        }
      }
      if( tfail )
      {
        ++fail;
        printf( "test '%s: %s' failed\n", name.c_str(), data.c_str() );
        printf( "got: %s\n", m_tag->xml().c_str() );
        tfail = false;
      }
      delete m_tag;
      m_tag = 0;

      // -------
      name = "mixed content";
      data = "<tag1>cdata1<tag2>cdata2</tag2>cdata3</tag1>";
      p->feed( data );
      if( m_tag == 0 )
      {
        ++fail;
        printf( "test '%s: %s' failed\n", name.c_str(), data.c_str() );
      }
      else
    //   printf( "stanza: %s\n", m_tag->xml().c_str() );
      delete m_tag;
      m_tag = 0;








      delete p;
      p = 0;

      if( fail == 0 )
      {
        printf( "Parser: all tests passed\n" );
        return 0;
      }
      else
      {
        printf( "Parser: %d test(s) failed\n", fail );
        return 1;
      }

    }

  private:
    Tag *m_tag;

};

int main( int /*argc*/, char** /*argv*/ )
{
  ParserTest p;
  return p.run();
}
