#include "../../tag.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  Tag *t = new Tag( "toe" ); t->addAttribute( "foo", "bar" );
  Tag *u = new Tag( t, "uni" ); u->addAttribute( "u3", "3u" );
  Tag *v = new Tag( t, "vie" ); v->addAttribute( "v3", "3v" );
  Tag *v2 = new Tag( t, "vie" ); v->addAttribute( "v32", "3v2" );
  Tag *w = new Tag( u, "who" ); w->addAttribute( "w3", "3w" );
  Tag *x = new Tag( v, "xep" ); x->addAttribute( "x3", "3x" );
  Tag *y = new Tag( u, "yps" ); y->addAttribute( "y3", "3y" );
  Tag *z = new Tag( w, "zoo" ); z->addAttribute( "z3", "3z" );
  Tag *c = 0;
  Tag *d = 0;

  // -------
  name = "undefined tag";
  c = new Tag();
  if( c->type() != StanzaUndefined || c->name() != "" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete c;
  c = 0;

  // -------
  name = "simple ctor";
  if( t->name() != "toe" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "cdata ctor";
  c = new Tag( "cod", "foobar" );
  if( c->name() != "cod" || c->cdata() != "foobar" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete c;
  c = 0;

  //-------
  name = "clone test 1";
  c = z->clone();
  if( *z != *c )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete c;
  c = 0;

  //-------
  name = "clone test 2";
  c = t->clone();
  if( *t != *c )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete c;
  c = 0;

  //-------
  name = "operator== test 1";
  c = new Tag();
  if( *t == *c )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete c;
  c = 0;

  //-------
  name = "operator== test 2";
  c = new Tag( "test" );
  c->addAttribute( "me", "help" );
  c->addChild( new Tag( "yes" ) );
  if( *t == *c )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete c;
  c = 0;

  //-------
  name = "operator== test 3";
  c = new Tag( "hello" );
  c->addAttribute( "test", "bacd" );
  c->addChild( new Tag( "hello" ) );
  d = new Tag( "hello" );
  d->addAttribute( "test", "bacd" );
  d->addChild( new Tag( "helloo" ) );
  if( *d == *c )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete c;
  delete d;
  c = 0;
  d = 0;

  //-------
  name = "operator!= test 1";
  c = new Tag( "hello" );
  c->addAttribute( "test", "bacd" );
  c->addChild( new Tag( "hello" ) );
  d = new Tag( "hello" );
  d->addAttribute( "test", "bacd" );
  d->addChild( new Tag( "hello" ) );
  if( *d != *c )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete c;
  delete d;
  c = 0;
  d = 0;

  //-------
  name = "findChildren test";
  Tag::TagList l = t->findChildren( "vie" );
  Tag::TagList::const_iterator it = l.begin();
  if( l.size() != 2 || (*it) != v || *(++it) != v2 )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete c;
  c = 0;

  //-------
  name = "escape";
  if ( Tag::escape( "&<>'\"" ) != "&amp;&lt;&gt;&apos;&quot;" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  //-------
  name = "relax";
  if ( Tag::relax( "&amp;&lt;&gt;&apos;&quot;&#60;&#62;&#39;&#34;""&#x3c;&#x3e;"
                   "&#x3C;&#x3E;&#x27;&#x22;&#X3c;&#x3e;&#X3C;&#X3E;&#X27;&#X22;" )
        != "&<>'\"<>'\"<><>'\"<><>'\"" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  //-------
  name = "xml() 1";
  if( t->xml() != "<toe foo='bar'><uni u3='3u'><who w3='3w'><zoo z3='3z'/></who><yps y3='3y'/>"
                    "</uni><vie v3='3v' v32='3v2'><xep x3='3x'/></vie><vie/></toe>" )
  {
    ++fail;
    printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }

  //-------
  name = "xml() 2";
  t->addAttribute( "test", "bacd" );
  if( t->xml() != "<toe foo='bar' test='bacd'><uni u3='3u'><who w3='3w'><zoo z3='3z'/></who><yps y3='3y'/>"
                    "</uni><vie v3='3v' v32='3v2'><xep x3='3x'/></vie><vie/></toe>" )
  {
    ++fail;
    printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }

  //-------
  name = "hasChild 1";
  if( !t->hasChild( "uni" ) || !t->hasChild( "vie" ) || !u->hasChild( "who" ) || !w->hasChild( "zoo" )
      || !u->hasChild( "yps" ) )
  {
    ++fail;
    printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }

  //-------
  name = "hasAttribute 1";
  if( !t->hasAttribute( "test" ) || !t->hasAttribute( "test", "bacd" )
      || !t->hasAttribute( "foo" ) || !t->hasAttribute( "foo", "bar" ) )
  {
    ++fail;
    printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }

  //-------
  name = "findAttribute 1";
  if( t->findAttribute( "test" ) != "bacd" || t->findAttribute( "foo" ) != "bar" )
  {
    ++fail;
    printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }

  //-------
  name = "findChild 1";
  c = t->findChild( "uni" );
  if( c != u )
  {
    ++fail;
    printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }

  //-------
  name = "findChild 2";
  c = t->findChild( "uni", "u3" );
  if( c != u )
  {
    ++fail;
    printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }

  //-------
  name = "findChild 3";
  c = t->findChild( "uni", "u3", "3u" );
  if( c != u )
  {
    ++fail;
    printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }

  //-------
  name = "findChildWithAttrib 1";
  c = t->findChildWithAttrib( "u3" );
  if( c != u )
  {
    ++fail;
    printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }

  //-------
  name = "findChildWithAttrib 2";
  c = t->findChildWithAttrib( "u3", "3u" );
  if( c != u )
  {
    ++fail;
    printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }

  //-------
  name = "attribute order";
  c = new Tag( "abc" );
  c->addAttribute( "abc", "def" );
  c->addAttribute( "xyz", "123" );
  d = c->clone();
  if( c->xml() != d->xml() )
  {
    ++fail;
    printf( "test '%s' failed: %s\n", name.c_str(), d->xml().c_str() );
  }
  delete c;
  c = 0;
  delete d;
  d = 0;








  delete t;
  t = 0;




















  if( fail == 0 )
  {
    printf( "Tag: all tests passed\n" );
    return 0;
  }
  else
  {
    printf( "Tag: %d test(s) failed\n", fail );
    return 1;
  }

}
