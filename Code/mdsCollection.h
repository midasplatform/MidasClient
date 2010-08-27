/*=========================================================================

  Copyright (c) Kitware Inc. All rights reserved.
  See Copyright.txt or http://www.Kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __mdsCollection_h_
#define __mdsCollection_h_

#include "mdsObject.h"

namespace mdo{
  class Collection;
}

namespace mds{

class Collection : public Object
{
public:

  Collection();
  ~Collection();

  bool Fetch();
  bool Commit();
  bool FetchTree();
  bool Delete();
  
  void SetObject(mdo::Object* object);
   
protected:

  mdo::Collection* m_Collection;

};

} //end namespace

#endif //__mdsCollection_h_
