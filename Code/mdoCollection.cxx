/*=========================================================================

  Copyright (c) Kitware Inc. All rights reserved.
  See Copyright.txt or http://www.Kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "mdoCollection.h"
#include "mdoCommunity.h"
#include <sstream>
#include <iostream>
#include "midasStandardIncludes.h"

namespace mdo{

/** Constructor */
Collection::Collection()
{
  m_Id = 0;
  m_ParentCommunity = NULL;
  fetched = false;
}
  
/** Destructor */
Collection::~Collection()
{
  // Clean the items
  std::vector<Item*>::iterator itItem = m_Items.begin();
  while(itItem != m_Items.end())
    {
    Item* item = *itItem;
    itItem++;
    delete item;
    }
  m_Items.clear();
  delete m_ParentCommunity;
}
  
/** Load */
bool Collection::Load()
{
  return m_Proxy->Load();
}

void Collection::Clear()
{
  this->m_Name = "";
  this->m_Description = "";
  this->m_IntroductoryText = "";
  this->m_Copyright = "";
  this->m_Uuid = "";
  this->m_Parent = "";
  this->m_HasAgreement = "0";
}

bool Collection::SetValue(std::string key, std::string value, bool append)
{
  key = kwsys::SystemTools::UpperCase(key);

  if(key == "NAME")
    {
    if(append)
      {
      m_Name += value;
      }
    else
      {
      m_Name = value;
      }
    return true;
    }
  if(key == "COPYRIGHT")
    {
    if(append)
      {
      m_Copyright += value;
      }
    else
      {
      m_Copyright = value;
      }
    return true;
    }
  if(key == "DESCRIPTION")
    {
    if(append)
      {
      m_Description += value;
      }
    else
      {
      m_Description = value;
      }
    return true;
    }
  if(key == "INTRODUCTORY" || key == "INTRODUCTORYTEXT")
    {
    if(append)
      {
      m_IntroductoryText += value;
      }
    else
      {
      m_IntroductoryText = value;
      }
    return true;
    }

  return false;
}

} // end namespace
