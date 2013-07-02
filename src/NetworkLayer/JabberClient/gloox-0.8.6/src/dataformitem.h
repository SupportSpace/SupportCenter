/*
  Copyright (c) 2005-2006 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef DATAFORMITEM_H__
#define DATAFORMITEM_H__

#include "dataformfield.h"
#include "dataformbase.h"

namespace gloox
{

  /**
   * @brief An abstraction of an &lt;item&gt; element in a JEP-0004 Data Form of type result.
   *
   * There are some constraints regarding usage of this element you should be aware of. Check JEP-0004
   * section 3.4. This class does not enforce correct usage at this point.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.7
   */
  class GLOOX_API DataFormItem : public DataFormBase, DataFormField
  {
    public:
      /**
       * Creates an empty 'item' element you can add fields to.
       */
      DataFormItem() : DataFormField( FIELD_TYPE_ITEM ) {};

      /**
       * Virtual destructor.
       */
      virtual ~DataFormItem() {};

  };

}

#endif // DATAFORMITEM_H__
