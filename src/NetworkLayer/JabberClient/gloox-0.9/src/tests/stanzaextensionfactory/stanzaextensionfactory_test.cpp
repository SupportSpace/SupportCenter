#include "../../oob.h"
#include "../../xdelayeddelivery.h"
#include "../../delayeddelivery.h"
#include "../../vcardupdate.h"
#include "../../gpgsigned.h"
#include "../../gpgencrypted.h"
#include "../../stanzaextension.h"
#include "../../stanzaextensionfactory.h"
#include "../../tag.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  StanzaExtension *se;
  Tag *t;

  // -------
  name = "OOB test";
  OOB *o = new OOB( "url", "desc", false );
  t = o->tag();
  se = StanzaExtensionFactory::create( t );
  if( se->type() != ExtOOB )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete o;
  delete t;
  delete se;
  t = 0;

  // -------
  {
    name = "XDelayedDelivery test";
    JID from( "abc@example.net" );
    XDelayedDelivery *x = new XDelayedDelivery( from, "stamp", "reason" );
    t = x->tag();
    se = StanzaExtensionFactory::create( t );
    if( se->type() != ExtXDelay )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete x;
    delete t;
    delete se;
    t = 0;
  }

  // -------
  {
    name = "DelayedDelivery test";
    JID from( "abc@example.net" );
    DelayedDelivery *d = new DelayedDelivery( from, "stamp", "reason" );
    t = d->tag();
    se = StanzaExtensionFactory::create( t );
    if( se->type() != ExtDelay )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete d;
    delete t;
    delete se;
    t = 0;
  }

  // -------
  {
    name = "VCardUpdate test";
    VCardUpdate *d = new VCardUpdate( "hash" );
    t = d->tag();
    se = StanzaExtensionFactory::create( t );
    if( se->type() != ExtVCardUpdate )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete d;
    delete t;
    delete se;
    t = 0;
  }

  // -------
  {
    name = "GPGSigned test";
    GPGSigned *d = new GPGSigned( "signature" );
    t = d->tag();
    se = StanzaExtensionFactory::create( t );
    if( se->type() != ExtGPGSigned )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete d;
    delete t;
    delete se;
    t = 0;
  }

  // -------
  {
    name = "GPGEncrypted test";
    GPGEncrypted *d = new GPGEncrypted( "encrypted" );
    t = d->tag();
    se = StanzaExtensionFactory::create( t );
    if( se->type() != ExtGPGEncrypted )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
    }
    delete d;
    delete t;
    delete se;
    t = 0;
  }



  if( fail == 0 )
  {
    printf( "StanzaExtensionFactory: all tests passed\n" );
    return 0;
  }
  else
  {
    printf( "StanzaExtensionFactory: %d test(s) failed\n", fail );
    return 1;
  }

}
