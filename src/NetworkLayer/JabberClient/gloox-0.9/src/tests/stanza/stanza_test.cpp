#include "../../stanza.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  Stanza *t;

  // -------
  name = "simple stanza";
  t = new Stanza( "test" );
  if( t->name() != "test" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete t;
  t = 0;










  if( fail == 0 )
  {
    printf( "Stanza: all tests passed\n" );
    return 0;
  }
  else
  {
    printf( "Stanza: %d test(s) failed\n", fail );
    return 1;
  }

}
