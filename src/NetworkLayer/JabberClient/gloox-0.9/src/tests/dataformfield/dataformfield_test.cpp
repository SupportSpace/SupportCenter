#include "../../dataformfield.h"
#include "../../tag.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  DataFormField *f;

  // -------
  name = "empty field";
  f = new DataFormField();
  if( f->type() != DataFormField::FieldTypeTextSingle )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "FieldTypeBoolean field";
  f = new DataFormField( DataFormField::FieldTypeBoolean );
  if( f->type() != DataFormField::FieldTypeBoolean )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "2nd ctor";
  f = new DataFormField( "fieldName", "fieldValue", "fieldLabel", DataFormField::FieldTypeBoolean );
  if( f->type() != DataFormField::FieldTypeBoolean || f->name() != "fieldName" ||
      f->value() != "fieldValue" || f->label() != "fieldLabel" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "parse 0";
  f = new DataFormField( 0 );
  if( f->type() != DataFormField::FieldTypeInvalid )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "parse empty Tag";
  Tag *t = new Tag();
  f = new DataFormField( t );
  delete t;
  if( f->type() != DataFormField::FieldTypeInvalid )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "set name";
  f = new DataFormField();
  f->setName( name );
  if( f->name() != name )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "set required";
  f = new DataFormField();
  bool req = true;
  f->setRequired( req );
  if( f->required() != req )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "set label";
  f = new DataFormField();
  f->setLabel( name );
  if( f->label() != name )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "set value";
  f = new DataFormField();
  f->setValue( name );
  if( f->value() != name )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "set values";
  f = new DataFormField();
  StringList val;
  val.push_back( "val 1" );
  val.push_back( "val 2" );
  val.push_back( "val 3" );
  f->setValues( val );
  if( f->values() != val )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "set values";
  f = new DataFormField();
  StringMap opt;
  opt["lock"] = "1";
  opt["stock"] = "1";
  opt["smoking barrel"] = "2";
  f->setOptions( opt );
  if( f->options() != opt )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
  }
  delete f;
  f = 0;

  // -------
  name = "parse Tag 1";
  t = new Tag( "field");
  t->addAttribute( "type", "fixed" );
  new Tag( t, "value", "abc" );
  f = new DataFormField( t );
  Tag *ft = f->tag();
  if( ft->xml() != t->xml() )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
    printf( "f->tag(): %s\n", f->tag()->xml().c_str() );
    printf( "       t: %s\n", t->xml().c_str() );
  }
  delete f;
  delete t;
  delete ft;
  f = 0;

  // -------
  t = new Tag( "field");
  t->addAttribute( "type", "list-multi" );
  t->addAttribute( "label", "blabla label" );
  t->addAttribute( "var", "features" );
  Tag *o = new Tag( t, "option" );
  o->addAttribute( "label", "lock" );
  new Tag( o, "value", "lock" );
  o = new Tag( t, "option" );
  o->addAttribute( "label", "stock" );
  new Tag( o, "value", "stock" );
  o = new Tag( t, "option" );
  o->addAttribute( "label", "smoking barrel" );
  new Tag( o, "value", "smoking barrel" );
  new Tag( t, "value", "lock" );
  new Tag( t, "value", "stock" );
  f = new DataFormField( t );
  Tag *r = f->tag();
  name = "parse Tag 2.1";
  if( r->name() != "field" || !r->hasAttribute( "type", "list-multi" ) )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
    printf( "f->tag(): %s\n", f->tag()->xml().c_str() );
    printf( "       t: %s\n", t->xml().c_str() );
  }

  name = "parse Tag 2.2";
  if( !r->hasAttribute( "label", "blabla label" ) || !r->hasAttribute( "var", "features" ) )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
    printf( "f->tag(): %s\n", f->tag()->xml().c_str() );
    printf( "       t: %s\n", t->xml().c_str() );
  }

  name = "parse Tag 2.3";
  if( !r->hasChild( "option" ) || !r->findChild( "option" )->hasAttribute( "label", "lock" ) )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
    printf( "f->tag(): %s\n", f->tag()->xml().c_str() );
    printf( "       t: %s\n", t->xml().c_str() );
  }

  name = "parse Tag 2.4";
  if( !r->hasChild( "option", "label", "stock" ) )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
    printf( "f->tag(): %s\n", f->tag()->xml().c_str() );
    printf( "       t: %s\n", t->xml().c_str() );
  }

  name = "parse Tag 2.5";
  if( !r->hasChild( "option", "label", "smoking barrel" ) )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
    printf( "f->tag(): %s\n", f->tag()->xml().c_str() );
    printf( "       t: %s\n", t->xml().c_str() );
  }

  name = "parse Tag 2.6";
  if( !r->findChild( "option" )->findChild( "value" ) )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
    printf( "f->tag(): %s\n", f->tag()->xml().c_str() );
    printf( "       t: %s\n", t->xml().c_str() );
  }

  name = "parse Tag 2.7";
  if( r->findChild( "option" )->findChild( "value" )->cdata() != "lock" )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
    printf( "f->tag(): %s\n", f->tag()->xml().c_str() );
    printf( "       t: %s\n", t->xml().c_str() );
  }

  name = "parse Tag 2.8";
  Tag::TagList l = r->children();
  Tag::TagList::const_iterator it = l.begin();
  for( ; it != l.end(); ++it )
  {
    if( (*it)->name() == "option" && ( !(*it)->hasChildWithCData( "value", "lock" ) &&
          !(*it)->hasChildWithCData( "value", "stock" ) &&
          !(*it)->hasChildWithCData( "value", "smoking barrel" ) ) )
    {
      ++fail;
      printf( "test '%s' failed\n", name.c_str() );
      printf( "f->tag(): %s\n", f->tag()->xml().c_str() );
      printf( "       t: %s\n", t->xml().c_str() );
    }
  }

  name = "parse Tag 2.9";
  if( !r->hasChildWithCData( "value", "lock" ) || !r->hasChildWithCData( "value", "stock" ) )
  {
    ++fail;
    printf( "test '%s' failed\n", name.c_str() );
    printf( "f->tag(): %s\n", f->tag()->xml().c_str() );
    printf( "       t: %s\n", t->xml().c_str() );
  }
  delete f;
  delete t;
  delete r;
  f = 0;



  // -------
  name = "boolean duplicate <value/>";
  f = new DataFormField( DataFormField::FieldTypeBoolean );
  f->setName( "name" );
  f->setValue( "1" );
  f->setLabel( "label" );
  t = f->tag();
  if( t->children().size() != 1 || t->xml() != "<field type='boolean' var='name' "
                                               "label='label'><value>1</value></field>" )
  {
    ++fail;
    printf( "test '%s' failed: %s\n", name.c_str(), t->xml().c_str() );
  }
  delete f;
  delete t;
  f = 0;
  t = 0;









  if( fail == 0 )
  {
    printf( "DataFormField: all tests passed\n" );
    return 0;
  }
  else
  {
    printf( "DataFormField: %d test(s) failed\n", fail );
    return 1;
  }

}
