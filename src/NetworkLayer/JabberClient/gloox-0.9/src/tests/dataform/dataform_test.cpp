#include "../../dataformbase.h"
#include "../../dataformfield.h"
#include "../../dataform.h"
#include "../../tag.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  DataForm *f;

  // -------
  name = "empty form";
  f = new DataForm();
  if( f->type() != DataForm::FormTypeInvalid )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "empty form tag";
  f = new DataForm();
  if( f->tag() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "form title";
  std::string title = "form test title";
  f = new DataForm();
  f->setTitle( title );
  if( f->title() != title )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "form instructions";
  StringList instructions;
  instructions.push_back( "form test instructions" );
  instructions.push_back( "line 2" );
  instructions.push_back( "line 3" );
  f = new DataForm();
  f->setInstructions( instructions );
  if( f->instructions() != instructions )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "form type, title, instructions";
  // using StringList instructions from previous test case
  // using std::string title from pre-previous test case
  f = new DataForm( DataForm::FormTypeForm, instructions, title );
  if( f->instructions() != instructions )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  if( f->title() != title )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  if( f->type() != DataForm::FormTypeForm )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "parse empty Tag (ctor)";
  Tag *t = new Tag();
  f = new DataForm( t );
  delete t;
  if( f->type() != DataForm::FormTypeInvalid )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "parse() empty Tag";
  f = new DataForm();
  t = new Tag();
  f->parse( t );
  delete t;
  if( f->type() != DataForm::FormTypeInvalid )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "parse 0";
  f = new DataForm();
  f->parse( 0 );
  if( f->type() != DataForm::FormTypeInvalid )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;





  if( fail == 0 )
  {
    printf( "DataForm: all tests passed\n" );
    return 0;
  }
  else
  {
    printf( "DataForm: %d test(s) failed\n", fail );
    return 1;
  }

}
