/*
  Copyright (c) 2005-2007 by Jakob Schroeter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef DATAFORMREPORTED_H__
#define DATAFORMREPORTED_H__

#include "dataformfield.h"
#include "dataformbase.h"

namespace gloox
{

  class Tag;

  /**
   * @brief An abstraction of an &lt;reported&gt; element in a XEP-0004 Data Form of type result.
   *
   * There are some constraints regarding usage of this element you should be aware of. Check XEP-0004
   * section 3.4. This class does not enforce correct usage at this point.
   *
   * @author Jakob Schroeter <js@camaya.net>
   * @since 0.7
   */
  class GLOOX_API DataFormReported : public DataFormBase, public DataFormField
  {
    public:
      /**
       * Creates an empty 'reported' element you can add fields to.
       */
      DataFormReported();

      /**
       * Creates a 'reported' element and fills it with the 'field' elements contained in the given Tag.
       * The Tag's root element must be a 'reported' element. Its child element should be 'field' elements.
       * @param tag The tag to read the 'field' elements from.
       * @since 0.8.5
       */
      DataFormReported( Tag* tag );

      /**
       * Virtual destructor.
       */
      virtual ~DataFormReported();

      // reimplemented from DataFormField
      virtual Tag* tag() const;

  };

}

#endif // DATAFORMREPORTED_H__
