/******************************************************************************
 * Copyright 2011 Kitware Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/


#ifndef __mdsCollection_h_
#define __mdsCollection_h_

#include "mdsObject.h"

namespace mdo
{
class Collection;
}

namespace mds
{

class Collection : public Object
{
public:

  Collection();
  ~Collection();

  bool Fetch();

  bool Commit();

  bool FetchTree();

  bool FetchSize();

  bool Delete(bool deleteOnDisk);

  void ParentPathChanged(std::string parentPath);

  void SetObject(mdo::Object* object);

  void SetRecursive(bool recurse);

  void SetPath(std::string path);

protected:
  bool             m_Recurse;
  mdo::Collection* m_Collection;
  std::string      m_Path;

};

} // end namespace

#endif // __mdsCollection_h_
