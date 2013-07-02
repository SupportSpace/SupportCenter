#include "../../tag.h"
#include "../../oob.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  Tag *t;
  OOB *d;

  // -------
  name = "parsing 0 tag";
  d = new OOB( 0 );
  if( d->tag() != 0 )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete d;
  d = 0;

  // -------
  name = "parsing empty tag";
  t = new Tag();
  d = new OOB( t );
  if( d->tag() != 0 )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete d;
  delete t;
  d = 0;
  t = 0;


  // -------------------
  // jabber:x:oob tests
  // -------------------
  Tag *x = new Tag( "x" );
  x->addAttribute( "xmlns", XMLNS_X_OOB );
  new Tag( x, "url", "invalidurl" );
  new Tag( x, "desc", "description" );


  // -------
  name = "filled object/getters";
  d = new OOB( "invalidurl", "description", false );
  if( d->url() != "invalidurl" || d->desc() != "description" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete d;
  delete t;
  d = 0;
  t = 0;

  // -------
  name = "filled object/tag()";
  d = new OOB( "invalidurl", "description", false );
  t = d->tag();
  if( !t || !t->hasAttribute( "xmlns", XMLNS_X_OOB )
       || !t->hasChild( "url" ) || t->findChild( "url" )->cdata() != "invalidurl"
       || !t->hasChild( "desc" ) || t->findChild( "desc" )->cdata() != "description")
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete d;
  delete t;
  d = 0;
  t = 0;

  // -------
  name = "from Tag/getters";
  d = new OOB( x );
  if( d->url() != "invalidurl" || d->desc() != "description" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete d;
  delete t;
  d = 0;
  t = 0;

  // -------
  name = "from Tag/tag()";
  d = new OOB( x );
  t = d->tag();
  if( !t || !t->hasAttribute( "xmlns", XMLNS_X_OOB )
       || !t->hasChild( "url" ) || t->findChild( "url" )->cdata() != "invalidurl"
       || !t->hasChild( "desc" ) || t->findChild( "desc" )->cdata() != "description")
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete d;
  delete t;
  d = 0;
  t = 0;


  // -------------------
  // jabber:iq:oob tests
  // -------------------

  delete x;
  x  = 0;
  x = new Tag( "query" );
  x->addAttribute( "xmlns", XMLNS_IQ_OOB );
  new Tag( x, "url", "invalidurl" );
  new Tag( x, "desc", "description" );


  // -------
  name = "filled object/getters";
  d = new OOB( "invalidurl", "description", true );
  if( d->url() != "invalidurl" || d->desc() != "description" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete d;
  delete t;
  d = 0;
  t = 0;

  // -------
  name = "filled object/tag()";
  d = new OOB( "invalidurl", "description", true );
  t = d->tag();
  if( !t || !t->hasAttribute( "xmlns", XMLNS_IQ_OOB )
       || !t->hasChild( "url" ) || t->findChild( "url" )->cdata() != "invalidurl"
       || !t->hasChild( "desc" ) || t->findChild( "desc" )->cdata() != "description")
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete d;
  delete t;
  d = 0;
  t = 0;

  // -------
  name = "from Tag/getters";
  d = new OOB( x );
  if( d->url() != "invalidurl" || d->desc() != "description" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete d;
  delete t;
  d = 0;
  t = 0;

  // -------
  name = "from Tag/tag()";
  d = new OOB( x );
  t = d->tag();
  if( !t || !t->hasAttribute( "xmlns", XMLNS_IQ_OOB )
       || !t->hasChild( "url" ) || t->findChild( "url" )->cdata() != "invalidurl"
       || !t->hasChild( "desc" ) || t->findChild( "desc" )->cdata() != "description")
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete d;
  delete t;
  d = 0;
  t = 0;



  delete x;
  x = 0;

  if( fail == 0 )
  {
    printf( "OOB: all tests passed\n" );
    return 0;
  }
  else
  {
    printf( "OOB: %d test(s) failed\n", fail );
    return 1;
  }

}
