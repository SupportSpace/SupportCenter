#include "../../jid.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  JID j;

  // -------
  name = "bare JID ctor";
  j = JID( "abc@server.dom" );
  if( j.bare() != "abc@server.dom" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "full JID ctor";
  j = JID( "abc@server.dom/res" );
  if( j.full() != "abc@server.dom/res" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "prepped node";
  j = JID( "ABC@server.dom" );
  if( j.bare() != "abc@server.dom" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "prepped dom";
  j = JID( "abc@SeRvEr.dom" );
  if( j.bare() != "abc@server.dom" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "resource getter";
  j = JID( "abc@server.dom/rEsOurCe" );
  if( j.resource() != "rEsOurCe" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "node getter";
  j = JID( "aBc@server.dom/rEsOurCe" );
  if( j.username() != "abc" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "server getter";
  j = JID( "abc@serVer.dom/rEsOurCe" );
  if( j.server() != "server.dom" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "bare JID getter";
  j = JID( "abc@serVer.dom/rEsOurCe" );
  JID t1( "abc@serVer.dom/rEsOurCe");
  if( j.bareJID() != t1.bareJID() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "full JID getter";
  j = JID( "abc@serVer.dom/rEsOurCe" );
  JID t2( "abc@serVer.dom/rEsOurCe");
  if( j.fullJID() != t2.fullJID() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }

  // -------
  name = "clear jid";
  j = JID( "abc@serVer.dom/rEsOurCe" );
  j.setJID( "" );
  if( !j.empty() || !j.username().empty()
                 || !j.server().empty()
                 || !j.serverRaw().empty()
                 || !j.resource().empty()
                 || !j.bare().empty()
                 || !j.full().empty() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }












  if( fail == 0 )
  {
    printf( "JID: all tests passed\n" );
    return 0;
  }
  else
  {
    printf( "JID: %d test(s) failed\n", fail );
    return 1;
  }

}
