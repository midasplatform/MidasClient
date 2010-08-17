/*=========================================================================

  Copyright (c) Kitware Inc. All rights reserved.
  See Copyright.txt or http://www.Kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MIDASUTILS_H
#define MIDASUTILS_H

#include "midasStandardIncludes.h"

namespace mdo{
  class Object;
}

class midasUtils
{
public:
  static std::string GenerateUUID();
  static std::string EscapeForURL(std::string s);
  static std::string EscapeForSQL(std::string s);
  static std::string EscapeName(std::string name);
  static std::string FileSizeString(long bytes);
  static std::string GetTypeName(int type);
  static bool IsDatabaseValid(std::string path);
  static bool CreateNewDatabase(std::string path);
  static mdo::Object* FetchByUuid(std::string uuid);
  static void StringTrim(std::string& str);
  static int GetParentType(int type);
};

#endif