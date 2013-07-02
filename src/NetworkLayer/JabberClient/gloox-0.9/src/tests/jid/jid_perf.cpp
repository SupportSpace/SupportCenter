#include "../../jid.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

#include <sys/time.h>

static double divider = 1000000;
static int num = 10000;
static double t;

static JID * jid;

static void printTime ( const char * testName, struct timeval tv1, struct timeval tv2 )
{
  t = tv2.tv_sec - tv1.tv_sec;
  t +=  ( tv2.tv_usec - tv1.tv_usec ) / divider;
  printf( "%s: %.03f seconds (%.00f/s)\n", testName, t, num / t );
}

static const std::string addr = "username@server.org/resource";

static const int sz_s = 100;
static const int sz_b = 1000;


int main( int /*argc*/, char** /*argv*/ )
{
  struct timeval tv1;
  struct timeval tv2;

  printf( "Testing %d...\n", num );

  gettimeofday( &tv1, 0 );
  for( int i = 0; i < num; ++i )
  {
    delete new JID( addr );
  }
  gettimeofday( &tv2, 0 );
  printTime ("create/delete", tv1, tv2);


  // ---------------------------------------------------------------------

  JID * jid;
  gettimeofday( &tv1, 0 );
  for( int i = 0; i < num; ++i )
  {
    jid = new JID(addr);
    jid->bare();
    delete jid;
  }
  gettimeofday( &tv2, 0 );
  printTime ("create/delete bare", tv1, tv2);


  // ---------------------------------------------------------------------

  gettimeofday( &tv1, 0 );
  for( int i = 0; i < num; ++i )
  {
    jid = new JID(addr);
    jid->full();
    delete jid;
  }
  gettimeofday( &tv2, 0 );
  printTime ("create/delete full", tv1, tv2);


  // ---------------------------------------------------------------------

  gettimeofday( &tv1, 0 );
  for( int i = 0; i < num; ++i )
  {
    jid = new JID(addr);
    jid->bare();
    jid->full();
    delete jid;
  }
  gettimeofday( &tv2, 0 );
  printTime ("create/delete bare/full", tv1, tv2);


  // -----------------------------------------------------------------------

  jid = new JID(addr);
  gettimeofday( &tv1, 0 );
  for (int i = 0; i < num; ++i)
  {
    jid->bare();
  }
  gettimeofday( &tv2, 0 );
  delete jid;
  printTime ("bare", tv1, tv2);

  // -----------------------------------------------------------------------

  jid = new JID(addr);
  gettimeofday( &tv1, 0 );
  for (int i = 0; i < num; ++i)
  {
    jid->full();
  }
  gettimeofday( &tv2, 0 );
  delete jid;
  printTime ("full", tv1, tv2);



  return 0;
}
